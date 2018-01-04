#include "Intrinsics.h"

void Intrinsics::setup(float focalLength, cv::Size imageSize, cv::Size2f sensorSize, cv::Point2d principalPoint) {
	float focalPixels = (focalLength / sensorSize.width) * imageSize.width;
	float fx = focalPixels; // focal length in pixels on x
	float fy = focalPixels;  // focal length in pixels on y
	float cx = imageSize.width * principalPoint.x;  // image center in pixels on x
	float cy = imageSize.height * principalPoint.y;  // image center in pixels on y
	cv::Mat cameraMatrix = (cv::Mat1d(3, 3) <<
		fx, 0, cx,
		0, fy, cy,
		0, 0, 1);
	setup(cameraMatrix, imageSize, sensorSize);
}
void Intrinsics::setup(cv::Mat cameraMatrix, cv::Size imageSize, cv::Size2f sensorSize) {
	this->cameraMatrix = cameraMatrix;
	this->imageSize = imageSize;
	this->sensorSize = sensorSize;
	updateValues();
}

void Intrinsics::updateValues() {
	calibrationMatrixValues(cameraMatrix,
		imageSize,
		sensorSize.width, sensorSize.height,
		fov.x, fov.y,
		focalLength,
		principalPoint, // sets principalPoint in mm
		aspectRatio);
}

void Intrinsics::setImageSize(cv::Size imgSize) {
	imageSize = imgSize;
}

cv::Mat Intrinsics::getCameraMatrix() const {
	return cameraMatrix;
}

cv::Size Intrinsics::getImageSize() const {
	return imageSize;
}

cv::Size2f Intrinsics::getSensorSize() const {
	return sensorSize;
}

cv::Point2d Intrinsics::getFov() const {
	return fov;
}

double Intrinsics::getFocalLength() const {
	return focalLength;
}

double Intrinsics::getAspectRatio() const {
	return aspectRatio;
}

cv::Point2d Intrinsics::getPrincipalPoint() const {
	return principalPoint;
}

void Intrinsics::loadProjectionMatrix(float nearDist, float farDist, cv::Point2d viewportOffset) const {
	ofViewport(viewportOffset.x, viewportOffset.y, imageSize.width, imageSize.height);
	ofSetMatrixMode(OF_MATRIX_PROJECTION);
	ofLoadIdentityMatrix();
	float w = imageSize.width;
	float h = imageSize.height;
	float fx = cameraMatrix.at<double>(0, 0);
	float fy = cameraMatrix.at<double>(1, 1);
	float cx = principalPoint.x;
	float cy = principalPoint.y;

	ofMatrix4x4 frustum;
	frustum.makeFrustumMatrix(
		nearDist * (-cx) / fx, nearDist * (w - cx) / fx,
		nearDist * (cy) / fy, nearDist * (cy - h) / fy,
		nearDist, farDist);
	ofMultMatrix(frustum);

	ofSetMatrixMode(OF_MATRIX_MODELVIEW);
	ofLoadIdentityMatrix();

	ofMatrix4x4 lookAt;
	lookAt.makeLookAtViewMatrix(ofVec3f(0, 0, 0), ofVec3f(0, 0, 1), ofVec3f(0, -1, 0));
	ofMultMatrix(lookAt);
}
