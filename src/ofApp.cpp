#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

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
	ofSetDrawBitmapMode(OF_BITMAPMODE_MODEL_BILLBOARD);
	ofSetVerticalSync(true);

	setupMesh("model.dae");
	shader.setup("shader");

	setupControlPanel();
}

void ofApp::update() {
	if(getb("randomLighting")) {
		setf("lightX", ofSignedNoise(ofGetElapsedTimef(), 1, 1) * 1000);
		setf("lightY", ofSignedNoise(1, ofGetElapsedTimef(), 1) * 1000);
		setf("lightZ", ofSignedNoise(1, 1, ofGetElapsedTimef()) * 1000);
	}
	light.setPosition(getf("lightX"), getf("lightY"), getf("lightZ"));

	if(getb("selectionMode")) {
		cam.enableMouseInput();
	} else {
		updateRenderMode();
		cam.disableMouseInput();
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
}

void enableFog(float nearFog, float farFog) {
	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_LINEAR);
	GLfloat fogColor[4]= {0, 0, 0, 1};
	glFogfv(GL_FOG_COLOR, fogColor);
	glHint(GL_FOG_HINT, GL_FASTEST);
	glFogf(GL_FOG_START, nearFog);
	glFogf(GL_FOG_END, farFog);
}

void disableFog() {
	glDisable(GL_FOG);
}

