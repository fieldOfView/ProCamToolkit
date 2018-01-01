#pragma once

#include "EventWatcher.h"
#include "DraggablePoint.h"

class SelectablePoints : public EventWatcher {
protected:
	vector<DraggablePoint> points;
	set<unsigned int> selected;
	
	float clickRadiusSquared;
	bool allowMultiSelect, autoMark;

public:
	SelectablePoints()
	:clickRadiusSquared(0) 
	,allowMultiSelect(true) {
	}
	unsigned int size() {
		return points.size();
	}
	void add(const ofVec2f& v) {
		points.push_back(DraggablePoint());
		points.back().setAutoMark(autoMark);
		points.back().position = v;
	}
	void remove(unsigned int index) {
		points.erase(points.begin() + index);
		selected.clear();
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
    }
	void deselectAll() {
		for (auto itr = points.begin(); itr != points.end(); itr++) {
			(*itr).selected = false;
		}
		selected.clear();
	}
	void setClickRadius(float clickRadius) {
		this->clickRadiusSquared = clickRadius * clickRadius;
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
	void mousePressed(ofMouseEventArgs& mouse) {
		bool shift = ofGetKeyPressed(OF_KEY_SHIFT) && allowMultiSelect;
		int nearestPointIndex = -1;
		float nearestPointDistanceSquared;
		for(int i = 0; i < size(); i++) {
			bool hit = points[i].isHit(mouse, clickRadiusSquared);
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
			points[i].draw(clickRadiusSquared);
		}
        ofPopStyle();
	}
};