#pragma once

#include "EventWatcher.h"
#include "DraggablePoint.h"

class SelectablePoints : public EventWatcher {
protected:
	vector<DraggablePoint> points;
	set<unsigned int> selected;
	
	bool allowMultiSelect, autoMark;

	// { SIZE_CLICK_RADIUS_SQUARED, SIZE_DOT_RADIUS, SIZE_SELECTED_DOT_RADIUS, SIZE_SELECTED_CIRCLE_RADIUS, SIZE_SELECTED_CIRCLE_THICKNESS };
	vector<float> sizes = { 64, 4., 1., 10., 2. };
	// { COLOR_NORMAL, COLOR_MARKED, COLOR_SELECTED, COLOR_CROSSHAIR };
	vector<ofColor> colors = { ofColor::red, ofColor::pink, ofColor::yellow, ofColor::purple };

public:
	SelectablePoints()
	:allowMultiSelect(true)
	,pointsChanged(false) {
	}

	bool pointsChanged;

	unsigned int size() {
		return points.size();
	}
	void add(const ofVec2f& v) {
		points.push_back(DraggablePoint());
		points.back().setAutoMark(autoMark);
		points.back().position = v;
		points.back().setTheme(sizes, colors);
		pointsChanged = true;
	}
	void remove(unsigned int index) {
		points.erase(points.begin() + index);
		selected.clear();
		pointsChanged = true;
	}
	DraggablePoint& get(int i) {
		return points[i];
	}
	void setSelected(unsigned int index, bool select) {
		if (points[index].selected && !select) {
			selected.erase(index);
		} else if (!points[index].selected && select) {
			selected.insert(index);
		}
		points[index].selected = select;
	}

	vector<unsigned int> getSelected() {
		vector<unsigned int> result;
		std::copy(selected.begin(), selected.end(), std::back_inserter(result));
		return result;
	}
	vector<unsigned int> getMarked() {
		vector<unsigned int> result;
		for (auto itr = points.begin(); itr != points.end(); itr++) {
			if ((*itr).marked) {
				result.push_back(itr - points.begin());
			}
		}
		return result;
	}
	void clear() {
		points.clear();
		selected.clear();
		pointsChanged = true;
	}
	void deselectAll(bool keepMark = true) {
		for (auto itr = points.begin(); itr != points.end(); itr++) {
			(*itr).selected = false;
			if (!keepMark) {
				(*itr).marked = false;
			}
		}
		selected.clear();
	}
	void setAllowMultiSelect(bool allow) {
		this->allowMultiSelect = allow;
	}
	void setAutoMark(bool flag) {
		this->autoMark = flag;
		for (set<unsigned int>::iterator itr = selected.begin(); itr != selected.end(); itr++) {
			points[*itr].autoMark = flag;
		}
	}

	void setTheme(vector<float> _sizes, vector<ofColor> _colors) {
		sizes = _sizes;
		colors = _colors;
		for (auto itr = points.begin(); itr != points.end(); itr++) {
			(*itr).setTheme(sizes, colors);
		}
	}

	void mousePressed(ofMouseEventArgs& mouse) {
		bool shift = ofGetKeyPressed(OF_KEY_SHIFT) && allowMultiSelect;
		int nearestPointIndex = -1;
		float nearestPointDistanceSquared;
		for(int i = 0; i < size(); i++) {
			bool hit = points[i].isHit(mouse);
			if(hit) {
				float distanceSquared = points[i].position.distanceSquared(mouse);
				if (distanceSquared < 1.0) {
					nearestPointIndex = i;
					break;
				}
				if (nearestPointIndex == -1 || distanceSquared < nearestPointDistanceSquared) {
					nearestPointDistanceSquared = distanceSquared;
					nearestPointIndex = i;
				}
			}			
		}
		if (!shift) {
			deselectAll();
		}
		if (nearestPointIndex != -1 && !points[nearestPointIndex].selected) {
			points[nearestPointIndex].selected = true;
			selected.insert(nearestPointIndex);
		}
	}
	virtual void keyPressed(ofKeyEventArgs& key) {
		if(key.key == OF_KEY_DEL || key.key == OF_KEY_BACKSPACE) {
			for(set<unsigned int>::iterator itr = selected.begin(); itr != selected.end(); itr++) {
				points[*itr].reset();
			}
			selected.clear();
		}
	}
	void draw(ofEventArgs& args) {
		ofPushStyle();
		for(int i = 0; i < size(); i++) {
			points[i].draw();
		}
		ofPopStyle();
	}
};