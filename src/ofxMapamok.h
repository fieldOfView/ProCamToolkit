#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "Intrinsics.h"

class ofxMapamok {
public:
	bool calibrationReady;
	float nearDist = 10;
	float farDist = 2000;

	void calibrate(int width, int height, vector<cv::Point2f>& imagePoints, vector<cv::Point3f>& objectPoints, int flags, float aov = 80);

	void begin();
	void end();

	ofVec3f worldToScreen(ofVec3f WorldXYZ, ofRectangle viewport = ofRectangle());
	
	void load(string fileName);
	void save(string fileName, string fileNameSummary = "");
	void reset();

private:
	ofMatrix4x4 makeMatrix(cv::Mat rotation, cv::Mat translation);

	cv::Mat rvec, tvec;
	ofMatrix4x4 modelMatrix;
	Intrinsics intrinsics;
};