#include "ros/ros.h"
#include "geometry_msgs/Twist.h"
#include "RoboMap/behavior.h"
#include <functional>
#include <queue>
#include <vector>

//define priority of behaviors here
#define PRIORITY_SEEK 0
#define PRIORITY_AVOID 1

void callback_wander(const RoboMap::behavior::ConstPtr& msg);
void callback_avoid(const RoboMap::behavior::ConstPtr& msg);
/*void callback_seek(const RoboMap::behavior::ConstPtr& msg);*/
/*void callback_detect(const RoboMap::behavior::ConstPtr& msg);*/
geometry_msgs::Twist msg_move;
RoboMap::behavior msg_bh;
ros::Publisher pub_vel;

//define comparison operator here
bool compare_priorities(std::pair<int, RoboMap::behavior> a,
             std::pair<int, RoboMap::behavior> b) {
    return a.first < b.first;
}

//create priority queue
std::priority_queue< std::pair<int, RoboMap::behavior>,
            std::vector<std::pair<int, RoboMap::behavior> >,
            decltype(&compare_priorities)> behavior_queue{compare_priorities};

//translate behavior message to twist
void move_robot(RoboMap::behavior& msg) {
    geometry_msgs::Twist msg_move;
    msg_move.linear.x = msg.vel_fw;
    msg_move.angular.z = msg.vel_turn;
    pub_vel.publish(msg_move);
}

void stop_robot() {
    geometry_msgs::Twist msg_move;
    msg_move.linear.x = 0;
    msg_move.angular.z = 0;
    pub_vel.publish(msg_move);
}

//process all behavior messages
void process_behaviors() {
    if (!behavior_queue.empty()) {
        RoboMap::behavior priority_msg = behavior_queue.top().second;
        move_robot(priority_msg);
        while(behavior_queue.size() > 0) {
            //C++ Priority Queue has no clear function,
            //so we loop to pop off the old messages
            behavior_queue.pop();
        }
    }
    else {
        stop_robot();
    }
}

int main(int argc, char** argv) {
	ros::init(argc, argv, "arbiter_node");
	ROS_INFO("Starting arbiter node.");
	ros::NodeHandle nh;
	ros::Rate loop_rate(30);

    ros::Subscriber sub_wander = nh.subscribe("behavior/wander", 1, callback_wander);
    ros::Subscriber sub_avoid = nh.subscribe("behavior/avoid", 1, callback_avoid);
	/*ros::Subscriber sub_seek = nh.subscribe("behavior/seek", 1, callback_seek);*/
	/*ros::Subscriber sub_avoid = nh.subscribe("behavior/detect", 1, callback_detect);*/

	pub_vel = nh.advertise<geometry_msgs::Twist>("irobot/cmd_vel", 1);



	while(ros::ok()) {
		
		process_behaviors();

		ros::spinOnce();
		loop_rate.sleep();
	}

	return 0;
}

//callbacks for behaviors

void callback_wander (const RoboMap::behavior::ConstPtr& msg) {
    if (msg->active) {
        behavior_queue.push(std::pair<int, RoboMap::behavior>(PRIORITY_SEEK, *msg));
    }
    ROS_INFO("Arbiter: Wander(%s) Fw: %.1f Turn: %.1f", msg->active ? "on" : "off", msg->vel_fw, msg->vel_turn);
}

void callback_avoid (const RoboMap::behavior::ConstPtr& msg) {
    if (msg->active) {
        behavior_queue.push(std::pair<int, RoboMap::behavior>(PRIORITY_AVOID, *msg));
    }
    ROS_INFO("Arbiter: Avoid(%s) Fw: %.1f Turn: %.1f", msg->active ? "on" : "off", msg->vel_fw, msg->vel_turn);
}

/*void callback_seek (const RoboMap::behavior::ConstPtr& msg) {
	if (msg->active) {
        behavior_queue.push(std::pair<int, RoboMap::behavior>(PRIORITY_SEEK, *msg));
    }
    ROS_INFO("Arbiter: Seek(%s) Fw: %.1f Turn: %.1f", msg->active ? "on" : "off", msg->vel_fw, msg->vel_turn);
}*/

/*void callback_detect (const RoboMap::behavior::ConstPtr& msg) {
	if (msg->active) {
        behavior_queue.push(std::pair<int, RoboMap::behavior>(PRIORITY_AVOID, *msg));
    }
    ROS_INFO("Arbiter: Detect(%s) Fw: %.1f Turn: %.1f", msg->active ? "on" : "off", msg->vel_fw, msg->vel_turn);
}*/