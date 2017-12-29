#pragma once
#include "ofMain.h"

class AutoShader : public ofShader {
public:
	void setup(string name) {
		this->name = name;
		ofEventArgs args;
		update(args);
		ofAddListener(ofEvents().update, this, &AutoShader::update);
	}
	
	void update(ofEventArgs &args) {	
		bool needsReload = false;
			
		string fragName = name + ".frag";
		ofFile fragFile(fragName);
		if(fragFile.exists()) {
			uint64_t fragTimestamp = filesystem::last_write_time(fragFile);
			if(fragTimestamp != lastFragTimestamp) {
				needsReload = true;
				lastFragTimestamp = fragTimestamp;
			}
		} else {
			fragName = "";
		}
		
		string vertName = name + ".vert";
		ofFile vertFile(vertName);
		if(vertFile.exists()) {
			uint64_t vertTimestamp = filesystem::last_write_time(vertFile);
			if(vertTimestamp != lastVertTimestamp) {
				needsReload = true;
				lastVertTimestamp = vertTimestamp;
			}
		} else {
			vertName = "";
		}
		
		if(needsReload) {
			ofLogVerbose("AutoShader") << "reloading shader at " << ofGetTimestampString("%H:%M:%S");
			ofShader::load(vertName, fragName);
		}
	}

private:
	string name;
	uint64_t lastFragTimestamp, lastVertTimestamp;
};
