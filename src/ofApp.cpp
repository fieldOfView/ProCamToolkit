#include "ofApp.h"
#include "MeshUtils.h"


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

	referenceMeshPoints.setAutoMark(false);
	placedPoints.setAutoMark(false);

	referenceMeshPoints.disableDrawEvent();
	referenceMeshPoints.enableControlEvents();
	placedPoints.disableDrawEvent();
	placedPoints.disableControlEvents();

	referenceMeshPoints.setClickRadius(8);
	placedPoints.setClickRadius(8);
}

void ofApp::update() {
	if(getb("randomLighting")) {
		setf("lightX", ofSignedNoise(ofGetElapsedTimef(), 1, 1) * 1000);
		setf("lightY", ofSignedNoise(1, ofGetElapsedTimef(), 1) * 1000);
		setf("lightZ", ofSignedNoise(1, 1, ofGetElapsedTimef()) * 1000);
	}
	light.setPosition(getf("lightX"), getf("lightY"), getf("lightZ"));

	if(getb("selectionMode")) {
		updateSelectionMode();
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

void ofApp::draw() {
	ofBackground(geti("backgroundColor"));

	string message = "";

	if (referenceMesh.getNumIndices() > 0) {
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
	if(key == OF_KEY_BACKSPACE) { // delete selected
		if (getb("selectionMode")) {
			vector<unsigned int> selectedPoints = referenceMeshPoints.getSelected();
			reverse(selectedPoints.begin(), selectedPoints.end());
			for (unsigned int const& selectedPoint : selectedPoints) {
				if (referenceMeshPoints.get(selectedPoint).marked) {
					referenceMeshPoints.get(selectedPoint).marked = false;
					auto result = find(pointIndices.begin(), pointIndices.end(), selectedPoint);
					if (result != pointIndices.end()) {
						unsigned int placedPointIndex = result - pointIndices.begin();
						placedPoints.remove(placedPointIndex);
						pointIndices.erase(pointIndices.begin() + placedPointIndex);
					}
				}
			}
		} else {
			vector<unsigned int> selectedPoints = placedPoints.getSelected();
			reverse(selectedPoints.begin(), selectedPoints.end());
			for (unsigned int const& selectedPoint : selectedPoints) {
				referenceMeshPoints.get((pointIndices[selectedPoint])).marked = false;
				placedPoints.remove(selectedPoint);
				pointIndices.erase(pointIndices.begin() + selectedPoint);
			}
		}
	}

	if(key == ' ') { // toggle render/select mode
		setb("selectionMode", !getb("selectionMode"));
		if (getb("selectionMode")) {
			referenceMeshPoints.enableControlEvents();
			placedPoints.disableControlEvents();

			referenceMeshPoints.deselectAll();

		} else {
			referenceMeshPoints.disableControlEvents();
			placedPoints.enableControlEvents();

			placedPoints.deselectAll();

			vector<unsigned int> selectedPoints = referenceMeshPoints.getSelected();
			for (unsigned int const& selectedPoint : selectedPoints) {
				if (!referenceMeshPoints.get(selectedPoint).marked) {
					// new point
					ofVec2f newPoint;
					if (mapamok.calibrationReady) {

					} else {
						newPoint = ofVec2f(ofGetMouseX(), ofGetMouseY());
					}
					objectPoints.push_back(toCv(objectMesh.getVertex(selectedPoint)));
					imagePoints.push_back(toCv(newPoint));
					placedPoints.add(newPoint);

					unsigned int newPlacedPointIndex = placedPoints.size() - 1;
					pointIndices.push_back(selectedPoint);

					referenceMeshPoints.get(selectedPoint).marked = true;

					placedPoints.setSelected(newPlacedPointIndex, true);
				} else {
					// point was already placed

					unsigned int previouslyPlacedPointIndex = -1;
					auto result = find(pointIndices.begin(), pointIndices.end(), selectedPoint);
					if (result != pointIndices.end()) {
						previouslyPlacedPointIndex = result - pointIndices.begin();

						placedPoints.setSelected(previouslyPlacedPointIndex, true);
					}
				}
			}
		}
	}
}

void ofApp::setupMesh(string fileName) {
	objectMesh = ofVboMesh();
	ofFile meshFile(fileName);
	if (meshFile.exists()) {
		model.loadModel(fileName);
		vector<ofMesh> meshes = getMeshes(model);

		// join all the meshes
		objectMesh = ofVboMesh();
		objectMesh = joinMeshes(meshes);
	}

	referenceMesh = ofVboMesh(objectMesh);
	referenceMesh = mergeNearbyVertices(referenceMesh, selectionMergeTolerance);

	referenceMeshPoints.clear();
	for (std::vector<int>::size_type index = 0; index != referenceMesh.getNumVertices(); index++) {
		referenceMeshPoints.add(ofVec2f());
	}

	objectPoints.clear();
	imagePoints.clear();
	pointIndices.clear();
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
	string dirName = "calibrations/" + ofGetTimestampString() + "/";
	ofDirectory dir(dirName);
	dir.create(true);

	mapamok.save(dirName + "calibration.yml", dirName + "summary.txt");

	FileStorage fs(ofToDataPath(dirName + "pointdata.yml"), FileStorage::WRITE);
	if (!fs.isOpened()) {
		ofLogError() << "could not open pointdata file for writing";
		return;
	}

	fs << "objectPoints" << objectPoints;
	fs << "imagePoints" << imagePoints;
	fs << "pointIndices" << vector<int>(pointIndices.begin(), pointIndices.end());
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

	FileStorage fs(ofToDataPath(calibPath + "/pointdata.yml", true), FileStorage::READ);
	if (!fs.isOpened()) {
		ofLogError() << "could not open pointdata file for reading";
		return;
	}

	vector<int> pointIndicesSigned;
	fs["objectPoints"] >> objectPoints;
	fs["imagePoints"] >> imagePoints;
	fs["pointIndices"] >> pointIndicesSigned;
	pointIndices = vector<unsigned int>(pointIndicesSigned.begin(), pointIndicesSigned.end());

	mapamok.load(calibPath + "/calibration.yml");
	dataChanged = false;
}

void ofApp::resetCalibration() {
	objectPoints.clear();
	imagePoints.clear();
	pointIndices.clear();

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
	panel.addSlider("backgroundColor", 0, 0, 255, true);
	panel.addSlider("screenPointSize", 2, 1, 16, true);
	panel.addSlider("selectedPointSize", 8, 1, 16, true);
	panel.addSlider("selectionRadius", 12, 1, 32);
	panel.addSlider("lightX", 200, -1000, 1000);
	panel.addSlider("lightY", 400, -1000, 1000);
	panel.addSlider("lightZ", 800, -1000, 1000);
	panel.addToggle("randomLighting", false);
}

void ofApp::updateSelectionMode() {
	if (getb("setupMode")) {
		// TODO: only do this if the camera has changed
		imageMesh = ofVboMesh(referenceMesh);
		project(imageMesh, cam, ofGetCurrentViewport());
		vector<ofPoint> imageMeshPoints = imageMesh.getVertices();
		for (std::vector<int>::size_type index = 0; index != imageMeshPoints.size(); index++) {
			referenceMeshPoints.get(index).position = ofVec2f(imageMeshPoints[index].x, imageMeshPoints[index].y);
		}
	}
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
		mapamok.calibrate(ofGetWidth(), ofGetHeight(), imagePoints, objectPoints, flags, aov);
		dataChanged = false;
	}
}


void ofApp::drawSelectionMode() {
	cam.begin();
	
	float scale = getf("scale");
	ofScale(scale, scale, scale);

	render();

	cam.end();

	if (getb("setupMode")) {
		referenceMeshPoints.draw(ofEventArgs());
	}
}

void ofApp::drawRenderMode() {
	if (mapamok.calibrationReady) {
		mapamok.begin();

		render();
		if (getb("setupMode")) {
			imageMesh = getProjectedMesh(referenceMesh);
		}

		mapamok.end();
	}


	if (getb("setupMode")) {
		placedPoints.draw(ofEventArgs());
	}
}
