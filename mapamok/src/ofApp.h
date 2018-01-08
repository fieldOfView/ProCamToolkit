#pragma once

#include "ofMain.h"
#include "ofxAssimpModelLoader.h"
#include "ofxAutoControlPanel.h"
#include "ofxMapamokCalibrator.h"
#include "LineArt.h"
#include "AutoShader.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void keyPressed(int key);
	void dragEvent(ofDragInfo dragInfo);

	void setupControlPanel();
	void loadModel(string fileName);

	void render();

	void loadCalibration();
	void saveCalibration();
	void resetCalibration();

	ofxAutoControlPanel panel;
	ofVboMesh objectMesh;

	ofLight light;
	AutoShader shader;

private:
	ofxMapamokCalibrator calibrator;

	void setb(string name, bool value);
	void seti(string name, int value);
	void setf(string name, float value);
	bool getb(string name);
	int geti(string name);
	float getf(string name);

	ofRectangle makeViewport();
};
