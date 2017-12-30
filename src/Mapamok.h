#pragma once

#include "ofMain.h"
#include "ofxCv.h"

class Mapamok {
public:
	bool calibrationReady;
	float nearDist = 10;
	float farDist = 2000;

	void calibrate(int width, int height, vector<cv::Point2f>& imagePoints, vector<cv::Point3f>& objectPoints, int flags, float aov = 80);

	void begin();

	void end();

	void load(string fileName);
	void save(string fileName, string fileNameSummary = "");
	void reset();

private:
	cv::Mat rvec, tvec;
	ofMatrix4x4 modelMatrix;
	ofxCv::Intrinsics intrinsics;
};