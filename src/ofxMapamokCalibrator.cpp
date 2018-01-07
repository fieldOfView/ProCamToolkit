#include "ofxMapamokCalibrator.h"


ofxMapamokCalibrator::ofxMapamokCalibrator() {
	referenceMeshPoints.setAutoMark(false);
	referenceMeshPoints.setAllowMultiSelect(false);
	placedPoints.setAutoMark(false);
	placedPoints.setAllowMultiSelect(false);

	referenceMeshPoints.disableDrawEvent();
	referenceMeshPoints.enableControlEvents();
	placedPoints.disableDrawEvent();
	placedPoints.disableControlEvents();

	// { SIZE_CLICK_RADIUS_SQUARED, SIZE_DOT_RADIUS, SIZE_SELECTED_DOT_RADIUS, SIZE_SELECTED_CIRCLE_RADIUS, SIZE_SELECTED_CIRCLE_THICKNESS };
	vector<float> sizes = { 64, 4., 1., 10., 2. };
	// { COLOR_NORMAL, COLOR_MARKED, COLOR_SELECTED, COLOR_CROSSHAIR };
	vector<ofColor> colors = { ofColor(0, 171, 236), ofColor::yellow, ofColor::yellow, ofColor::yellow };
	referenceMeshPoints.setTheme(sizes, colors);
	colors[0] = ofColor::yellow;
	placedPoints.setTheme(sizes, colors);

	enabled = true;
	selectPoints = true;
}

void ofxMapamokCalibrator::setup(ofMesh mesh) {
	displayMesh = mesh;

	referenceMesh = ofVboMesh(mesh);
	referenceMesh = mergeNearbyVertices(referenceMesh, selectionMergeTolerance);

	referenceMeshPoints.clear();
	for (std::vector<int>::size_type index = 0; index != referenceMesh.getNumVertices(); index++) {
		referenceMeshPoints.add(ofVec2f());
	}

	placedPoints.clear();
	objectPoints.clear();
	pointIndices.clear();
	dataChanged = true;
}

void ofxMapamokCalibrator::update() {
	if (!enabled) {
		return;
	}

	if (selectPoints) {
		ofMatrix4x4 modelViewProjectionMatrix = camera.getModelViewProjectionMatrix();
		if (viewportChanged ||
			!modelViewProjectionMatrix.getRowAsVec4f(0).match(lastModelViewProjectionMatrix.getRowAsVec4f(0)) ||
			!modelViewProjectionMatrix.getRowAsVec4f(1).match(lastModelViewProjectionMatrix.getRowAsVec4f(1)) ||
			!modelViewProjectionMatrix.getRowAsVec4f(2).match(lastModelViewProjectionMatrix.getRowAsVec4f(2)) ||
			!modelViewProjectionMatrix.getRowAsVec4f(3).match(lastModelViewProjectionMatrix.getRowAsVec4f(3))) {

			lastModelViewProjectionMatrix = modelViewProjectionMatrix;
			viewportChanged = false;

			ofMesh imageMesh = ofVboMesh(referenceMesh);
			ofRectangle windowRect(0, 0, ofGetWidth(), ofGetHeight());
			ofPoint offset = viewport.getCenter() - windowRect.getCenter();
			project(imageMesh, camera, windowRect);
			vector<ofPoint> imageMeshPoints = imageMesh.getVertices();
			for (std::vector<int>::size_type index = 0; index != imageMeshPoints.size(); index++) {
				referenceMeshPoints.get(index).position = ofVec2f(imageMeshPoints[index].x, imageMeshPoints[index].y) + offset;
			}
		}
	}
}

void ofxMapamokCalibrator::draw() {
	if (!enabled) {
		return;
	}

	ofColor transparentBlack(0, 0, 0, 0);

	if (selectPoints) {
		ofRectangle restoreViewport = ofGetCurrentViewport();
		ofViewport(viewport);
		camera.begin();

		drawHiddenLine(displayMesh);
		
		camera.end();
		ofViewport(restoreViewport);

		referenceMeshPoints.draw(ofEventArgs());
	} else {
		if (mapamok.calibrationReady)
		{
			mapamok.begin();
			drawHiddenLine(displayMesh);
			mapamok.end();
		}

		placedPoints.draw(ofEventArgs());
	}
}

void ofxMapamokCalibrator::drawHiddenLine(ofMesh mesh) {
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	ofSetDepthTest(true);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_POLYGON_OFFSET_FILL);
	float lineWidth = ofGetStyle().lineWidth;
	glPolygonOffset(+lineWidth, +lineWidth);

	glColorMask(false, false, false, false);
	mesh.drawFaces();

	glColorMask(true, true, true, true);
	glDisable(GL_POLYGON_OFFSET_FILL);
	mesh.drawWireframe();

	glPopAttrib();
}

