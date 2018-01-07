#pragma once

class DraggablePoint {
public:
	ofVec2f position, positionStart;
	bool selected, dragging, marked, autoMark;

	enum Sizes { SIZE_CLICK_RADIUS_SQUARED, SIZE_DOT_RADIUS, SIZE_SELECTED_DOT_RADIUS, SIZE_SELECTED_CIRCLE_RADIUS, SIZE_SELECTED_CIRCLE_THICKNESS };
	vector<float> sizes = { 64, 4., 1., 10., 2. };
	enum Colors { COLOR_NORMAL, COLOR_MARKED, COLOR_SELECTED, COLOR_CROSSHAIR };
	vector<ofColor> colors = { ofColor::red, ofColor::pink, ofColor::yellow, ofColor::purple };

	DraggablePoint()
	:selected(false)
	,dragging(false)
	,marked(false)
	,autoMark(true) {
	}
	void reset(bool keepMark = false) {
		selected = false;
		dragging = false;
		if (!keepMark) {
			marked = false;
		}
	}
	bool isHit(ofVec2f v) {
		bool curHit = position.distanceSquared(v) < sizes[SIZE_CLICK_RADIUS_SQUARED];
		if(curHit && autoMark) {
			marked = true;
		}
		return curHit;
	}
	void mark(bool mark = true) {
		marked = mark;
	}
	void setAutoMark(bool flag) {
		autoMark = flag;
	}
	void setTheme(vector<float> _sizes, vector<ofColor> _colors) {
		sizes = _sizes;
		colors = _colors;
	}
	void draw() {
		ofPushStyle();
		ofNoFill();
		if(selected) {
			ofSetColor(colors[COLOR_SELECTED]);
			ofSetLineWidth(sizes[SIZE_SELECTED_CIRCLE_THICKNESS]);
			ofCircle(position, sizes[SIZE_SELECTED_CIRCLE_RADIUS]);
			ofSetLineWidth(1);
			ofSetColor(colors[COLOR_CROSSHAIR]);
			ofRectangle viewport = ofGetCurrentViewport();
			ofLine(position.x, 0, position.x, viewport.height);
			ofLine(0, position.y, viewport.width, position.y);
		}
		ofPopStyle();
		ofPushStyle();
		ofFill();
		float r = sizes[SIZE_DOT_RADIUS];
		if (selected) {
			r = sizes[SIZE_SELECTED_DOT_RADIUS];
			ofSetColor(colors[COLOR_SELECTED]);
		} else if(marked) {
			ofSetColor(colors[COLOR_MARKED]);
		} else {
			ofSetColor(colors[COLOR_NORMAL]);
		}
		ofCircle(position, r);
		ofPopStyle();
	}
};