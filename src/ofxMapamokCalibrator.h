#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"

#include "DraggablePoints.h"
#include "SelectablePoints.h"
#include "ofxMapamok.h"
#include "MeshUtils.h"

class ofxMapamokCalibrator
{
public:
	ofxMapamokCalibrator();

	void setup(ofMesh mesh);
	void update();
	void draw();

	void setState(bool select);
	void removeSelected();

	void calibrate(int flags);

	void load(string fileName);
	void save(string fileName);
	void reset();

	bool enabled;
	bool selectPoints;

	ofEasyCam camera;
	ofxMapamok mapamok;

private:
	void drawHiddenLine(ofMesh mesh);
	cv::Point2f toCv(ofVec2f vec);
	cv::Point3f toCv(ofVec3f vec);
	ofVec2f toOf(cv::Point2f point);
	ofVec3f toOf(cv::Point3f point);

	ofVboMesh displayMesh;
	ofVboMesh referenceMesh;
	SelectablePoints referenceMeshPoints;

	DraggablePoints placedPoints;
	vector<unsigned int> pointIndices;
	vector<cv::Point3f> objectPoints;

	const float selectionMergeTolerance = .01;

	bool dataChanged = false;
	int lastFlags;
	ofMatrix4x4 lastModelViewProjectionMatrix;
};
