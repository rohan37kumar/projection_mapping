#pragma once
#include "MonitorPortUtility.h"
#include "SharedSliceState.h"
#include "ofJson.h"
#include "ofMain.h"
#include "ofxGui.h"

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

private:
	std::shared_ptr<SharedSliceState> sharedState;

	ofRectangle imageRect;
	int activeEdgeIndex = -1;
	float minEdgeSpacing = 0.02f;
	void updateImageRect();

	std::vector<int> portSelectorForSlice;
	struct SliceRow {
		ofRectangle prevBtn, nextBtn, label;
	};
	std::vector<SliceRow> sliceRows;
	ofRectangle applyBtn, saveBtn;
	void buildMappingPanel();
	void drawMappingPanel();
	void syncSelectorsFromState();
	void applyMapping();

	ofxPanel slicePanel;
	ofxIntSlider numSlicesSlider;

	struct BlendSlider {
		ofRectangle rect;
		int sliceIdx = 0;
		int paramIdx = 0; // 0=brightness  1=R  2=G  3=B
		float minVal = 0.f;
		float maxVal = 1.f;
	};
	std::vector<BlendSlider> blendSliders;
	int activeBlendSlider = -1;

	void buildBlendPanel();
	void drawBlendPanel();
	float getBlendPanelTop() const;
	float getBlendValue(int sliceIdx, int paramIdx) const;
	void setBlendValue(int sliceIdx, int paramIdx, float val);
	void applyMouseToBlendSlider(int idx, float mouseX);

	int warpCopySrc = 0;
	int warpCopyDst = 1;

	struct WarpCopyRects {
		ofRectangle panel;
		ofRectangle srcPrev, srcLabel, srcNext;
		ofRectangle dstPrev, dstLabel, dstNext;
		ofRectangle copyBtn;
	} wcp;

	void buildWarpCopyPanel();
	void drawWarpCopyPanel();
};
