#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxAssimpModelLoader.h"
#include "ofxProCamToolkit.h"
#include "ofxAutoControlPanel.h"
#include "Mapamok.h"
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
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);

	void setupControlPanel();
	void setupMesh(string fileName);
	void drawLabeledPoint(int label, ofVec2f position, ofColor color, bool crossHair = true, ofColor bg = ofColor::black, ofColor fg = ofColor::white);
	void updateRenderMode();
	void drawSelectionMode();
	void drawRenderMode();
	void render();

	void loadCalibration();
	void saveCalibration();
	void resetCalibration();

	Mapamok mapamok;
	ofxAssimpModelLoader model;
	ofEasyCam cam;
	ofVboMesh objectMesh;
	ofMesh imageMesh;
	ofLight light;
	ofxAutoControlPanel panel;

	vector<cv::Point3f> objectPoints;
	vector<cv::Point2f> imagePoints;
	vector<unsigned int> pointIndices;

	AutoShader shader;

private:
	bool isDragging = false;
	bool isArrowing = false;
	bool hasSelection = false;
	bool isHovering = false;
	unsigned int hoveredIndex;
	unsigned int selectedIndex;
	int selectedReferenceIndex;

	bool dataChanged = false;
};
