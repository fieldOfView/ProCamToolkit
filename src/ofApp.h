#pragma once

#include "ofMain.h"
#include "ofxAssimpModelLoader.h"
#include "ofxAutoControlPanel.h"
#include "MeshUtils.h"
#include "ofxMapamokCalibration.h"
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

	void render();

	void loadCalibration();
	void saveCalibration();
	void resetCalibration();

	ofxAutoControlPanel panel;
	ofVboMesh objectMesh;

	ofLight light;
	AutoShader shader;

private:
	ofxMapamokCalibration referencePoints;
};
