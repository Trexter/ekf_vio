/*
 * Frame.h
 *
 *  Created on: Aug 1, 2017
 *      Author: kevin
 */

#ifndef PAUVSI_VIO_INCLUDE_PAUVSI_VIO_FRAME_H_
#define PAUVSI_VIO_INCLUDE_PAUVSI_VIO_FRAME_H_

#include "../invio/vioParams.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/features2d.hpp>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"
#include "opencv2/video.hpp"
#include "sensor_msgs/CameraInfo.h"

#include <Eigen/Core>

class Frame {

public:

	Eigen::Matrix<double, 3, 3> K;
	cv::Mat img;
	ros::Time t;

	double undistorted_width, undistorted_height; // used for border weight computation.

	Frame();
	Frame(cv::Mat _img, cv::Mat_<float> _k, ros::Time _t);

	virtual ~Frame();

	bool isPixelInBox(cv::Point2f px);
};

#endif /* PAUVSI_VIO_INCLUDE_PAUVSI_VIO_FRAME_H_ */
