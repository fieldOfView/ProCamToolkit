#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxAssimpModelLoader.h"
#include "ofxAutoControlPanel.h"
#include "Mapamok.h"
#include "SelectablePoints.h"
#include "DraggablePoints.h"
#include "LineArt.h"
#include "AutoShader.h"

class ofApp : public ofBaseApp {
public:
	void setb(string name, bool value);
	void seti(string name, int value);
	void setf(string name, float value);
	bool getb(string name);
	int geti(string name);
	float getf(string name);

	void setup();
	void update();
	void draw();
	void keyPressed(int key);

	void setupControlPanel();
	void setupMesh(string fileName);

	void updateSelectionMode();
	void updateRenderMode();
	void drawSelectionMode();
	void drawRenderMode();
	void render();

	void loadCalibration();
	void saveCalibration();
	void resetCalibration();

	Mapamok mapamok;
	ofEasyCam cam;
	ofVboMesh objectMesh;
	ofVboMesh referenceMesh;
	ofMesh imageMesh;
	ofLight light;
	ofxAutoControlPanel panel;

	vector<cv::Point3f> objectPoints;
	vector<unsigned int> pointIndices;

	DraggablePoints placedPoints;
	SelectablePoints referenceMeshPoints;

	AutoShader shader;

private:
	const float selectionMergeTolerance = .01;
	bool dataChanged = false;
};
