#include "Mapamok.h"

using namespace ofxCv;
using namespace cv;

void Mapamok::calibrate(int width, int height, vector<cv::Point2f>& imagePoints, vector<cv::Point3f>& objectPoints, int flags, float aov) {
	int n = imagePoints.size();
	const static int minPoints = 6;
	if (n < minPoints) {
		calibrationReady = false;
		return;
	}
	vector<cv::Mat> rvecs, tvecs;
	cv::Mat distCoeffs;
	vector<vector<cv::Point3f> > objectPointsCv;
	vector<vector<cv::Point2f> > imagePointsCv;
	objectPointsCv.push_back(objectPoints);
	imagePointsCv.push_back(imagePoints);

	cv::Size2i imageSize(width, height);
	float f = imageSize.width * ofDegToRad(aov); // this might be wrong, but it's optimized out
	cv::Point2f c = cv::Point2f(imageSize) * (1. / 2);
	cv::Mat1d cameraMatrix = (cv::Mat1d(3, 3) <<
		f, 0, c.x,
		0, f, c.y,
		0, 0, 1);

	calibrateCamera(objectPointsCv, imagePointsCv, imageSize, cameraMatrix, distCoeffs, rvecs, tvecs, flags);
	rvec = rvecs[0];
	tvec = tvecs[0];
	intrinsics.setup(cameraMatrix, imageSize);
	modelMatrix = ofxCv::makeMatrix(rvec, tvec);
	calibrationReady = true;
}

void Mapamok::begin() {
	if (!calibrationReady) {
		return;
	}
	ofPushMatrix();
	ofSetMatrixMode(OF_MATRIX_PROJECTION);
	ofPushMatrix();
	ofSetMatrixMode(OF_MATRIX_MODELVIEW);
	intrinsics.loadProjectionMatrix(nearDist, farDist);
	ofMultMatrix(modelMatrix);
}

void Mapamok::end() {
	if (!calibrationReady) {
		return;
	}
	ofPopMatrix();
	ofSetMatrixMode(OF_MATRIX_PROJECTION);
	ofPopMatrix();
	ofSetMatrixMode(OF_MATRIX_MODELVIEW);
}

ofVec3f Mapamok::worldToScreen(ofVec3f WorldXYZ, ofRectangle viewport) {
	if (!calibrationReady) {
		return WorldXYZ;
	}

	if (viewport.isZero()) {
		viewport = ofGetCurrentViewport();
	}

	begin();
	ofMatrix4x4 modelViewMatrix = ofGetCurrentMatrix(OF_MATRIX_MODELVIEW);
	ofMatrix4x4 projectionMatrix = ofGetCurrentMatrix(OF_MATRIX_PROJECTION);
	end();

	ofVec3f CameraXYZ = WorldXYZ * modelViewMatrix * projectionMatrix;
	ofVec3f ScreenXYZ;

	ScreenXYZ.x = (CameraXYZ.x + 1.0f) / 2.0f * viewport.width + viewport.x;
	ScreenXYZ.y = (1.0f - CameraXYZ.y) / 2.0f * viewport.height + viewport.y;

	ScreenXYZ.z = CameraXYZ.z;

	return ScreenXYZ;
}


void Mapamok::load(string fileName) {
	// load the calibration-advanced yml
	FileStorage fs(ofToDataPath(fileName, true), FileStorage::READ);

	Mat cameraMatrix;
	Size2i imageSize;
	fs["cameraMatrix"] >> cameraMatrix;
	fs["imageSize"][0] >> imageSize.width;
	fs["imageSize"][1] >> imageSize.height;
	fs["rotationVector"] >> rvec;
	fs["translationVector"] >> tvec;

	if (imageSize.width != 0 && imageSize.height != 0) {
		intrinsics.setup(cameraMatrix, imageSize);
		modelMatrix = makeMatrix(rvec, tvec);

		calibrationReady = true;
	}
	else {
		ofLogError() << "calibration does not contain image size";
	}

}