void ofApp::draw() {
	ofBackground(geti("backgroundColor"));

	string message = "";

	if (objectMesh.getNumIndices() > 0) {
		if (getb("selectionMode")) {
			drawSelectionMode();
		}
		else {
			drawRenderMode();
		}
	}
	else {
		if (message != "") message += "\n";
		message += "No model loaded.";
	}
	if (!shader.isLoaded()) {
		if (message != "") message += "\n";
		message += "Shader failed to load.";
	}

	if(message != "") {
		ofPushStyle();
		ofSetColor(magentaPrint);
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
	if(key == OF_KEY_LEFT || key == OF_KEY_UP || key == OF_KEY_RIGHT|| key == OF_KEY_DOWN){
		int index = selectedIndex;
		if(index > 0){
			isArrowing = true;
			Point2f& cur = imagePoints[index];
			switch(key) {
				case OF_KEY_LEFT: cur.x -= 1; break;
				case OF_KEY_RIGHT: cur.x += 1; break;
				case OF_KEY_UP: cur.y -= 1; break;
				case OF_KEY_DOWN: cur.y += 1; break;
			}
			dataChanged = true;
		}
	} else {
		isArrowing = false;
	}
	if(key == OF_KEY_BACKSPACE) { // delete selected
		if(hasSelection) {
			hasSelection = false;
			int index = selectedIndex;
			referencePoints[index] = false;
			imagePoints[index] = Point2f();
			dataChanged = true;
		}
	}
	if(key == '\n') { // deselect
		hasSelection = false;
	}
	if(key == ' ') { // toggle render/select mode
		setb("selectionMode", !getb("selectionMode"));
	}
}

void ofApp::mousePressed(int x, int y, int button) {
	hasSelection = isHovering;
	selectedIndex = hoveredIndex;
	if(hasSelection) {
		isDragging = true;
	}
}

void ofApp::mouseReleased(int x, int y, int button) {
	isDragging = false;
}

void ofApp::setupMesh(string fileName) {
	objectMesh = ofVboMesh();
	ofFile meshFile(fileName);
	if (meshFile.exists()) {
		model.loadModel(fileName);
		objectMesh = model.getMesh(0);
	}
	int n = objectMesh.getNumVertices();
	objectPoints.resize(n);
	imagePoints.resize(n);
	referencePoints.resize(n, false);
	for(int i = 0; i < n; i++) {
		objectPoints[i] = toCv(objectMesh.getVertex(i));
	}
	dataChanged = true;
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
	string dirName = "calibration-" + ofGetTimestampString() + "/";
	ofDirectory dir(dirName);
	dir.create();

	mapamok.save(dirName + "calibration.yml", dirName + "summary.txt");

	FileStorage fs(ofToDataPath(dirName + "pointdata.yml"), FileStorage::WRITE);
	if (!fs.isOpened()) {
		ofLogError() << "could not open calibration file for writing";
		return;
	}
	vector<unsigned char> referencePointsChar;
	for (auto const& refPoint : referencePoints) {
		referencePointsChar.push_back(refPoint);
	}
	fs << "objectPoints" << objectPoints;
	fs << "imagePoints" << imagePoints;
	fs << "referencePoints" << referencePointsChar;
}

void ofApp::loadCalibration() {

	string calibPath;
	ofFileDialogResult result = ofSystemLoadDialog("Select a calibration folder", true, ofToDataPath("", true));
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

	FileStorage fs(ofToDataPath(calibPath + "/pointdata.yml", true), FileStorage::READ);
	vector<unsigned char> referencePointsChar;

	fs["objectPoints"] >> objectPoints;
	fs["imagePoints"] >> imagePoints;
	fs["referencePoints"] >> referencePointsChar;

	referencePoints.clear();
	for (auto const& refPoint : referencePointsChar) {
		referencePoints.push_back(refPoint);
	}

	mapamok.load(calibPath + "/calibration.yml");
	dataChanged = false;
}

void ofApp::resetCalibration() {
	int n = referencePoints.size();
	for (int i = 0; i < n; i++) {
		referencePoints[i] = false;
		imagePoints[i] = Point2f();
	}
	mapamok.reset();
}

void ofApp::setupControlPanel() {
	panel.setup();
	panel.msg = "tab hides the panel, space toggles render/selection mode, 'f' toggles fullscreen.";

	panel.addPanel("Interaction");
	panel.addToggle("setupMode", true);
	panel.addToggle("selectionMode", true);
	panel.addMultiToggle("drawMode", 3, variadic("faces")("fullWireframe")("outlineWireframe")("occludedWireframe"));
	panel.addMultiToggle("shading", 0, variadic("none")("lights")("shader"));
	panel.addToggle("loadCalibration", false);
	panel.addToggle("saveCalibration", false);
	panel.addToggle("resetCalibration", false);

	panel.addPanel("Calibration");
	panel.addSlider("scale", 1, .1, 25);
	panel.addSlider("aov", 80, 50, 100);
	panel.addToggle("CV_CALIB_FIX_ASPECT_RATIO", true);
	panel.addToggle("CV_CALIB_FIX_K1", true);
	panel.addToggle("CV_CALIB_FIX_K2", true);
	panel.addToggle("CV_CALIB_FIX_K3", true);
	panel.addToggle("CV_CALIB_ZERO_TANGENT_DIST", true);
	panel.addToggle("CV_CALIB_FIX_PRINCIPAL_POINT", false);

	panel.addPanel("Rendering");
	panel.addSlider("lineWidth", 1, 1, 8, true);
	panel.addToggle("useSmoothing", false);
	panel.addToggle("useFog", false);
	panel.addSlider("fogNear", 200, 0, 1000);
	panel.addSlider("fogFar", 1850, 0, 2500);
	panel.addSlider("backgroundColor", 0, 0, 255, true);
	panel.addSlider("screenPointSize", 2, 1, 16, true);
	panel.addSlider("selectedPointSize", 8, 1, 16, true);
	panel.addSlider("selectionRadius", 12, 1, 32);
	panel.addSlider("lightX", 200, -1000, 1000);
	panel.addSlider("lightY", 400, -1000, 1000);
	panel.addSlider("lightZ", 800, -1000, 1000);
	panel.addToggle("randomLighting", false);

	panel.addPanel("Internal");
	panel.addSlider("slowLerpRate", .001, 0, .01);
	panel.addSlider("fastLerpRate", 1, 0, 1);
}

void ofApp::updateRenderMode() {
	float aov = getf("aov");

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

	if (dataChanged) {
		mapamok.calibrate(ofGetWidth(), ofGetHeight(), imagePoints, objectPoints, referencePoints, flags, aov);
		dataChanged = false;
	}
}

void ofApp::drawLabeledPoint(int label, ofVec2f position, ofColor color, bool crossHair, ofColor bg, ofColor fg) {
	ofPushStyle();
	glPushAttrib(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	//glEnable(GL_DEPTH_TEST);
	ofVec2f tooltipOffset(5, -25);
	ofSetColor(color);
	ofSetLineWidth(1);
	if (crossHair) {
		float w = ofGetWidth();
		float h = ofGetHeight();
		ofLine(position - ofVec2f(w, 0), position + ofVec2f(w, 0));
		ofLine(position - ofVec2f(0, h), position + ofVec2f(0, h));
	}
	ofNoFill();
	ofCircle(position, geti("selectedPointSize"));
	ofDrawBitmapStringHighlight(ofToString(label), position + tooltipOffset, bg, fg);
	glPopAttrib();
	ofPopStyle();
}

void ofApp::drawSelectionMode() {
	ofSetColor(255);
	cam.begin();
	float scale = getf("scale");
	ofScale(scale, scale, scale);
	if(getb("useFog")) {
		enableFog(getf("fogNear"), getf("fogFar"));
	}
	render();
	if(getb("useFog")) {
		disableFog();
	}
	if(getb("setupMode")) {
		imageMesh = getProjectedMesh(objectMesh);
	}
	cam.end();

	if(getb("setupMode")) {
		// draw all points cyan small
		glPointSize(geti("screenPointSize"));
		glEnable(GL_POINT_SMOOTH);
		ofSetColor(cyanPrint);
		imageMesh.drawVertices();

		// draw all reference points cyan
		int n = referencePoints.size();
		for(int i = 0; i < n; i++) {
			if(referencePoints[i]) {
				drawLabeledPoint(i, imageMesh.getVertex(i), cyanPrint, false);
			}
		}

		// check to see if anything is selected
		// draw hover point magenta
		int index;
		float distance;
		ofVec3f selected = getClosestPointOnMesh(imageMesh, mouseX, mouseY, &index, &distance);
		if(!ofGetMousePressed() && distance < getf("selectionRadius")) {
			hoveredIndex = index;
			isHovering = true;
			drawLabeledPoint(index, selected, magentaPrint);
		} else {
			isHovering = false;
		}

		// draw selected point yellow
		if(hasSelection) {
			int index = selectedIndex;
			ofVec2f selected = imageMesh.getVertex(index);
			drawLabeledPoint(index, selected, yellowPrint, true, ofColor::white, ofColor::black);
		}
	}
}

void ofApp::drawRenderMode() {
	mapamok.begin();

	if(mapamok.calibrationReady) {
		render();
		if(getb("setupMode")) {
			imageMesh = getProjectedMesh(objectMesh);
		}
	}

	mapamok.end();

	if(getb("setupMode")) {
		// draw all reference points cyan
		int n = referencePoints.size();
		for(int i = 0; i < n; i++) {
			if(referencePoints[i]) {
				drawLabeledPoint(i, toOf(imagePoints[i]), cyanPrint, false);
			}
		}

		// move points that need to be dragged
		// draw selected yellow
		int index = selectedIndex;
		if(hasSelection) {
			referencePoints[index] = true;
			Point2f& cur = imagePoints[index];
			if(cur == Point2f()) {
				if(mapamok.calibrationReady) {
					cur = toCv(ofVec2f(imageMesh.getVertex(index)));
				} else {
					cur = Point2f(mouseX, mouseY);
				}
			}
			dataChanged = true;
		}
		if(hasSelection) {
			Point2f& cur = imagePoints[index];
			if (isDragging) {
				float rate = ofGetMousePressed(0) ? getf("slowLerpRate") : getf("fastLerpRate");
				cur = Point2f(ofLerp(cur.x, mouseX, rate), ofLerp(cur.y, mouseY, rate));
			}
			drawLabeledPoint(index, toOf(cur), yellowPrint, true, ofColor::white, ofColor::black);
			ofSetColor(ofColor::white);
			ofRect(toOf(cur), 1, 1);
		} else {
			// check to see if anything is selected
			// draw hover magenta
			float distance;
			ofVec2f selected = toOf(getClosestPoint(imagePoints, mouseX, mouseY, &index, &distance));
			if(!ofGetMousePressed() && referencePoints[index] && distance < getf("selectionRadius")) {
				hoveredIndex = index;
				isHovering = true;
				drawLabeledPoint(index, selected, magentaPrint);
			} else {
				isHovering = false;
			}
		}
	}
}
