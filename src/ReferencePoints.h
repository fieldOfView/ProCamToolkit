#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"

#include "DraggablePoints.h"
#include "SelectablePoints.h"
#include "Mapamok.h"
#include "MeshUtils.h"

class ReferencePoints
{
public:
	ReferencePoints();

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
	Mapamok mapamok;

private:
	cv::Point2f toCv(ofVec2f vec);
	cv::Point3f toCv(ofVec3f vec);
	ofVec2f toOf(cv::Point2f point);
	ofVec3f toOf(cv::Point3f point);

	ofVboMesh referenceMesh;
	SelectablePoints referenceMeshPoints;

	DraggablePoints placedPoints;
	vector<unsigned int> pointIndices;
	vector<cv::Point3f> objectPoints;

	const float selectionMergeTolerance = .01;
	bool dataChanged = false;

	ofMatrix4x4 lastModelViewProjectionMatrix;
};
