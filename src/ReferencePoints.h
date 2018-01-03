#pragma once

#include "ofMain.h"
#include "ofxCv.h"

#include "DraggablePoints.h"
#include "SelectablePoints.h"
#include "ofxCv.h"
#include "Mapamok.h"

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
	ofVboMesh referenceMesh;
	SelectablePoints referenceMeshPoints;

	DraggablePoints placedPoints;
	vector<unsigned int> pointIndices;
	vector<cv::Point3f> objectPoints;

	const float selectionMergeTolerance = .01;
	bool dataChanged = false;

	ofMatrix4x4 lastModelViewProjectionMatrix;
};
