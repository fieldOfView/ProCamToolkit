#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "Intrinsics.h"

#ifndef STRINGIFY
#define STRINGIFY(x) #x
#endif

class ofxMapamok {
public:
	ofxMapamok();

	void calibrate(ofRectangle vp, vector<cv::Point2f>& imagePoints, vector<cv::Point3f>& objectPoints, int flags, float aov = 80);
	void setData(cv::Mat1d, cv::Mat rvec, cv::Mat tvec, cv::Size2i imageSize, cv::Mat distortionCoefficients);
	void setViewport(ofRectangle vp);

	void begin();
	void end();

	ofVec3f worldToScreen(ofVec3f WorldXYZ, ofRectangle viewport = ofRectangle());

	void load(string fileName);
	void save(string fileName, string fileNameSummary = "");
	void reset();

	bool calibrationReady;
	float nearDist = 10;
	float farDist = 2000;

private:
	ofMatrix4x4 makeMatrix(cv::Mat rotation, cv::Mat translation);

	cv::Mat rvec, tvec;
	ofMatrix4x4 modelMatrix;
	Intrinsics intrinsics;
	cv::Mat distCoeffs;

	ofRectangle viewport;

	bool useDistortionShader;
	ofShader distortionShader;
	ofFbo distortionBuffer;


	string distortionVertexShader = STRINGIFY(
		#version 120\n

		varying vec2 texCoordVarying;

		void main(void)
		{
			texCoordVarying = gl_MultiTexCoord0.xy;
			gl_Position = ftransform();
		}
	);

	string distortionFragmentShader = STRINGIFY(
		#version 120\n

		varying vec2 texCoordVarying;

		uniform sampler2DRect tex0;

		uniform vec3 k; // radial coefficients k1, k2, k3
		uniform vec3 p; // tangential coefficients p1, p2

		uniform vec2 focalLength;
		uniform vec2 principalPoint;


		void main()
		{
			vec2 lensCoordinates = (texCoordVarying - principalPoint) / focalLength;

			float r_2 = dot(lensCoordinates, lensCoordinates);
			float r_4 = r_2 * r_2;
			float r_6 = r_2 * r_4;
			float _2xy = 2.f * lensCoordinates.x * lensCoordinates.y;

			vec2 distorted = (lensCoordinates / (1.f + k.x * r_2 + k.y * r_4 + k.z * r_6)) + vec2(
				(p.x * _2xy) + p.y * (r_2 + 2.f * lensCoordinates.x * lensCoordinates.x),
				(p.y * _2xy) + p.x * (r_2 + 2.f * lensCoordinates.y * lensCoordinates.y)
			);

			vec2 resultUV = distorted * focalLength + principalPoint;

			gl_FragColor = texture2DRect(tex0, resultUV);
		}
	);
};