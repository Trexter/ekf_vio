/*
 * vio.h
 *
 *  Created on: Sep 19, 2016
 *      Author: kevinsheridan
 */

#ifndef PAUVSI_VIO_INCLUDE_VIO_H_
#define PAUVSI_VIO_INCLUDE_VIO_H_

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/features2d.hpp>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"
#include "opencv2/video.hpp"
#include <vector>
#include <string>
#include <ros/ros.h>
#include <sensor_msgs/Imu.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/PointCloud.h>
#include "message_filters/subscriber.h"
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <tf2_ros/transform_broadcaster.h>
#include <tf2_ros/message_filter.h>
#include <tf2_ros/transform_listener.h>

#include "EgoMotionEstimator.hpp"
#include "Frame.hpp"
#include "VIOFeature3D.hpp"
#include "VIOFeature2D.hpp"
#include "FeatureTracker.h"
#include "VIOEKF.h"
#include "VIOState.hpp"


#define DEFAULT_CAMERA_TOPIC "/camera/image"
#define DEFAULT_IMU_TOPIC "/IMU_Full"
#define DEFAULT_FAST_THRESHOLD 50
#define DEFAULT_2D_KILL_RADIUS 210
#define DEFAULT_FEATURE_SIMILARITY_THRESHOLD 10
#define DEFAULT_MIN_EIGEN_VALUE 1e-4
#define DEFAULT_NUM_FEATURES 50
#define DEFAULT_MIN_NEW_FEATURE_DIST 10
#define DEFAULT_IMU_FRAME_NAME "imu_frame"
#define DEFAULT_ODOM_FRAME_NAME "odom"
#define DEFAULT_CAMERA_FRAME_NAME "camera_frame"
#define DEFAULT_COM_FRAME_NAME "base_link"
#define DEFAULT_WORLD_FRAME_NAME "world"
#define DEFAULT_GRAVITY_MAGNITUDE 9.8065
#define PI_OVER_180 0.01745329251
#define DEFAULT_RECALIBRATION_THRESHOLD 0.02
#define DEFAULT_QUEUE_SIZE 10
#define DEFAULT_ACTIVE_FEATURES_TOPIC "/pauvsi_vio/activefeatures"
#define DEFAULT_PUBLISH_ACTIVE_FEATURES true
#define DEFAULT_MIN_TRIANGUALTION_DIST 0.1
#define DEFAULT_INIT_PXL_DELTA 1
#define DEFAULT_FRAME_BUFFER_LENGTH 20
#define DEFAULT_MAX_TRIAG_ERROR 2000
#define DEFAULT_MIN_TRIAG_Z 0.02
#define DEFAULT_MIN_TRIAG_FEATURES 40
#define DEFAULT_IDEAL_FUNDAMENTAL_PXL_DELTA 0.3
#define DEFAULT_MIN_FUNDAMENTAL_PXL_DELTA 0.3


class VIO
{

public:

	int FAST_THRESHOLD;
	float KILL_RADIUS;
	int FEATURE_SIMILARITY_THRESHOLD;
	float MIN_EIGEN_VALUE;
	bool KILL_BY_DISSIMILARITY;
	int NUM_FEATURES;
	int MIN_NEW_FEATURE_DISTANCE;
	double GRAVITY_MAG;
	double RECALIBRATION_THRESHOLD;
	bool PUBLISH_ACTIVE_FEATURES;
	double MIN_TRIANGUALTION_DIST;
	double INIT_PXL_DELTA;
	int FRAME_BUFFER_LENGTH;
	double MAX_TRIAG_ERROR;
	double MIN_TRIAG_Z;

	int MIN_TRIAG_FEATURES;
	double IDEAL_FUNDAMENTAL_PXL_DELTA;
	double MIN_FUNDAMENTAL_PXL_DELTA;

	bool initialized;

	std::string ACTIVE_FEATURES_TOPIC;

	//frames
	std::string imu_frame;
	std::string camera_frame;
	std::string odom_frame;
	std::string CoM_frame;
	std::string world_frame;

	VIO();
	~VIO();

	//callbacks
	void imuCallback(const sensor_msgs::ImuConstPtr& msg);
	void cameraCallback(const sensor_msgs::ImageConstPtr& img, const sensor_msgs::CameraInfoConstPtr& cam);

	cv::Mat get3x3FromVector(boost::array<double, 9> vec);

	void correctOrientation(tf::Quaternion q, double certainty);