void Mapamok::save(string fileName, string fileNameSummary) {
	if (!calibrationReady) {
		ofLogWarning() << "not enough points set to save a calibration";
		return;
	}

	FileStorage fs(ofToDataPath(fileName), FileStorage::WRITE);
	if (!fs.isOpened()) {
		ofLogError() << "could not open calibration file for writing";
		return;
	}

	Mat cameraMatrix = intrinsics.getCameraMatrix();
	fs << "cameraMatrix" << cameraMatrix;

	double focalLength = intrinsics.getFocalLength();
	fs << "focalLength" << focalLength;

	Point2d fov = intrinsics.getFov();
	fs << "fov" << fov;

	Point2d principalPoint = intrinsics.getPrincipalPoint();
	fs << "principalPoint" << principalPoint;

	cv::Size imageSize = intrinsics.getImageSize();
	fs << "imageSize" << imageSize;

	fs << "translationVector" << tvec;
	fs << "rotationVector" << rvec;

	Mat rotationMatrix;
	Rodrigues(rvec, rotationMatrix);
	fs << "rotationMatrix" << rotationMatrix;

	double rotationAngleRadians = norm(rvec, NORM_L2);
	double rotationAngleDegrees = ofRadToDeg(rotationAngleRadians);
	Mat rotationAxis = rvec / rotationAngleRadians;
	fs << "rotationAngleRadians" << rotationAngleRadians;
	fs << "rotationAngleDegrees" << rotationAngleDegrees;
	fs << "rotationAxis" << rotationAxis;

	ofVec3f axis(rotationAxis.at<double>(0), rotationAxis.at<double>(1), rotationAxis.at<double>(2));
	ofVec3f euler = ofQuaternion(rotationAngleDegrees, axis).getEuler();
	Mat eulerMat = (Mat_<double>(3, 1) << euler.x, euler.y, euler.z);
	fs << "euler" << eulerMat;

	if (fileNameSummary == "") {
		// skip summary if no summary filename supplied
		ofLogWarning() << "could not open summary file for writing";
		return;
	}

	ofFile basic(fileNameSummary, ofFile::WriteOnly);
	if (!basic.is_open()) {
		ofLogWarning() << "could not open summary file for writing";
		return;
	}
	ofVec3f position(tvec.at<double>(1), tvec.at<double>(2));
	basic << "position (in world units):" << endl;
	basic << "\tx: " << ofToString(tvec.at<double>(0), 2) << endl;
	basic << "\ty: " << ofToString(tvec.at<double>(1), 2) << endl;
	basic << "\tz: " << ofToString(tvec.at<double>(2), 2) << endl;
	basic << "axis-angle rotation (in degrees):" << endl;
	basic << "\taxis x: " << ofToString(axis.x, 2) << endl;
	basic << "\taxis y: " << ofToString(axis.y, 2) << endl;
	basic << "\taxis z: " << ofToString(axis.z, 2) << endl;
	basic << "\tangle: " << ofToString(rotationAngleDegrees, 2) << endl;
	basic << "euler rotation (in degrees):" << endl;
	basic << "\tx: " << ofToString(euler.x, 2) << endl;
	basic << "\ty: " << ofToString(euler.y, 2) << endl;
	basic << "\tz: " << ofToString(euler.z, 2) << endl;
	basic << "fov (in degrees):" << endl;
	basic << "\thorizontal: " << ofToString(fov.x, 2) << endl;
	basic << "\tvertical: " << ofToString(fov.y, 2) << endl;
	basic << "principal point (in screen units):" << endl;
	basic << "\tx: " << ofToString(principalPoint.x, 2) << endl;
	basic << "\ty: " << ofToString(principalPoint.y, 2) << endl;
	basic << "image size (in pixels):" << endl;
	basic << "\tx: " << ofToString(principalPoint.x, 2) << endl;
	basic << "\ty: " << ofToString(principalPoint.y, 2) << endl;
}

void Mapamok::reset() {
	calibrationReady = false;
}