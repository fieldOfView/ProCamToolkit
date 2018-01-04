#pragma once
#include "ofxOpenCv.h"

class Intrinsics {
public:
	void setup(float focalLengthMm, cv::Size imageSizePx, cv::Size2f sensorSizeMm, cv::Point2d principalPointPct = cv::Point2d(.5, .5));
	void setup(cv::Mat cameraMatrix, cv::Size imageSizePx, cv::Size2f sensorSizeMm = cv::Size2f(0, 0));
	void setImageSize(cv::Size imgSize);
	cv::Mat getCameraMatrix() const;
	cv::Size getImageSize() const;
	cv::Size2f getSensorSize() const;
	cv::Point2d getFov() const;
	double getFocalLength() const;
	double getAspectRatio() const;
	cv::Point2d getPrincipalPoint() const;
	void loadProjectionMatrix(float nearDist = 10., float farDist = 10000., cv::Point2d viewportOffset = cv::Point2d(0, 0)) const;
protected:
	void updateValues();
	cv::Mat cameraMatrix;
	cv::Size imageSize;
	cv::Size2f sensorSize;
	cv::Point2d fov;
	double focalLength, aspectRatio;
	cv::Point2d principalPoint;
};
