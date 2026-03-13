#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxWarp.h"
#include "MonitorPortUtility.h"
#include "SharedSliceState.h"

#include "ofJson.h"
#include "GLFW/glfw3.h"

struct ProjectorSlot {
	string imagePath;
	int monitorX, monitorY;
	int monitorW, monitorH;
	bool found = false;
};

class ofApp_ControlWindow : public ofBaseApp {

public:
	ofApp_ControlWindow(std::shared_ptr<SharedSliceState> sharedState);
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

	void listGPUPortsInfo();

private:
	std::shared_ptr<SharedSliceState> sharedState;
	ofRectangle imageRect;
	int activeEdgeIndex = -1;
	float minEdgeSpacing = 0.02f;

	void updateImageRect();
};
