#include "ofApp1.h"
#include <cstdio>
#include <sstream>
#include <utility>

ofApp1::ofApp1(std::shared_ptr<SharedSliceState> sharedState, int sliceIndex)
	: sharedState(std::move(sharedState))
	, sliceIndex(sliceIndex) {
	settingsFile = "settings_window_" + std::to_string(sliceIndex) + ".json";
}

void ofApp1::setup() {
	ofSetLogLevel(OF_LOG_NOTICE);
	ofDisableArbTex();
	ofBackground(ofColor::black);

	if (sharedState && sliceIndex < (int)sharedState->outputWindows.size())
		sharedState->outputWindows[sliceIndex] = ofGetCurrentWindow();

	warpController.setup();
	warpController.loadSettings(settingsFile);
	useBeginEnd = false;

	controlValuesPanel.setup();
	controlValuesPanel.setPosition(10, 80);
	controlValuesPanel.add(numControlsX.setup("numControlsX_w" + std::to_string(sliceIndex), 4, 2, 10));
	controlValuesPanel.add(numControlsY.setup("numControlsY_w" + std::to_string(sliceIndex), 4, 2, 10));
}

void ofApp1::exit() {
	warpController.saveSettings(settingsFile);
}

void ofApp1::update() {
	ofSetWindowTitle("Window " + std::to_string(sliceIndex)
		+ " :: " + ofToString(ofGetFrameRate(), 2) + " FPS");


	if (sharedState) {
		int n = sharedState->getSliceCount();
		if ((int)sharedState->sliceBlend.size() < n)
			sharedState->sliceBlend.resize(n);
		if ((int)sharedState->warpDataCache.size() < n)
			sharedState->warpDataCache.resize(n);
	}

	ensureWarpSetup();

	if (!warpInitialized || !sharedState) return;


	auto sliceArea = sharedState->getSliceArea(sliceIndex);
	for (int i = 0; i < warpController.getNumWarps(); ++i)
		srcAreas[i] = sliceArea;


	if (sliceIndex < (int)sharedState->sliceBlend.size()) {
		auto & blend = sharedState->sliceBlend[sliceIndex];
		if (blend.dirty) {
			applyBlendToWarps(blend);
			blend.dirty = false;
		}
	}


	if (sliceIndex < (int)sharedState->warpDataCache.size()) {
		warpController.serialize(sharedState->warpDataCache[sliceIndex]);
	}


	auto & job = sharedState->warpCopyJob;
	if (job.pending && job.dst == sliceIndex) {
		int src = job.src;
		bool srcValid = (src != sliceIndex)
			&& (src >= 0)
			&& (src < (int)sharedState->warpDataCache.size())
			&& sharedState->warpDataCache[src].contains("warps");
		if (srcValid) {
			copyWarpGeometryFrom(sharedState->warpDataCache[src]);
		}
		job.pending = false;
	}
}


void ofApp1::applyBlendToWarps(const SharedSliceState::BlendSettings & blend) {
	ofJson j;
	warpController.serialize(j);
	if (!j.contains("warps")) return;

	for (auto & wj : j["warps"]) {
		// Brightness
		wj["brightness"] = blend.brightness;

		// Luminance – stored as the string "R, G, B" matching ofxWarp's format
		if (!wj.contains("blend")) wj["blend"] = ofJson::object();
		char buf[80];
		std::snprintf(buf, sizeof(buf), "%.6f, %.6f, %.6f",
			blend.luminanceR, blend.luminanceG, blend.luminanceB);
		wj["blend"]["luminance"] = std::string(buf);
	}

	warpController.deserialize(j);
	restoreWarpSizes();
}


void ofApp1::copyWarpGeometryFrom(const ofJson & srcData) {
	// Capture the target's current full state
	ofJson myData;
	warpController.serialize(myData);

	if (!myData.contains("warps") || !srcData.contains("warps")) return;

	int nWarps = (int)std::min(srcData["warps"].size(), myData["warps"].size());
	for (int i = 0; i < nWarps; ++i) {
		const auto & sw = srcData["warps"][i];
		auto & mw = myData["warps"][i];


		if (sw.contains("corners"))
			mw["corners"] = sw["corners"];


		if (sw.contains("warp"))
			mw["warp"] = sw["warp"];

	}

	warpController.deserialize(myData);
	restoreWarpSizes();

	ofLogNotice("ofApp1") << "Slice " << (sliceIndex + 1)
						  << ": warp geometry copied from slice "
						  << (sharedState->warpCopyJob.src + 1);
}


