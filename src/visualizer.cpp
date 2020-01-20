#include <iostream>

#include <ros/ros.h>
#include <visualization_msgs/Marker.h>
#include <nav_msgs/Odometry.h>
#include <cmath>
#include <fstream>
#include <string>

class TrajectoryVisualizer {
public:
	TrajectoryVisualizer(void):private_nh_("~") {
		initSetup();	
	}
	void initSetup(void) {
		waypoint_pub_ = nh_.advertise<visualization_msgs::Marker>("/waypoint", 1);
		trajectory_pub_ = nh_.advertise<visualization_msgs::Marker>("/trajectory", 1);
		odom_sub_ = nh_.subscribe("/odom", 10, &TrajectoryVisualizer::odomCallback, this);
		initParam();
		getWaypoints();
		run();
	}
	void initParam(void) {
		private_nh_.getParam("file_path", file_path_);
		private_nh_.getParam("pose_x_offset", pose_x_offset_);
		private_nh_.getParam("pose_y_offset", pose_y_offset_);
		private_nh_.getParam("waypoint_x_offset", waypoint_x_offset_);
		private_nh_.getParam("waypoint_y_offset", waypoint_y_offset_);
	}
	void odomCallback(const nav_msgs::OdometryConstPtr &odom_msg) {
		visualization_msgs::Marker trajectory;
		geometry_msgs::Point temp_pose;
		
		temp_pose.x = odom_msg->pose.pose.position.x - pose_x_offset_;
		temp_pose.y = odom_msg->pose.pose.position.y - pose_y_offset_;
		
		vehicle_pose_.push_back(temp_pose);
			
		trajectory.header.frame_id = "map";
		trajectory.header.stamp.fromSec(ros::Time::now().toSec());
		trajectory.ns = "trajectory";
		trajectory.id = 0;
		trajectory.action = visualization_msgs::Marker::ADD;
		trajectory.pose.orientation.w = 1.0;
		// visualization_msgs::Marker::LINE_STRIP
		// visualization_msgs::Marker::SPHERE_LIST
		trajectory.type = visualization_msgs::Marker::LINE_STRIP;
		trajectory.scale.x = 0.1;
		trajectory.color.g = 1.0f;
		trajectory.color.a = 1.0f;

		trajectory.points = vehicle_pose_;		

		std::cout<<"CURRENT TRAJECTORY CONSISTS OF "<<trajectory.points.size()<<"POSES."<<std::endl;

		trajectory_pub_.publish(trajectory);
	}	
	void getWaypoints(void) {
		std::string first_str_buf, second_str_buf, third_str_buf, fourth_str_buf;
		int first_seg_pos, second_seg_pos, third_seg_pos, fourth_seg_pos; 
		geometry_msgs::Point temp_waypoint;

		is_.open(file_path_);
		
		if(!is_.is_open()) {
			ROS_ERROR("CANNOT FIND [%s], PLEASE CHECK WHETHER IT IS CORRECT PATH.", file_path_.c_str());
			ros::shutdown();
		} else {	
			while(!is_.eof()) {
				getline(is_, first_str_buf);
				if(first_str_buf != "") {
					first_seg_pos = first_str_buf.find(",");

					second_str_buf = first_str_buf.substr(first_seg_pos+1);
					second_seg_pos = second_str_buf.find(",");
					temp_waypoint.x = std::stof(second_str_buf.substr(0, second_seg_pos))-waypoint_x_offset_;
				
					third_str_buf = second_str_buf.substr(second_seg_pos+1);
					third_seg_pos = third_str_buf.find(",");
					temp_waypoint.y = std::stof(third_str_buf.substr(0, third_seg_pos))-waypoint_y_offset_;
			
					waypoints_.push_back(temp_waypoint);
				}
			} 
			is_.close();
		
			std::cout<<waypoints_.size()<<" WAYPOINTS SUCCESSFULLY LOADED."<<std::endl;
		}
	}
	void run(void) {
		ros::Rate loop_rate(10);
		
		while(ros::ok()) {
			visualization_msgs::Marker waypoint_list;
			
			waypoint_list.header.frame_id = "map";
			waypoint_list.header.stamp.fromSec(ros::Time::now().toSec());
			waypoint_list.ns = "waypoint";
			waypoint_list.id = 0;
			waypoint_list.action = visualization_msgs::Marker::ADD;
			waypoint_list.pose.orientation.w = 1.0;
			// visualization_msgs::Marker::LINE_STRIP
			// visualization_msgs::Marker::SPHERE_LIST
			waypoint_list.type = visualization_msgs::Marker::SPHERE_LIST;
			waypoint_list.scale.x = 0.1;
			waypoint_list.scale.y = 0.1;
			waypoint_list.scale.z = 0.1;
			waypoint_list.color.r = 1.0f;
			waypoint_list.color.a = 1.0f;	

			for(int i=0;i<waypoints_.size();i++) {
				waypoint_list.points.push_back(waypoints_[i]);
			}	
	
			waypoint_pub_.publish(waypoint_list);
			
			ros::spinOnce();

			loop_rate.sleep();
		}
	}
private:
	ros::NodeHandle nh_;
	ros::NodeHandle private_nh_;
	ros::Publisher waypoint_pub_;
	ros::Publisher trajectory_pub_;
	ros::Subscriber odom_sub_;
	
	std::ifstream is_;

	std::string file_path_;
	std::vector<geometry_msgs::Point> waypoints_;
	std::vector<geometry_msgs::Point> vehicle_pose_;
	
	int pose_x_offset_, pose_y_offset_;
	int waypoint_x_offset_, waypoint_y_offset_;
};

int main(int argc, char** argv) {
	ros::init(argc, argv, "trajectory_visualizer_node");

	TrajectoryVisualizer tv;
}
