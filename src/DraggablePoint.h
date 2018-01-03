#pragma once

class DraggablePoint {
public:
	ofVec2f position, positionStart;
	bool selected, dragging, marked, autoMark;
	
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
	bool isHit(ofVec2f v, float clickRadiusSquared) {
		bool curHit = position.distanceSquared(v) < clickRadiusSquared;
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
	void draw(float clickRadiusSquared) {
		float r = 2;//sqrt(clickRadiusSquared);
		ofPushStyle();
		ofNoFill();
		ofSetLineWidth(2);
		if(selected) {
			ofSetColor(ofColor::yellow);
			ofCircle(position, r + 4);
			ofSetLineWidth(1);
			ofSetColor(255);
			ofLine(position.x, 0, position.x, ofGetHeight());
			ofLine(0, position.y, ofGetWidth(), position.y);
		}
		ofPopStyle();
		ofPushStyle();
		ofFill();
		if (selected) {
			ofSetColor(ofColor::yellow);
		} else if(marked) {
			ofSetColor(ofColor::green);
		} else {
			ofSetColor(ofColor::red);
		}
		ofCircle(position, r);
		ofPopStyle();
	}
};