void ofApp1::restoreWarpSizes() {
	if (!sharedState || !sharedState->texture.isAllocated()) return;
	float tw = sharedState->texture.getWidth();
	float th = sharedState->texture.getHeight();
	int nw = warpController.getNumWarps();
	for (int i = 0; i < nw; ++i)
		warpController.getWarp(i)->setSize(tw, th);
	srcAreas.resize(nw);
}


void ofApp1::draw() {
	ofBackground(ofColor::black);

	if (warpInitialized && sharedState->ready && sharedState->texture.isAllocated()) {
		for (int i = 0; i < warpController.getNumWarps(); ++i) {
			auto warp = warpController.getWarp(i);
			if (useBeginEnd) {
				warp->begin();
				auto bounds = warp->getBounds();
				sharedState->texture.drawSubsection(
					bounds.x, bounds.y, bounds.width, bounds.height,
					srcAreas[i].x, srcAreas[i].y, srcAreas[i].width, srcAreas[i].height);
				warp->end();
			} else {
				warp->draw(sharedState->texture, srcAreas[i]);
			}
		}
	}

	std::ostringstream oss;
	oss << ofToString(ofGetFrameRate(), 2) << " fps\n";
	if (sharedState)
		oss << "slice " << (sliceIndex + 1) << "/" << sharedState->getSliceCount() << "\n";
	auto warp0 = warpController.getWarp(0);
	oss << "[w]arp edit: " << (warp0 && warp0->isEditing() ? "on" : "off");
	ofSetColor(ofColor::white);
	ofDrawBitmapStringHighlight(oss.str(), 10, 20);

	controlValuesPanel.draw();
}


void ofApp1::keyPressed(int key) {
	if (key == 'f') {
		ofToggleFullscreen();
	} else if (key == 'r') {
		for (int i = 0; i < warpController.getNumWarps(); ++i) {
			auto warpPB = std::dynamic_pointer_cast<ofxWarpPerspectiveBilinear>(
				warpController.getWarp(i));
			if (warpPB) {
				warpPB->setNumControlsX(numControlsX);
				warpPB->setNumControlsY(numControlsY);
				warpPB->reset();
			}
		}
	} else if (key == 'd') {
		useBeginEnd ^= 1;
	}
}


void ofApp1::ensureWarpSetup() {
	if (warpInitialized || !sharedState || !sharedState->ready
		|| !sharedState->texture.isAllocated()) return;

	if (warpController.getWarps().empty()) {
		auto warp = warpController.buildWarp<ofxWarpPerspectiveBilinear>();
		warp->setNumControlsX(3);
		warp->setNumControlsY(3);
		warp->setBrightness(1.0f);
		warp->setExponent(1.0f);
		warp->setEdges(glm::vec4(0.f, 0.f, 0.f, 0.f));
	}

	restoreWarpSizes();
	warpInitialized = true;


	if (sliceIndex < (int)sharedState->sliceBlend.size()) {
		auto & blend = sharedState->sliceBlend[sliceIndex];
		ofJson j;
		warpController.serialize(j);
		if (j.contains("warps") && !j["warps"].empty()) {
			const auto & w0 = j["warps"][0];

			if (w0.contains("brightness"))
				blend.brightness = w0["brightness"].get<float>();

			if (w0.contains("blend") && w0["blend"].contains("luminance")) {
				std::string s = w0["blend"]["luminance"].get<std::string>();
				float r = 1.f, g = 1.f, b = 1.f;
				// ofxWarp serialises luminance as "R, G, B"
				std::sscanf(s.c_str(), "%f, %f, %f", &r, &g, &b);
				blend.luminanceR = r;
				blend.luminanceG = g;
				blend.luminanceB = b;
			}
		}
		blend.dirty = false;
	}
}


void ofApp1::keyReleased(int key) { }
void ofApp1::mouseMoved(int x, int y) { }
void ofApp1::mouseDragged(int x, int y, int button) { }
void ofApp1::mousePressed(int x, int y, int button) { }
void ofApp1::mouseReleased(int x, int y, int button) { }
void ofApp1::mouseEntered(int x, int y) { }
void ofApp1::mouseExited(int x, int y) { }
void ofApp1::windowResized(int w, int h) { }
void ofApp1::gotMessage(ofMessage msg) { }
void ofApp1::dragEvent(ofDragInfo dragInfo) { }
