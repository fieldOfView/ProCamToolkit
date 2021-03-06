#include "ofApp.h"

void ofApp::setb(string name, bool value) {
	panel.setValueB(name, value);
}
void ofApp::seti(string name, int value) {
	panel.setValueI(name, value);
}
void ofApp::setf(string name, float value) {
	panel.setValueF(name, value);
}
bool ofApp::getb(string name) {
	return panel.getValueB(name);
}
int ofApp::geti(string name) {
	return panel.getValueI(name);
}
float ofApp::getf(string name) {
	return panel.getValueF(name);
}

void ofApp::setup() {
	ofSetWindowTitle("mapamok");
	ofSetVerticalSync(true);
	ofBackground(0);

	loadModel("model.dae");
	shader.setup("shader");

	setupControlPanel();
}

void ofApp::update() {
	calibrator.enabled = getb("setupMode");

	if (getb("selectionMode") != calibrator.selectPoints) {
		calibrator.setState(getb("selectionMode"));
	}

	if (getb("loadCalibration")) {
		loadCalibration();
		setb("loadCalibration", false);
	}
	if (getb("saveCalibration")) {
		saveCalibration();
		setb("saveCalibration", false);
	}
	if (getb("resetCalibration")) {
		resetCalibration();
		setb("resetCalibration", false);
	}

	if (getb("setupMode") && !getb("selectionMode")) {
		// generate flags
#define getFlag(flag) (panel.getValueB((#flag)) ? flag : 0)
		int flags =
			CV_CALIB_USE_INTRINSIC_GUESS |
			getFlag(CV_CALIB_FIX_PRINCIPAL_POINT) |
			getFlag(CV_CALIB_FIX_ASPECT_RATIO) |
			getFlag(CV_CALIB_FIX_K1) |
			getFlag(CV_CALIB_FIX_K2) |
			getFlag(CV_CALIB_FIX_K3) |
			getFlag(CV_CALIB_ZERO_TANGENT_DIST);

		calibrator.calibrate(flags);
	}

	if (getb("randomLighting")) {
		setf("lightX", ofSignedNoise(ofGetElapsedTimef(), 1, 1) * 1000);
		setf("lightY", ofSignedNoise(1, ofGetElapsedTimef(), 1) * 1000);
		setf("lightZ", ofSignedNoise(1, 1, ofGetElapsedTimef()) * 1000);
	}
	light.setPosition(getf("lightX"), getf("lightY"), getf("lightZ"));

	calibrator.setViewport(makeViewport());
	calibrator.update();
}

void ofApp::draw() {
	string message = "";

	int viewports = geti("viewports");

	int i = 0;
	if (objectMesh.getNumIndices() > 0) {
		if (getb("setupMode")) {
			if (viewports > 1) {
				ofNoFill();
				ofSetColor(ofColor::magenta);
				ofDrawRectangle(calibrator.viewport);
				ofFill();
				ofSetColor(ofColor::white);
			}

			calibrator.draw();
		} else {
			if (calibrator.mapamok.calibrationReady) {
				calibrator.mapamok.begin();
				render();
				calibrator.mapamok.end();
			}
			else {
				if (message != "") message += "\n";
				message += "Calibration not complete.";
			}
		}
	} else {
		if (message != "") message += "\n";
		message += "No model loaded.";
	}

	if (!shader.isLoaded()) {
		if (message != "") message += "\n";
		message += "Shader failed to load.";
	}

	if(message != "") {
		ofPushStyle();
		ofSetColor(ofColor::magenta);
		ofSetLineWidth(8);
		ofLine(0, 0, ofGetWidth(), ofGetHeight());
		ofLine(ofGetWidth(), 0, 0, ofGetHeight());
		ofVec2f center(ofGetWidth(), ofGetHeight());
		center /= 2;
		center.x -= message.size() * 8 / 2;
		ofDrawBitmapStringHighlight(message, center);
		ofPopStyle();
	}
}

void ofApp::keyPressed(int key) {
	if(key == OF_KEY_BACKSPACE || key == OF_KEY_DEL) { // delete selected
		calibrator.removeSelected();
	}

	if(key == ' ') { // toggle render/select mode
		setb("selectionMode", !getb("selectionMode"));
	}
}

void ofApp::dragEvent(ofDragInfo dragInfo) {
	if (dragInfo.files.size() == 1) {
		string filename = dragInfo.files[0];
		loadModel(filename);
	}
}

void ofApp::loadModel(string fileName) {
	objectMesh = ofVboMesh();
	ofFile meshFile(fileName);
	if (meshFile.exists()) {
		ofxAssimpModelLoader model;
		model.loadModel(fileName);

		// join all the meshes
		objectMesh = ofVboMesh();
		for (int i = 0; i < model.getNumMeshes(); i++) {
			objectMesh.append(model.getMesh(i));
		}
	}
	
	calibrator.setup(objectMesh);
}