void ofxMapamokCalibrator::setState(bool select) {
	selectPoints = select;

	if (selectPoints) {
		camera.enableMouseInput();

		referenceMeshPoints.enableControlEvents();
		placedPoints.disableControlEvents();

		referenceMeshPoints.deselectAll();

		vector<unsigned int> selectedPoints = placedPoints.getSelected();
		for (unsigned int const& selectedPoint : selectedPoints) {
			referenceMeshPoints.setSelected(pointIndices[selectedPoint], true);
		}
	}
	else {
		camera.disableMouseInput();

		referenceMeshPoints.disableControlEvents();
		placedPoints.enableControlEvents();

		placedPoints.deselectAll();

		vector<unsigned int> selectedPoints = referenceMeshPoints.getSelected();
		for (unsigned int const& selectedPoint : selectedPoints) {
			if (!referenceMeshPoints.get(selectedPoint).marked) {
				// new point
				ofVec2f newPoint;
				if (mapamok.calibrationReady) {
					ofVec3f imagePoint = referenceMesh.getVertex(selectedPoint);
					imagePoint = mapamok.worldToScreen(imagePoint, viewport);
					newPoint = ofVec2f(imagePoint);
				}
				else {
					newPoint = ofVec2f(ofGetMouseX(), ofGetMouseY());
				}
				objectPoints.push_back(toCv(referenceMesh.getVertex(selectedPoint)));
				placedPoints.add(newPoint);

				unsigned int newPlacedPointIndex = placedPoints.size() - 1;
				pointIndices.push_back(selectedPoint);

				referenceMeshPoints.get(selectedPoint).marked = true;

				placedPoints.setSelected(newPlacedPointIndex, true);
			}
			else {
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

void ofxMapamokCalibrator::removeSelected() {
	if (selectPoints) {
		// unmark currently selected reference mesh point and remove corresponding placed point
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
					objectPoints.erase(objectPoints.begin() + placedPointIndex);
				}
			}
		}
	}
	else {
		// remove currently selected placed point and unmark corresponding reference mesh point
		vector<unsigned int> selectedPoints = placedPoints.getSelected();
		reverse(selectedPoints.begin(), selectedPoints.end());
		for (unsigned int const& selectedPoint : selectedPoints) {
			referenceMeshPoints.get((pointIndices[selectedPoint])).marked = false;
			placedPoints.remove(selectedPoint);
			pointIndices.erase(pointIndices.begin() + selectedPoint);
			objectPoints.erase(objectPoints.begin() + selectedPoint);
		}
	}
}

void ofxMapamokCalibrator::calibrate(int flags) {
	if (placedPoints.pointsChanged || flags != lastFlags) {
		lastFlags = flags;

		placedPoints.pointsChanged = false;
		vector<cv::Point2f> imagePoints;
		for (std::vector<int>::size_type i = 0; i != placedPoints.size(); i++) {
			imagePoints.push_back(toCv(placedPoints.get(i).position));
		}

		mapamok.calibrate(viewport, imagePoints, objectPoints, flags, 80);
	}
}

void ofxMapamokCalibrator::setViewport(ofRectangle vp) {
	if (vp != viewport) {
		for (std::vector<int>::size_type i = 0; i != placedPoints.size(); i++) {
			// scale/translate all placed points from old viewport (viewport) to new viewport (vp)
			placedPoints.get(i).position = ((placedPoints.get(i).position - viewport.getTopLeft()) / (viewport.getBottomRight() - viewport.getTopLeft())) * (vp.getBottomRight() - vp.getTopLeft()) + vp.getTopLeft();
		}

		viewport = vp;
		mapamok.setViewport(viewport);
		viewportChanged = true;
		placedPoints.pointsChanged = true;
	}
}

void ofxMapamokCalibrator::load(string fileName) {
	cv::FileStorage fs(ofToDataPath(fileName, true), cv::FileStorage::READ);
	if (!fs.isOpened()) {
		ofLogError() << "could not open pointdata file for reading";
		return;
	}

	vector<int> pointIndicesSigned;
	vector<cv::Point2f> imagePoints;

	fs["objectPoints"] >> objectPoints;
	fs["imagePoints"] >> imagePoints;
	fs["pointIndices"] >> pointIndicesSigned;
	pointIndices = vector<unsigned int>(pointIndicesSigned.begin(), pointIndicesSigned.end());

	placedPoints.clear();
	for (std::vector<int>::size_type i = 0; i != imagePoints.size(); i++) {
		placedPoints.add(toOf(imagePoints[i]));
	}
	referenceMeshPoints.deselectAll(false);
	for (auto const& index : pointIndices) {
		referenceMeshPoints.get(index).marked = true;
	}

	dataChanged = false;
}

void ofxMapamokCalibrator::save(string fileName) {
	cv::FileStorage fs(ofToDataPath(fileName), cv::FileStorage::WRITE);
	if (!fs.isOpened()) {
		ofLogError() << "could not open pointdata file for writing";
		return;
	}

	vector<cv::Point2f> imagePoints;
	for (std::vector<int>::size_type i = 0; i != placedPoints.size(); i++) {
		imagePoints.push_back(toCv(placedPoints.get(i).position));
	}

	fs << "objectPoints" << objectPoints;
	fs << "imagePoints" << imagePoints;
	fs << "pointIndices" << vector<int>(pointIndices.begin(), pointIndices.end());
}

void ofxMapamokCalibrator::reset() {
	referenceMeshPoints.deselectAll(false);
	placedPoints.clear();
	objectPoints.clear();
	pointIndices.clear();
	dataChanged = true;
}

cv::Point2f ofxMapamokCalibrator::toCv(ofVec2f vec) {
	return cv::Point2f(vec.x, vec.y);
}

cv::Point3f ofxMapamokCalibrator::toCv(ofVec3f vec) {
	return cv::Point3f(vec.x, vec.y, vec.z);
}

ofVec2f ofxMapamokCalibrator::toOf(cv::Point2f point) {
	return ofVec2f(point.x, point.y);
}

ofVec3f ofxMapamokCalibrator::toOf(cv::Point3f point) {
	return ofVec3f(point.x, point.y, point.z);
}

