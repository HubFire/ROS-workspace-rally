#include "ros/ros.h"
#include "std_msgs/String.h"

int main(int argc, char **argv){
	ros::init(argc, argv, "publisher");
	ros::NodeHandle n;
	ros::Publisher chatter_pub = n.advertise<std_msgs::String>("Reached", 3000);
	ros::Rate loop_rate(3);

	while(ros::ok()){
		std_msgs::String msg;
		msg.data = "TRUE";
		chatter_pub.publish(msg);
	
	}

}
