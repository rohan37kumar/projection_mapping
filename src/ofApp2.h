#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxWarp.h"
#include "SharedSliceState.h"

class ofApp2 : public ofBaseApp {

public:
	ofApp2(std::shared_ptr<SharedSliceState> sharedState, int sliceIndex);
	void setup();
	void update();
	void draw();
	void exit();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	bool useBeginEnd;
	ofxWarpController warpController;
	std::vector<ofRectangle> srcAreas;

	ofxPanel controlValuesPanel;
	ofxIntField numControlsX;
	ofxIntField numControlsY;

private:
	std::shared_ptr<SharedSliceState> sharedState;
	int sliceIndex = 0;
	bool warpInitialized = false;

	void ensureWarpSetup();
};
