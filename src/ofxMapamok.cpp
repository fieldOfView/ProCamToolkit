#include "ofxMapamok.h"

void ofxMapamok::calibrate(int width, int height, vector<cv::Point2f>& imagePoints, vector<cv::Point3f>& objectPoints, int flags, float aov) {
	int n = imagePoints.size();
	const static int minPoints = 6;
	if (n < minPoints) {
		calibrationReady = false;
		return;
	}
	vector<cv::Mat> rvecs, tvecs;
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
	cv::Mat distortionCoefficients;

	calibrateCamera(objectPointsCv, imagePointsCv, imageSize, cameraMatrix, distortionCoefficients, rvecs, tvecs, flags);
	setData(cameraMatrix, rvecs[0], tvecs[0], imageSize, distortionCoefficients);
}

void ofxMapamok::setData(cv::Mat1d cameraMatrix, cv::Mat rotation, cv::Mat translation, cv::Size2i imageSize, cv::Mat distortionCoefficients) {
	rvec = rotation;
	tvec = translation;
	intrinsics.setup(cameraMatrix, imageSize);
	modelMatrix = makeMatrix(rvec, tvec);
	distCoeffs = distortionCoefficients;

	calibrationReady = true;
}

void ofxMapamok::begin() {
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

void ofxMapamok::end() {
	if (!calibrationReady) {
		return;
	}
	ofPopMatrix();
	ofSetMatrixMode(OF_MATRIX_PROJECTION);
	ofPopMatrix();
	ofSetMatrixMode(OF_MATRIX_MODELVIEW);
}

ofVec3f ofxMapamok::worldToScreen(ofVec3f WorldXYZ, ofRectangle viewport) {
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


void ofxMapamok::load(string fileName) {
	// load the calibration-advanced yml
	cv::FileStorage fs(ofToDataPath(fileName, true), cv::FileStorage::READ);

	cv::Mat cameraMatrix;
	cv::Size2i imageSize;
	cv::Mat rotation, translation;
	cv::Mat distortionCoefficients;
	fs["cameraMatrix"] >> cameraMatrix;
	fs["imageSize"][0] >> imageSize.width;
	fs["imageSize"][1] >> imageSize.height;
	fs["rotationVector"] >> rotation;
	fs["translationVector"] >> translation;
	fs["distCoeffs"] >> distortionCoefficients;

	if (imageSize.width != 0 && imageSize.height != 0) {
		setData(cameraMatrix, rotation, translation, imageSize, distortionCoefficients);
	}
	else {
		ofLogError() << "calibration does not contain image size";
	}
}

void ofxMapamok::save(string fileName, string fileNameSummary) {
	if (!calibrationReady) {
		ofLogWarning() << "not enough points set to save a calibration";
		return;
	}

	cv::FileStorage fs(ofToDataPath(fileName), cv::FileStorage::WRITE);
	if (!fs.isOpened()) {
		ofLogError() << "could not open calibration file for writing";
		return;
	}

	cv::Mat cameraMatrix = intrinsics.getCameraMatrix();
	fs << "cameraMatrix" << cameraMatrix;

	double focalLength = intrinsics.getFocalLength();
	fs << "focalLength" << focalLength;

	cv::Point2d fov = intrinsics.getFov();
	fs << "fov" << fov;
	fs << "distCoeffs" << distCoeffs;

	cv::Point2d principalPoint = intrinsics.getPrincipalPoint();
	fs << "principalPoint" << principalPoint;

	cv::Size imageSize = intrinsics.getImageSize();
	fs << "imageSize" << imageSize;

	fs << "translationVector" << tvec;
	fs << "rotationVector" << rvec;

	cv::Mat rotationMatrix;
	Rodrigues(rvec, rotationMatrix);
	fs << "rotationMatrix" << rotationMatrix;

	double rotationAngleRadians = norm(rvec, cv::NORM_L2);
	double rotationAngleDegrees = ofRadToDeg(rotationAngleRadians);
	cv::Mat rotationAxis = rvec / rotationAngleRadians;
	fs << "rotationAngleRadians" << rotationAngleRadians;
	fs << "rotationAngleDegrees" << rotationAngleDegrees;
	fs << "rotationAxis" << rotationAxis;

	ofVec3f axis(rotationAxis.at<double>(0), rotationAxis.at<double>(1), rotationAxis.at<double>(2));
	ofVec3f euler = ofQuaternion(rotationAngleDegrees, axis).getEuler();
	cv::Mat eulerMat = (cv::Mat_<double>(3, 1) << euler.x, euler.y, euler.z);
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
	basic << "\tx: " << ofToString(imageSize.width, 2) << endl;
	basic << "\ty: " << ofToString(imageSize.height, 2) << endl;
	basic << "distortion coefficients:" << endl;
	basic << "\tradial: " << ofToString(distCoeffs.at<double>(0), 4) << " " << ofToString(distCoeffs.at<double>(1), 4) << " " << ofToString(distCoeffs.at<double>(4), 4) << endl;
	basic << "\ttangential: " << ofToString(distCoeffs.at<double>(2), 4) << " " << ofToString(distCoeffs.at<double>(3), 4) << endl;
}

void ofxMapamok::reset() {
	calibrationReady = false;

	rvec = cv::Mat();
	tvec = cv::Mat();
	modelMatrix = ofMatrix4x4();
	intrinsics = Intrinsics();
	distCoeffs = cv::Mat();

}

ofMatrix4x4 ofxMapamok::makeMatrix(cv::Mat rotation, cv::Mat translation) {
	cv::Mat rot3x3;
	if (rotation.rows == 3 && rotation.cols == 3) {
		rot3x3 = rotation;
	}
	else {
		Rodrigues(rotation, rot3x3);
	}
	double* rm = rot3x3.ptr<double>(0);
	double* tm = translation.ptr<double>(0);
	return ofMatrix4x4(rm[0], rm[3], rm[6], 0.0f,
		rm[1], rm[4], rm[7], 0.0f,
		rm[2], rm[5], rm[8], 0.0f,
		tm[0], tm[1], tm[2], 1.0f);
}