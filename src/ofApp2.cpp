#include "ofApp2.h"

void ofApp2::setup() {
	ofSetLogLevel(OF_LOG_NOTICE);
	ofDisableArbTex();
	ofBackground(ofColor::black);

	// Setup warp controller for this window.
	this->warpController.setup();

	ofImage image;
	image.setUseTexture(false);
	if (!image.load("images/world_map.jpg")) {
		ofLogError("ofApp2::setup") << "Could not load image!";
		return;
	}
	this->texture.enableMipmap();
	this->texture.loadData(image.getPixels());

	this->warpController.loadSettings("settings_window2.json");
	if (this->warpController.getWarps().empty()) {
		std::shared_ptr<ofxWarpPerspectiveBilinear> warp;

		warp = this->warpController.buildWarp<ofxWarpPerspectiveBilinear>();
		warp->setNumControlsX(3);
		warp->setNumControlsY(3);
		warp->setSize(this->texture.getWidth(), this->texture.getHeight());
		warp->setEdges(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	}

	this->srcAreas.resize(this->warpController.getNumWarps());

	this->areaMode = -1;
	this->keyPressed('a');

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
}

void ofApp2::draw() {
	ofBackground(ofColor::black);

	if (this->texture.isAllocated()) {
		for (auto i = 0; i < this->warpController.getNumWarps(); ++i) {
			auto warp = this->warpController.getWarp(i);
			if (this->useBeginEnd) {
				warp->begin();
				{
					auto bounds = warp->getBounds();
					this->texture.drawSubsection(
						bounds.x, bounds.y, bounds.width, bounds.height,
						this->srcAreas[i].x, this->srcAreas[i].y,
						this->srcAreas[i].width, this->srcAreas[i].height);
				}
				warp->end();
			} else {
				warp->draw(this->texture, this->srcAreas[i]);
			}
		}
	}

	std::ostringstream oss;
	oss << ofToString(ofGetFrameRate(), 2) << " fps" << std::endl;
	oss << "[a]rea mode: " << areaName << std::endl;
	oss << "[w]arp edit: " << (this->warpController.getWarp(0)->isEditing() ? "on" : "off");
	ofSetColor(ofColor::white);
	ofDrawBitmapStringHighlight(oss.str(), 10, 20);

	controlValuesPanel.draw();
}

void ofApp2::keyPressed(int key) {
	if (key == 'f') {
		ofToggleFullscreen();
	} else if (key == 'a') {
		this->areaMode = (this->areaMode + 1) % 3;
		if (this->areaMode == 0) {
			// Draw the full image for each warp.
			auto area = ofRectangle(0, 0, this->texture.getWidth(), this->texture.getHeight());
			for (auto i = 0; i < this->warpController.getNumWarps(); ++i) {
				this->srcAreas[i] = area;
			}
			this->areaName = "full";
		} else if (this->areaMode == 1) {
			// Draw a corner region of the image so that all warps make up the entire image.
			for (auto i = 0; i < this->warpController.getNumWarps(); ++i) {
				static const auto overlap = 10.0f;
				if (i == 0) {
					// Top-left.
					this->srcAreas[i] = ofRectangle(0, 0,
						this->texture.getWidth() * 0.5f + overlap,
						this->texture.getHeight() * 0.5f + overlap);
				} else if (i == 1) {
					// Top-right.
					this->srcAreas[i] = ofRectangle(
						this->texture.getWidth() * 0.5f - overlap, 0,
						this->texture.getWidth() * 0.5f + overlap,
						this->texture.getHeight() * 0.5f + overlap);
				} else if (i == 2) {
					// Bottom-right.
					this->srcAreas[i] = ofRectangle(
						this->texture.getWidth() * 0.5f - overlap,
						this->texture.getHeight() * 0.5f - overlap,
						this->texture.getWidth() * 0.5f + overlap,
						this->texture.getHeight() * 0.5f + overlap);
				} else {
					// Bottom-left.
					this->srcAreas[i] = ofRectangle(0,
						this->texture.getHeight() * 0.5f - overlap,
						this->texture.getWidth() * 0.5f + overlap,
						this->texture.getHeight() * 0.5f + overlap);
				}
			}
			this->areaName = "corners";
		} else {
			// Draw a random region of the image for each warp.
			auto x1 = ofRandom(0, this->texture.getWidth() - 150);
			auto y1 = ofRandom(0, this->texture.getHeight() - 150);
			auto x2 = ofRandom(x1 + 150, this->texture.getWidth());
			auto y2 = ofRandom(y1 + 150, this->texture.getHeight());
			auto area = ofRectangle(x1, y1, x2 - x1, y2 - y1);
			for (auto i = 0; i < this->warpController.getNumWarps(); ++i) {
				this->srcAreas[i] = area;
			}
			this->areaName = "random";
		}
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
