#include "ReferencePoints.h"
#include "MeshUtils.h"


using namespace ofxCv;
using namespace cv;

ReferencePoints::ReferencePoints() {
	referenceMeshPoints.setAutoMark(false);
	referenceMeshPoints.setAllowMultiSelect(false);
	placedPoints.setAutoMark(false);
	placedPoints.setAllowMultiSelect(false);

	referenceMeshPoints.disableDrawEvent();
	referenceMeshPoints.enableControlEvents();
	placedPoints.disableDrawEvent();
	placedPoints.disableControlEvents();

	referenceMeshPoints.setClickRadius(8);
	placedPoints.setClickRadius(8);

	enabled = true;
	selectPoints = true;
}

void ReferencePoints::setup(ofMesh mesh) {
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

void ReferencePoints::update() {
	if (!enabled) {
		return;
	}

	if (selectPoints) {
		updateSelectMode();
	}
	else {
		updatePlaceMode();
	}
}

void ReferencePoints::updateSelectMode() {
	// TODO: only do this if the camera has changed
	ofMesh imageMesh = ofVboMesh(referenceMesh);
	project(imageMesh, camera, ofGetCurrentViewport());
	vector<ofPoint> imageMeshPoints = imageMesh.getVertices();
	for (std::vector<int>::size_type index = 0; index != imageMeshPoints.size(); index++) {
		referenceMeshPoints.get(index).position = ofVec2f(imageMeshPoints[index].x, imageMeshPoints[index].y);
	}
}

void ReferencePoints::updatePlaceMode() {

}

void ReferencePoints::draw() {
	if (!enabled) {
		return;
	}

	if (selectPoints) {
		drawSelectMode();
	} else {
		drawPlaceMode();
	}
}

void ReferencePoints::drawSelectMode() {
	referenceMeshPoints.draw(ofEventArgs());
}

void ReferencePoints::drawPlaceMode() {
	placedPoints.draw(ofEventArgs());
}

void ReferencePoints::setState(bool select) {
	selectPoints = select;

	if (selectPoints) {
		referenceMeshPoints.enableControlEvents();
		placedPoints.disableControlEvents();

		referenceMeshPoints.deselectAll();

		vector<unsigned int> selectedPoints = placedPoints.getSelected();
		for (unsigned int const& selectedPoint : selectedPoints) {
			referenceMeshPoints.setSelected(pointIndices[selectedPoint], true);
		}
	}
	else {

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
					imagePoint = mapamok.worldToScreen(imagePoint);
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

void ReferencePoints::removeSelected() {
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

void ReferencePoints::calibrate(int flags) {
	if (placedPoints.pointsChanged) {
		placedPoints.pointsChanged = false;
		vector<Point2f> imagePoints;
		for (std::vector<int>::size_type i = 0; i != placedPoints.size(); i++) {
			imagePoints.push_back(toCv(placedPoints.get(i).position));
		}

		mapamok.calibrate(ofGetWidth(), ofGetHeight(), imagePoints, objectPoints, flags, 80);
	}
}

void ReferencePoints::load(string fileName) {
	FileStorage fs(ofToDataPath(fileName, true), FileStorage::READ);
	if (!fs.isOpened()) {
		ofLogError() << "could not open pointdata file for reading";
		return;
	}

	vector<int> pointIndicesSigned;
	vector<Point2f> imagePoints;

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

void ReferencePoints::save(string fileName) {
	FileStorage fs(ofToDataPath(fileName), FileStorage::WRITE);
	if (!fs.isOpened()) {
		ofLogError() << "could not open pointdata file for writing";
		return;
	}

	vector<Point2f> imagePoints;
	for (std::vector<int>::size_type i = 0; i != placedPoints.size(); i++) {
		imagePoints.push_back(toCv(placedPoints.get(i).position));
	}

	fs << "objectPoints" << objectPoints;
	fs << "imagePoints" << imagePoints;
	fs << "pointIndices" << vector<int>(pointIndices.begin(), pointIndices.end());
}

void ReferencePoints::reset() {
	referenceMeshPoints.deselectAll(false);
	placedPoints.clear();
	objectPoints.clear();
	pointIndices.clear();
	dataChanged = true;
}
