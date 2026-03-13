#include "ofApp2.h"
#include <utility>

ofApp2::ofApp2(std::shared_ptr<SharedSliceState> sharedState, int sliceIndex)
	: sharedState(std::move(sharedState)), sliceIndex(sliceIndex) {
}

void ofApp2::setup() {
	ofSetLogLevel(OF_LOG_NOTICE);
	ofDisableArbTex();
	ofBackground(ofColor::black);

	// Setup warp controller for this window.
	this->warpController.setup();

	this->warpController.loadSettings("settings_window2.json");

	this->useBeginEnd = false;

	// GUI integrations
	controlValuesPanel.setup();
	controlValuesPanel.setPosition(10, 80);
	controlValuesPanel.add(numControlsX.setup("numControlsX_win2", 4, 2, 10));
	controlValuesPanel.add(numControlsY.setup("numControlsY_win2", 4, 2, 10));
}

void ofApp2::exit() {
	this->warpController.saveSettings("settings_window2.json");
}

void ofApp2::update() {
	ofSetWindowTitle("Window 2 :: " + ofToString(ofGetFrameRate(), 2) + " FPS ");
	ensureWarpSetup();

	if (warpInitialized) {
		auto sliceArea = sharedState->getSliceArea(sliceIndex);
		for (auto i = 0; i < this->warpController.getNumWarps(); ++i) {
			this->srcAreas[i] = sliceArea;
		}
	}
}

void ofApp2::draw() {
	ofBackground(ofColor::black);

	if (warpInitialized && sharedState->ready && sharedState->texture.isAllocated()) {
		for (auto i = 0; i < this->warpController.getNumWarps(); ++i) {
			auto warp = this->warpController.getWarp(i);
			if (this->useBeginEnd) {
				warp->begin();
				{
					auto bounds = warp->getBounds();
					sharedState->texture.drawSubsection(
						bounds.x, bounds.y, bounds.width, bounds.height,
						this->srcAreas[i].x, this->srcAreas[i].y,
						this->srcAreas[i].width, this->srcAreas[i].height);
				}
				warp->end();
			} else {
				warp->draw(sharedState->texture, this->srcAreas[i]);
			}
		}
	}

	std::ostringstream oss;
	oss << ofToString(ofGetFrameRate(), 2) << " fps" << std::endl;
	if (sharedState) {
		oss << "slice " << (sliceIndex + 1) << "/" << sharedState->getSliceCount() << std::endl;
	}
	auto warp0 = this->warpController.getWarp(0);
	oss << "[w]arp edit: " << (warp0 && warp0->isEditing() ? "on" : "off");
	ofSetColor(ofColor::white);
	ofDrawBitmapStringHighlight(oss.str(), 10, 20);

	controlValuesPanel.draw();
}

void ofApp2::keyPressed(int key) {
	if (key == 'f') {
		ofToggleFullscreen();
	} else if (key == 'r') {
		for (auto i = 0; i < this->warpController.getNumWarps(); ++i) {
			auto warp = this->warpController.getWarp(i);
			auto warpPB = std::dynamic_pointer_cast<ofxWarpPerspectiveBilinear>(warp);
			if (warpPB) {
				warpPB->setNumControlsX(numControlsX);
				warpPB->setNumControlsY(numControlsY);
				warpPB->reset();
			}
		}
	} else if (key == 'd') {
		this->useBeginEnd ^= 1;
	}
}

void ofApp2::keyReleased(int key) {
}

void ofApp2::mouseMoved(int x, int y) {
}

void ofApp2::mouseDragged(int x, int y, int button) {
}

void ofApp2::mousePressed(int x, int y, int button) {
}

void ofApp2::mouseReleased(int x, int y, int button) {
}

void ofApp2::mouseEntered(int x, int y) {
}

void ofApp2::mouseExited(int x, int y) {
}

void ofApp2::windowResized(int w, int h) {
}

void ofApp2::gotMessage(ofMessage msg) {
}

void ofApp2::dragEvent(ofDragInfo dragInfo) {
}

void ofApp2::ensureWarpSetup() {
	if (warpInitialized || !sharedState || !sharedState->ready || !sharedState->texture.isAllocated()) {
		return;
	}

	if (this->warpController.getWarps().empty()) {
		std::shared_ptr<ofxWarpPerspectiveBilinear> warp;

		warp = this->warpController.buildWarp<ofxWarpPerspectiveBilinear>();
		warp->setNumControlsX(3);
		warp->setNumControlsY(3);
		warp->setEdges(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	}

	for (auto i = 0; i < this->warpController.getNumWarps(); ++i) {
		auto warp = this->warpController.getWarp(i);
		warp->setSize(sharedState->texture.getWidth(), sharedState->texture.getHeight());
	}

	this->srcAreas.resize(this->warpController.getNumWarps());
	warpInitialized = true;
}
