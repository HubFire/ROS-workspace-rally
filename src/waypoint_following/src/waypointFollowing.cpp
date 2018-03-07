/**********************************
* Description:  Takes position info from AMCL and uses a PID controller to follow hard-coded waypoints.
* Author:       Nicole Chan
* Created:      5/6/2017
**********************************/
#include <iostream>
#include <vector>
#include <sstream>
#include <math.h>
// ROS
#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/Point.h>
#include <race/drive_param.h>
#include <std_msgs/Float64.h>

// Global vars
//#define NUM_POINTS 10   	// number of total waypoints to follow
#define MAX_RERROR 0.1  	// when the car is within this dist, it has "reached" the waypoint
#define MIN_RERROR 2.5		// a point is reached if within this dist AND the point is behind the car
race::drive_param pwm_msg; 	// global because multiple CB functions will use it
int radius_error = 0;
int theta_error = 0;
double last_error = 0;
double theta_integral = 0;
float Kp = 0.8;
float Kd = 0;
float Ki = 0;
float freq = 100; 		// update if timer frequency is changed

class waypointFollower
{
  ros::NodeHandle nh;
  ros::Subscriber amclPose_sub;
  ros::Publisher pwm_pub;
  ros::Timer timer;
  std::vector<geometry_msgs::Point> waypoints;
  std::vector<geometry_msgs::Point>::iterator currPt;

public:
  waypointFollower(){
    // Subscribe to amcl output topic and publish output drive pwm values
    amclPose_sub = nh.subscribe("slam_out_pose",1,&waypointFollower::poseCb,this);
    pwm_pub = nh.advertise<race::drive_param>("drive_parameters", 10);
    timer = nh.createTimer(ros::Duration(0.5), &waypointFollower::controlCb,this); // timer cb every 0.01 sec
    
    // Generate desired waypoints
    waypoints.resize(10);
	waypoints[0].x = 4.02287483215;
	waypoints[0].y = 2.29184675217;

    currPt = waypoints.begin();
  }

  void controlCb(const ros::TimerEvent&){
    pwm_pub.publish(pwm_msg);
  }

  void poseCb(const geometry_msgs::PoseStamped &msg){
    // If already completed desired waypoints
    if(currPt==waypoints.end()){
       pwm_msg.velocity = 9780;
       pwm_msg.angle = 0;
       return;
    }

    // Get pose from hector_mapping
    geometry_msgs::Point currPos = msg.pose.position; 		// x,y,z coordinate
    double radius = sqrt(pow(currPos.x,2)+pow(currPos.y,2)); 	// position vector in map frame
    double theta1 = 2*acos(msg.pose.orientation.w); 		// car's heading (+x-dir) in map

    double theta2 = 2*asin(msg.pose.orientation.z); 

    double theta;
	if(theta2 < 0)
		theta = -theta1;
	else
		theta = theta1;

    // Compute relative position from waypoint (i.e. car at origin)
    //double ref_rad = sqrt(pow((*currPt)->x,2)+pow((*currPt)->y,2));

    double rel_x = (currPt)->x-currPos.x;
    double rel_y = (currPt)->y-currPos.y;
    double rel_rad = sqrt(pow(rel_x,2)+pow(rel_y,2));
	double ref_theta = atan2(rel_y,rel_x);
    double rel_theta = ref_theta-theta; // TODO: check if the axes should be swapped
	if(rel_theta < -M_PI)
		rel_theta += 2*M_PI;
	else if (rel_theta > M_PI)
		rel_theta -= 2*M_PI;    
    
    // Update waypoint, if necessary
    if((rel_rad < MAX_RERROR) || (rel_rad < MIN_RERROR && abs(rel_theta) > 1.57)){
       currPt++; 
    }
	std::cout << "ref_theta = " << ref_theta << std::endl;
	std::cout << "theta = " << theta << std::endl;
	std::cout << "rel_theta = " << rel_theta << std::endl;
	std::cout << std::endl;	

    // Navigate to waypoint using PID for angle (bang-bang for velocity)
    last_error = theta_error;
    radius_error = rel_rad;
    theta_error = rel_theta;
    theta_integral = theta_integral + theta_error;

    // Compute steering angle
    double pid = (Kp*theta_error+Kd*(theta_error-last_error)*freq+Ki*theta_integral)*(10)/(6);

    // Reverse if the waypoint is behind the car
    if (abs(rel_theta) > 1.57){
      pwm_msg.velocity = 9480;
      pwm_msg.angle = (int)pid;
    }
    // Accelerate if the waypoint is in front of the car
    else{
      pwm_msg.velocity = 10080;
      pwm_msg.angle = -(int)pid;
    }

	std::cout << "PID equals " << pid << std::endl;
    //else{
    //  pwm_msg.velocity = 9780;
    //}

  }
};

int main(int argc, char** argv)
{
  ros::init(argc, argv, "waypointFollower");
  waypointFollower wpfollow;
  ros::spin();
  return 0;
}
