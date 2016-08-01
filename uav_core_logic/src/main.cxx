/** @file UAV core logic node
  * @author DreamTale
  * @date Jul 30, 2016
  */
#include "generalHeader.h"
#include "recvInfo.h"

#include <opencv2/opencv.hpp>

double KP_X = 0.0;
double KP_Y = 0.0;
double KD_X = 0.0;
double KD_Y = 0.0;

ros::Publisher pubServo;
ros::Publisher pubOnboard;

enum uavStatusInfo
{
    uavStateWait  = 0xf00,
    uavStateReady,
    uavStateRunning
};

// The sort of uav control signals
enum uavCtrlSignal
{
    uavTakeOff = 0,
    uavLanding,
    uavHangSlient,
    uavGoFlightHeigh,
    uavGoMissionHeigh,
    uavMovVelocity,
    uavForceMannul,
    uavGetCtrlAbility,
    uavControlSum
};

volatile int uavStatus = uavStateWait;
// +-------+-------+-------+-------+-------+-------+
// | vel_x | vel_y | vel_z | pos_x | pos_y | pos_z |
// +-------+-------+-------+-------+-------+-------+
volatile double droneVoInfo[6] = {0.0};

void callBackGroundBuffCar(const std_msgs::String::ConstPtr& msg)
{

}

void callBackGuidanceVoInfo(const std_msgs::Float64MultiArray::ConstPtr& msg)
{
    for(int i = 0; i < msg->data.size(); i++)
    {
        droneVoInfo[i] = msg->data[i];
    }
}

void callBackUavVisionDoll(const std_msgs::Float64MultiArray::ConstPtr& msg)
{

}

void callBackUavVisionMarker(const std_msgs::Float64MultiArray::ConstPtr& msg)
{
    // Insure the drone's status is OK!
    if(uavStatus == uavStateReady || uavStatus == uavStateRunning)
    {
        double dimX = msg->data[3];
        double dimY = msg->data[4];
        //Method 1: Use P-D to control the odrone
        if(droneVoInfo[0] > 0.1 || droneVoInfo[0] < - 0.1)
        {
            for(int i = 0; i < 3; i++)
            {
                cout << droneVoInfo[i] << endl;
            }
            cout << "*********************" << endl << endl;
        }
        //Control_X = KP_X * (dimX) + KD_X * (-Guidance_velocity);
        //Con
        // Assume We get the velocity
        double ctrlX = KP_X * (dimX) + KD_X * (- droneVoInfo[0]);
        double ctrlY = KP_Y * (dimY) + KD_Y * (- droneVoInfo[1]);

        std_msgs::Int32MultiArray pdCtrlMsg;

        pdCtrlMsg.data.push_back(uavMovVelocity);
        pdCtrlMsg.data.push_back((int)(ctrlX * 100));
        pdCtrlMsg.data.push_back((int)(ctrlY * 100));
        pdCtrlMsg.data.push_back(0);
        pdCtrlMsg.data.push_back(0);

        pubOnboard.publish(pdCtrlMsg);
    }
}

void callBackOnboard(const std_msgs::String::ConstPtr& msg)
{
    string strInfo;
    strInfo = "Onboard Callback:" + msg->data;
    ROS_INFO(strInfo.c_str());
}

void uavSetup()
{
    // Step 1: Get the permission of UAV control
    std_msgs::Int32MultiArray initCmd;
    initCmd.data.push_back(uavGetCtrlAbility);
    pubOnboard.publish(initCmd);

    // Step 2: the UAV take off
    initCmd.data.clear();
    initCmd.data.push_back(uavTakeOff);
    pubOnboard.publish(initCmd);
    usleep(50000000);           // Wait the drone setup ok

    // Step 3: the UAV is ready, Let's take missions
    uavStatus = uavStateReady;

}

int main(int argc, char *argv[])
{
    ros::init(argc, argv, "uavCoreLogic");
    ros::NodeHandle nhPub, nhSub;

    ros::param::get("~KP_X", KP_X);
    ros::param::get("~KP_X", KP_Y);
    ros::param::get("~KP_X", KD_X);
    ros::param::get("~KP_X", KD_Y);

    ros::Subscriber subGroundCar;
    ros::Subscriber subUavGuidanceVoInfo;
    ros::Subscriber subUavVisionDoll;
    ros::Subscriber subUavVisionMarker;
    ros::Subscriber subUavOnboard;

    pubServo   = nhPub.advertise<std_msgs::String>("/uav_ctrl/servo",   100);
    pubOnboard = nhPub.advertise<std_msgs::Int32MultiArray>("/uav_ctrl/onboard", 100);

    subGroundCar         = nhSub.subscribe("/uav_parter/buffCar",      100, callBackGroundBuffCar  );
    subUavGuidanceVoInfo = nhSub.subscribe("/uav_guider/velPosByVo",   100, callBackGuidanceVoInfo );
    subUavOnboard        = nhSub.subscribe("/uav_motion/onboard",      100, callBackOnboard        );
    subUavVisionDoll     = nhSub.subscribe("/uav_vision/detectDoll",   100, callBackUavVisionDoll  );
    subUavVisionMarker   = nhSub.subscribe("/uav_vision/detectMarker", 100, callBackUavVisionMarker);

    while (nhPub.ok() && nhSub.ok())
    {
        ros::spinOnce();

    }

    return 0;
}