	void readROSParameters();

	void setCurrentFrame(cv::Mat frame, ros::Time t);

	/*
	 * gets the most recently added frame
	 */
	Frame& currentFrame(){
		return this->frameBuffer.at(0);
	}

	/*
	 * gets the last frame
	 */
	Frame& lastFrame(){
		return this->frameBuffer.at(1);
	}

	/*
	 * returns the camera topic used by this node
	 */
	std::string getCameraTopic(){
		return cameraTopic;
	}
	std::string getIMUTopic(){
		return imuTopic;
	}

	void setK(cv::Mat _K){
		K = _K;
	}

	void setD(cv::Mat _D){
		D = _D;
	}

	void viewImage(cv::Mat img);
	void viewImage(Frame frame);
	void viewMatches(std::vector<VIOFeature2D> ft1, std::vector<VIOFeature2D> ft2, Frame f1, Frame f2, std::vector<cv::Point2f> pt1_new, std::vector<cv::Point2f> pt2_new);

	void broadcastWorldToOdomTF();

	ros::Time broadcastOdomToTempIMUTF(double roll, double pitch, double yaw, double x, double y, double z);

	void publishActivePoints();

	VIOState estimateMotion(VIOState x, Frame frame1, Frame frame2);

	void update3DFeatures();

	double computeFundamentalMatrix(cv::Mat& F, cv::Matx33f& R, cv::Matx31f& t, std::vector<cv::Point2f>& pt1, std::vector<cv::Point2f>& pt2, bool& pass);

	void getBestCorrespondences(double& pixel_delta,  std::vector<VIOFeature2D>& ft1, std::vector<VIOFeature2D>& ft2, VIOState& x1, VIOState& x2, int& match_index);

	VIOFeature2D getCorrespondingFeature(VIOFeature2D currFeature, Frame lastFrame);

	void findBestCorresponding2DFeature(VIOFeature2D start, Frame lf, std::deque<Frame> fb, VIOFeature2D& end, int& frameIndex);

	void run();

	double poseFromPoints(std::vector<VIOFeature3D> points3d, Frame lf, Frame cf, Eigen::Matrix<double, 7, 1>& Z, bool& pass);


	void recalibrateState(double avgPixelChange, double threshold, bool consecutive);

	/*
	 * uses manhattan method to find distance between two pixels
	 */
	float manhattan(cv::Point2f p1, cv::Point2f p2){
		return abs(p2.x - p1.x) + abs(p2.y - p1.y);
	}



	/*	bool visualMotionInference(Frame frame1, Frame frame2, tf::Vector3 angleChangePrediction, tf::Vector3& rotationInference,
				tf::Vector3& unitVelocityInference, double& averageMovement);
	cv::Mat_<double> IterativeLinearLSTriangulation(cv::Point3d u, cv::Matx34d P, cv::Point3d u1, cv::Matx34d P1);
	cv::Mat_<double> LinearLSTriangulation(cv::Point3d u, cv::Matx34d P, cv::Point3d u1, cv::Matx34d P1);
	bool triangulateAndCheck(cv::Point2f pt1, cv::Point2f pt2, cv::Matx33d K1, cv::Matx33d K2, VIOState x1, VIOState x2, double& error, cv::Matx31d& r, tf::Transform base2cam);*/

protected:
	ros::NodeHandle nh;

	//tf2_ros::Buffer tfBuffer;

	image_transport::CameraSubscriber cameraSub;
	ros::Subscriber imuSub;

	ros::Publisher activePointsPub;

	//initialized with default values
	std::string cameraTopic;
	std::string imuTopic;

	//Frame currentFrame; // the current frame
	//Frame lastFrame; //the last frame

	std::deque<Frame> frameBuffer; // holds frames

	FeatureTracker feature_tracker;

	VIOState lastState;
	VIOState state;
	VIOEKF ekf;

	struct gyroNode
	{
		tf::Vector3 gyroBias;
		double certainty;
	};
	struct accelNode
	{
		double accelScale;
		double certainty;
	};
	std::vector<gyroNode> gyroQueue;
	std::vector<accelNode> accelQueue;

	std::vector<VIOFeature3D> active3DFeatures;
	std::vector<VIOFeature3D> inactive3DFeatures;

	cv::Mat K;
	cv::Mat D;

};

#endif /* PAUVSI_VIO_INCLUDE_VIO_LIB_H_ */
