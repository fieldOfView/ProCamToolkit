#pragma once

#include "ofMain.h"
#include "ofxAssimpModelLoader.h"

int findNearestVertex(const vector<ofVec3f>& vertices, const ofVec3f& base);
ofMesh mergeNearbyVertices(const ofMesh& mesh, float tolerance = 0);
void project(ofMesh& mesh, const ofCamera& camera, ofRectangle viewport);
vector<ofMesh> getMeshes(ofxAssimpModelLoader& model); 
ofMesh joinMeshes(vector<ofMesh>& meshes);