void ofApp::render() {
	ofPushStyle();
	ofSetLineWidth(geti("lineWidth"));
	if(getb("useSmoothing")) {
		ofEnableSmoothing();
	} else {
		ofDisableSmoothing();
	}
	int shading = geti("shading");
	bool useLights = shading == 1;
	bool useShader = shading == 2;
	if(useLights) {
		light.enable();
		ofEnableLighting();
		glShadeModel(GL_SMOOTH);
		glEnable(GL_NORMALIZE);
	}

	ofSetColor(255);
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_DEPTH_TEST);

	if(useShader) {
		shader.begin();
		shader.setUniform1f("elapsedTime", ofGetElapsedTimef());
		shader.end();
	}

	ofColor transparentBlack(0, 0, 0, 0);
	switch(geti("drawMode")) {
		case 0: // faces
			if(useShader) shader.begin();
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			objectMesh.drawFaces();
			if(useShader) shader.end();
			break;
		case 1: // fullWireframe
			if(useShader) shader.begin();
			objectMesh.drawWireframe();
			if(useShader) shader.end();
			break;
		case 2: // outlineWireframe
			LineArt::draw(objectMesh, true, transparentBlack, useShader ? &shader : NULL);
			break;
		case 3: // occludedWireframe
			LineArt::draw(objectMesh, false, transparentBlack, useShader ? &shader : NULL);
			break;
	}
	glPopAttrib();
	if(useLights) {
		ofDisableLighting();
	}
	ofPopStyle();
}

void ofApp::saveCalibration() {
	string dirName = "calibrations/" + ofGetTimestampString() + "/";
	ofDirectory dir(dirName);
	dir.create(true);

	calibrator.mapamok.save(dirName + "calibration.yml", dirName + "summary.txt");
	calibrator.save(dirName + "pointdata.yml");
}

void ofApp::loadCalibration() {
#ifdef TARGET_WIN32
	int windowMode = ofGetWindowMode();
	if (windowMode == OF_FULLSCREEN) ofSetFullscreen(false);
#endif

	string calibPath;
	ofFileDialogResult result = ofSystemLoadDialog("Select a calibration folder", true, ofFilePath::addTrailingSlash(ofToDataPath("calibrations", true)));

#ifdef TARGET_WIN32
	if (windowMode == OF_FULLSCREEN) ofSetFullscreen(true);
#endif

	if (!result.bSuccess) {
		ofLogNotice() << "canceled loading calibration";
		return;
	}
	calibPath = result.getPath();

	ofFile pointDataFile(calibPath + "/pointdata.yml");
	ofFile calibrationFile(calibPath + "/calibration.yml");
	if (!pointDataFile.exists() || !calibrationFile.exists()) {
		ofLogError() << "calibration files not found in folder";
		return;
	}

	calibrator.mapamok.load(calibPath + "/calibration.yml");
	calibrator.load(calibPath + "/pointdata.yml");
}

void ofApp::resetCalibration() {
	calibrator.mapamok.reset();
	calibrator.reset();
}

void ofApp::setupControlPanel() {
	panel.setup();
	panel.msg = "tab hides the panel, space toggles render/selection mode, 'f' toggles fullscreen.";

	panel.addPanel("Main");
	panel.addToggle("setupMode", true);
	panel.addToggle("selectionMode", true);

	panel.addSlider("viewports", 1, 1, 4, true);
	panel.addSlider("activeViewport", 1, 1, 4, true);

	panel.addToggle("loadCalibration", false);
	panel.addToggle("saveCalibration", false);
	panel.addToggle("resetCalibration", false);

	panel.addPanel("Rendering");
	panel.addMultiToggle("drawMode", 3, variadic("faces")("fullWireframe")("outlineWireframe")("occludedWireframe"));
	panel.addMultiToggle("shading", 0, variadic("none")("lights")("shader"));
	panel.addSlider("lineWidth", 1, 1, 8, true);
	panel.addToggle("useSmoothing", false);
	panel.addSlider("lightX", 200, -1000, 1000);
	panel.addSlider("lightY", 400, -1000, 1000);
	panel.addSlider("lightZ", 800, -1000, 1000);
	panel.addToggle("randomLighting", false);

	panel.addPanel("Calibration");
	panel.addToggle("CV_CALIB_FIX_ASPECT_RATIO", true);
	panel.addToggle("CV_CALIB_FIX_K1", true);
	panel.addToggle("CV_CALIB_FIX_K2", true);
	panel.addToggle("CV_CALIB_FIX_K3", true);
	panel.addToggle("CV_CALIB_ZERO_TANGENT_DIST", true);
	panel.addToggle("CV_CALIB_FIX_PRINCIPAL_POINT", false);
}

ofRectangle ofApp::makeViewport()
{
	ofRectangle viewport;

	int viewports = geti("viewports");
	int currentViewport = min(geti("activeViewport"), viewports) - 1;

	if (viewports < 4) {
		int viewportWidth = ofGetWidth() / viewports;
		int viewportHeight = ofGetHeight();
		return ofRectangle(currentViewport * viewportWidth, 0, viewportWidth, viewportHeight);
	} else {
		int rows = sqrt(viewports);
		int columns = ceil((double)viewports / rows);
		int viewportWidth = ofGetWidth() / columns;
		int viewportHeight = ofGetHeight() / rows;

		return ofRectangle((currentViewport % columns) * viewportWidth, (currentViewport / columns) * viewportHeight, viewportWidth, viewportHeight);
	}
}
