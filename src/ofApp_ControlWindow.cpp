#include "ofApp_ControlWindow.h"
#include <algorithm>
#include <cmath>
#include <utility>

ofApp_ControlWindow::ofApp_ControlWindow(std::shared_ptr<SharedSliceState> sharedState)
	: sharedState(std::move(sharedState)) {
}

void ofApp_ControlWindow::setup() {
	ofSetLogLevel(OF_LOG_NOTICE);
	ofDisableArbTex();
	ofBackground(ofColor::black);

	ofImage image;
	image.setUseTexture(false);
	if (image.load("images/night_city.jpg")) {
		sharedState->texture.enableMipmap();
		sharedState->texture.loadData(image.getPixels());
		sharedState->ready = true;
		if (sharedState->sliceEdges.empty()) {
			sharedState->setupSlices(2);
		}
	} else {
		ofLogError("ofApp_ControlWindow::setup") << "Could not load image!";
	}

	updateImageRect();
}

void ofApp_ControlWindow::exit() {
	
}

void ofApp_ControlWindow::update() {

}

void ofApp_ControlWindow::draw() {
	ofBackground(ofColor::black);

	if (sharedState->ready && sharedState->texture.isAllocated()) {
		sharedState->texture.draw(imageRect);

		ofSetColor(ofColor::yellow);
		for (int i = 1; i < sharedState->getSliceCount(); ++i) {
			auto lineY = imageRect.y + imageRect.height * sharedState->sliceEdges[i];
			ofDrawLine(imageRect.x, lineY, imageRect.x + imageRect.width, lineY);
		}

		ofSetColor(ofColor::white);
		ofDrawBitmapStringHighlight("Drag the yellow lines to adjust horizontal slices", 10, 20);
	} else {
		ofSetColor(ofColor::white);
		ofDrawBitmapStringHighlight("Waiting for image load...", 10, 20);
	}
}

void ofApp_ControlWindow::keyPressed(int key) {
	
}


void listGPUPortsInfo()
{
	bool discoveryMode = true;
	if (discoveryMode) {
		//ofInit();

		std::cout << "\n===== PORT DISCOVERY RESULTS =====\n";
		auto ports = MonitorPortUtility::enumeratePorts();
		for (auto & p : ports) {
			std::cout << "Friendly Name : " << p.friendlyName << "\n"
					  << "Stable Key    : " << p.portKey << "\n"
					  << "Desktop Pos   : (" << p.desktopX << ", " << p.desktopY << ")\n"
					  << "Resolution    : " << p.width << "x" << p.height << "\n"
					  << "----------------------------------\n";
		}
		system("pause");
		return;
	}
}



void ofApp_ControlWindow::keyReleased(int key) {
}

void ofApp_ControlWindow::mouseMoved(int x, int y) {
}

void ofApp_ControlWindow::mouseDragged(int x, int y, int button) {
	if (activeEdgeIndex < 1 || activeEdgeIndex >= sharedState->getSliceCount()) {
		return;
	}

	auto normalized = (static_cast<float>(y) - imageRect.y) / imageRect.height;
	auto minY = sharedState->sliceEdges[activeEdgeIndex - 1] + minEdgeSpacing;
	auto maxY = sharedState->sliceEdges[activeEdgeIndex + 1] - minEdgeSpacing;
	sharedState->sliceEdges[activeEdgeIndex] = ofClamp(normalized, minY, maxY);
}

void ofApp_ControlWindow::mousePressed(int x, int y, int button) {
	if (!sharedState->ready || sharedState->getSliceCount() < 2) {
		return;
	}

	for (int i = 1; i < sharedState->getSliceCount(); ++i) {
		auto lineY = imageRect.y + imageRect.height * sharedState->sliceEdges[i];
		if (std::abs(lineY - y) <= 6.0f) {
			activeEdgeIndex = i;
			return;
		}
	}
}

void ofApp_ControlWindow::mouseReleased(int x, int y, int button) {
	activeEdgeIndex = -1;
}

void ofApp_ControlWindow::mouseEntered(int x, int y) {
}

void ofApp_ControlWindow::mouseExited(int x, int y) {
}

void ofApp_ControlWindow::windowResized(int w, int h) {
	updateImageRect();
}

void ofApp_ControlWindow::gotMessage(ofMessage msg) {
}

void ofApp_ControlWindow::dragEvent(ofDragInfo dragInfo) {
}

void ofApp_ControlWindow::updateImageRect() {
	if (!sharedState || !sharedState->texture.isAllocated()) {
		imageRect.set(0, 0, ofGetWidth(), ofGetHeight());
		return;
	}

	auto imageWidth = sharedState->texture.getWidth();
	auto imageHeight = sharedState->texture.getHeight();
	auto windowWidth = static_cast<float>(ofGetWidth());
	auto windowHeight = static_cast<float>(ofGetHeight());
	if (imageWidth <= 0.0f || imageHeight <= 0.0f) {
		imageRect.set(0, 0, windowWidth, windowHeight);
		return;
	}

	auto scale = std::min(windowWidth / imageWidth, windowHeight / imageHeight);
	auto drawWidth = imageWidth * scale;
	auto drawHeight = imageHeight * scale;
	imageRect.set((windowWidth - drawWidth) * 0.5f, (windowHeight - drawHeight) * 0.5f, drawWidth, drawHeight);
}


