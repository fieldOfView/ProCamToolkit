#pragma once


int findNearestVertex(const vector<ofVec3f>& vertices, const ofVec3f& base) {
    int nearestIndex = 0;
    float nearestDistance = 0;
    int n = vertices.size();
    for(int i = 0; i < n; i++) {
        float distance = base.squareDistance(vertices[i]);
        if(i == 0 || distance < nearestDistance) {
            nearestDistance = distance;
            nearestIndex = i;
        }
    }
    return nearestIndex;
}


// assumes mesh is indexed
// drops all normals, colors, and tex coords
ofMesh mergeNearbyVertices(const ofMesh& mesh, float tolerance = 0) {
    if(tolerance == 0) {
        return mesh;
    }
    float squareTolerance = tolerance * tolerance;
    ofMesh mergedMesh;
    int n = mesh.getNumVertices();
    vector<int> remappedIndices;
    for(int i = 0; i < n; i++) {
        const ofVec3f& cur = mesh.getVertices()[i];
        if(mergedMesh.getNumVertices() > 0) {
            int nearestIndex = findNearestVertex(mergedMesh.getVertices(), cur);
            const ofVec3f& nearestVertex = mergedMesh.getVertices()[nearestIndex];
            if(cur.squareDistance(nearestVertex) < squareTolerance) {
                remappedIndices.push_back(nearestIndex);
            } else {
                remappedIndices.push_back(mergedMesh.getNumVertices());
                mergedMesh.addVertex(cur);
            }
        } else {
            remappedIndices.push_back(0);
            mergedMesh.addVertex(cur);
        }
    }
    n = mesh.getNumIndices();
    for(int i = 0; i < n; i++) {
        mergedMesh.addIndex(remappedIndices[mesh.getIndex(i)]);
    }
    return mergedMesh;
}


void project(ofMesh& mesh, const ofCamera& camera, ofRectangle viewport) {
	ofMatrix4x4 modelViewProjectionMatrix = camera.getModelViewProjectionMatrix(viewport);
	viewport.width /= 2;
	viewport.height /= 2;
	for (int i = 0; i < mesh.getNumVertices(); i++) {
		ofVec3f& cur = mesh.getVerticesPointer()[i];
		ofVec3f CameraXYZ = cur * modelViewProjectionMatrix;
		cur.x = (CameraXYZ.x + 1.0f) * viewport.width + viewport.x;
		cur.y = (1.0f - CameraXYZ.y) * viewport.height + viewport.y;
		cur.z = CameraXYZ.z / 2;
	}
}


vector<ofMesh> getMeshes(ofxAssimpModelLoader& model) {
    vector<ofMesh> meshes;
    for(int i = 0; i < model.getNumMeshes(); i++) {
        meshes.push_back(model.getMesh(i));
    }
    return meshes;
}

ofMesh joinMeshes(vector<ofMesh>& meshes) {
	ofMesh mesh;
	for(int i = 0; i < meshes.size(); i++) {
		mesh.append(meshes[i]);
	}
	return mesh;
}

