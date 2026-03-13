#include "ofApp_ControlWindow.h"
#include <algorithm>
#include <cmath>
#include <utility>

static const float PANEL_X = 10.f;
static const float PANEL_TOP = 120.f;
static const float ROW_H = 36.f;
static const float PANEL_W = 420.f;
static const float BTN_W = 28.f;
static const float LABEL_W = PANEL_W - BTN_W * 2.f - 8.f;

static const float BL_HDR_H = 18.f;
static const float BL_ROW_H = 24.f;
static const float BL_SL_H = 12.f;
static const float BL_SN_W = 50.f;
static const float BL_BR_LW = 18.f;
static const float BL_BR_W = 90.f;
static const float BL_LM_LW = 26.f;
static const float BL_C_W = 56.f;
static const float BL_GAP = 4.f;


static const float WC_W = 268.f;
static const float WC_ROW_H = 28.f;
static const float WC_BTN_W = 24.f;
static const float WC_LBL_OFF = 62.f;

ofApp_ControlWindow::ofApp_ControlWindow(std::shared_ptr<SharedSliceState> ss)
	: sharedState(std::move(ss)) { }

void ofApp_ControlWindow::setup() {
	ofSetLogLevel(OF_LOG_NOTICE);
	ofDisableArbTex();
	ofBackground(ofColor::black);

	ofImage image;
	image.setUseTexture(false);
	if (image.load("images/world_map_HD.jpg")) {
		sharedState->texture.enableMipmap();
		sharedState->texture.loadData(image.getPixels());
		sharedState->ready = true;
		if (sharedState->sliceEdges.empty())
			sharedState->setupSlices(2);
	} else {
		ofLogError("ofApp_ControlWindow::setup") << "Could not load image!";
	}

	// Slice-count panel
	slicePanel.setup("Slices");
	slicePanel.setPosition(PANEL_X, 10);
	numSlicesSlider.setup("numSlices", sharedState->getSliceCount(), 1, 8);
	slicePanel.add(&numSlicesSlider);

	// Ensure new shared-state vectors match slice count
	int n = sharedState->getSliceCount();
	if ((int)sharedState->sliceBlend.size() < n) sharedState->sliceBlend.resize(n);
	if ((int)sharedState->warpDataCache.size() < n) sharedState->warpDataCache.resize(n);

	syncSelectorsFromState();
	buildMappingPanel();
	buildBlendPanel();
	buildWarpCopyPanel();
	updateImageRect();
}

void ofApp_ControlWindow::exit() { }

void ofApp_ControlWindow::update() {
	int n = sharedState->getSliceCount();

	if ((int)numSlicesSlider != n) {
		sharedState->setupSlices((int)numSlicesSlider);
		n = sharedState->getSliceCount();

		// Keep new shared-state vectors in sync
		if ((int)sharedState->sliceBlend.size() < n)
			sharedState->sliceBlend.resize(n);
		if ((int)sharedState->warpDataCache.size() < n)
			sharedState->warpDataCache.resize(n);

		syncSelectorsFromState();
		buildMappingPanel();
		buildBlendPanel();

		// Clamp warp-copy selections to valid range
		warpCopySrc = std::min(warpCopySrc, n - 1);
		warpCopyDst = std::min(warpCopyDst, n - 1);
		buildWarpCopyPanel();
	}
}


void ofApp_ControlWindow::draw() {
	ofBackground(ofColor::black);

	if (sharedState->ready && sharedState->texture.isAllocated()) {
		sharedState->texture.draw(imageRect);

		ofSetColor(ofColor::yellow);
		for (int i = 1; i < sharedState->getSliceCount(); ++i) {
			float lx = imageRect.x + imageRect.width * sharedState->sliceEdges[i];
			ofDrawLine(lx, imageRect.y, lx, imageRect.y + imageRect.height);
		}
		ofSetColor(ofColor::white);
		ofDrawBitmapStringHighlight(
			"Drag yellow lines | [s] save config | [a] apply mapping",
			10, imageRect.y + imageRect.height + 16);
	}

	slicePanel.draw();
	drawMappingPanel();
	drawBlendPanel();
	drawWarpCopyPanel();
}



void ofApp_ControlWindow::buildMappingPanel() {
	int n = sharedState->getSliceCount();
	sliceRows.resize(n);
	for (int i = 0; i < n; ++i) {
		float y = PANEL_TOP + i * ROW_H;
		sliceRows[i].prevBtn = ofRectangle(PANEL_X, y, BTN_W, ROW_H - 4);
		sliceRows[i].label = ofRectangle(PANEL_X + BTN_W + 4, y, LABEL_W, ROW_H - 4);
		sliceRows[i].nextBtn = ofRectangle(PANEL_X + BTN_W + 4 + LABEL_W + 4, y, BTN_W, ROW_H - 4);
	}
	float btnY = PANEL_TOP + n * ROW_H + 8;
	applyBtn = ofRectangle(PANEL_X, btnY, 100, 28);
	saveBtn = ofRectangle(PANEL_X + 110, btnY, 100, 28);
}

void ofApp_ControlWindow::drawMappingPanel() {
	int n = sharedState->getSliceCount();
	int numPorts = (int)sharedState->discoveredPorts.size();

	ofSetColor(200, 200, 200);
	ofDrawBitmapString("── Slice → Port Mapping ──", PANEL_X, PANEL_TOP - 6);

	for (int i = 0; i < n && i < (int)sliceRows.size(); ++i) {
		auto & row = sliceRows[i];
		int sel = portSelectorForSlice[i];

		ofSetColor(80, 80, 80);
		ofDrawRectRounded(row.prevBtn, 3);
		ofSetColor(220, 220, 220);
		ofDrawBitmapString("<", row.prevBtn.x + 8, row.prevBtn.y + 16);

		std::string lbl = "Slice " + std::to_string(i + 1) + ":  ";
		if (sel < 0 || numPorts == 0)
			lbl += "[Unassigned]";
		else
			lbl += sharedState->discoveredPorts[sel].friendlyName
				+ " (" + std::to_string(sharedState->discoveredPorts[sel].width)
				+ "x" + std::to_string(sharedState->discoveredPorts[sel].height) + ")";

		ofSetColor(40, 40, 60);
		ofDrawRectRounded(row.label, 3);
		ofSetColor(220, 220, 220);
		ofDrawBitmapString(lbl, row.label.x + 6, row.label.y + 16);

		ofSetColor(80, 80, 80);
		ofDrawRectRounded(row.nextBtn, 3);
		ofSetColor(220, 220, 220);
		ofDrawBitmapString(">", row.nextBtn.x + 8, row.nextBtn.y + 16);
	}

	ofSetColor(30, 100, 30);
	ofDrawRectRounded(applyBtn, 4);
	ofSetColor(255);
	ofDrawBitmapString("Apply", applyBtn.x + 14, applyBtn.y + 18);

	ofSetColor(20, 60, 120);
	ofDrawRectRounded(saveBtn, 4);
	ofSetColor(255);
	ofDrawBitmapString("Save", saveBtn.x + 16, saveBtn.y + 18);
}

void ofApp_ControlWindow::syncSelectorsFromState() {
	int n = sharedState->getSliceCount();
	portSelectorForSlice.assign(n, -1);
	for (int i = 0; i < n; ++i) {
		if (i < (int)sharedState->sliceToPortKey.size()
			&& !sharedState->sliceToPortKey[i].empty()) {
			portSelectorForSlice[i] = sharedState->findPortIndex(sharedState->sliceToPortKey[i]);
		}
	}
}

void ofApp_ControlWindow::applyMapping() {
	int n = sharedState->getSliceCount();
	sharedState->sliceToPortKey.resize(n, "");
	for (int i = 0; i < n; ++i) {
		int sel = portSelectorForSlice[i];
		sharedState->sliceToPortKey[i] = (sel >= 0 && sel < (int)sharedState->discoveredPorts.size())
			? sharedState->discoveredPorts[sel].portKey
			: "";
	}
	sharedState->applyPortMapping();
	ofLogNotice("ofApp_ControlWindow") << "Port mapping applied.";
}



float ofApp_ControlWindow::getBlendPanelTop() const {
	int n = sharedState->getSliceCount();
	// Below the Apply/Save row (y = PANEL_TOP + n*ROW_H + 8, height 28) + margin
	return PANEL_TOP + n * ROW_H + 8.f + 28.f + 20.f;
}

void ofApp_ControlWindow::buildBlendPanel() {
	blendSliders.clear();
	int n = sharedState->getSliceCount();
	float top = getBlendPanelTop() + BL_HDR_H + 4.f;

	for (int i = 0; i < n; ++i) {
		float ry = top + i * (BL_ROW_H + 4.f);
		float slY = ry + (BL_ROW_H - BL_SL_H) * 0.5f;

		// Brightness slider
		float bx = PANEL_X + BL_SN_W + BL_BR_LW;
		BlendSlider br;
		br.rect.set(bx, slY, BL_BR_W, BL_SL_H);
		br.sliceIdx = i;
		br.paramIdx = 0;
		br.minVal = 0.f;
		br.maxVal = 1.f;
		blendSliders.push_back(br);

		// R, G, B sliders
		float cx = bx + BL_BR_W + BL_GAP + BL_LM_LW + BL_GAP;
		for (int p = 1; p <= 3; ++p) {
			BlendSlider c;
			c.rect.set(cx + (p - 1) * (BL_C_W + BL_GAP), slY, BL_C_W, BL_SL_H);
			c.sliceIdx = i;
			c.paramIdx = p;
			c.minVal = 0.f;
			c.maxVal = 2.f;
			blendSliders.push_back(c);
		}
	}
}

void ofApp_ControlWindow::drawBlendPanel() {
	int n = sharedState->getSliceCount();
	if ((int)sharedState->sliceBlend.size() < n) return;

	float headerY = getBlendPanelTop();
	ofSetColor(200, 200, 200);
	ofDrawBitmapString("── Brightness / Luminance ──", PANEL_X, headerY + 11.f);

	float top = headerY + BL_HDR_H + 4.f;

	// Slice labels and text markers
	for (int i = 0; i < n; ++i) {
		float ry = top + i * (BL_ROW_H + 4.f);
		float midY = ry + BL_ROW_H * 0.5f + 5.f;

		// "Slice N" badge
		ofSetColor(35, 35, 55);
		ofDrawRectRounded(ofRectangle(PANEL_X, ry + 2.f, BL_SN_W - 2.f, BL_ROW_H - 4.f), 2);
		ofSetColor(170, 170, 200);
		ofDrawBitmapString("S" + std::to_string(i + 1), PANEL_X + 4.f, midY);

		// "Br" and "Lum" text labels
		ofSetColor(140, 140, 140);
		ofDrawBitmapString("Br", PANEL_X + BL_SN_W, midY);
		float lumX = PANEL_X + BL_SN_W + BL_BR_LW + BL_BR_W + BL_GAP;
		ofDrawBitmapString("Lum", lumX, midY);
	}

	// Slider bars
	static const ofColor trackCol(25, 25, 25);
	static const ofColor borderCol(70, 70, 70);
	static const ofColor fillCols[4] = {
		ofColor(200, 200, 200), // brightness – white
		ofColor(190, 55, 55), // R
		ofColor(55, 185, 55), // G
		ofColor(55, 90, 210) // B
	};

	for (const auto & sl : blendSliders) {
		float val = getBlendValue(sl.sliceIdx, sl.paramIdx);
		float t = ofClamp((val - sl.minVal) / (sl.maxVal - sl.minVal), 0.f, 1.f);
		float fillW = t * sl.rect.width;

		// Track
		ofSetColor(trackCol);
		ofDrawRectRounded(sl.rect, 2);

		// Fill
		if (fillW > 1.f) {
			ofSetColor(fillCols[sl.paramIdx]);
			ofDrawRectRounded(
				ofRectangle(sl.rect.x, sl.rect.y, fillW, sl.rect.height), 2);
		}

		// Border
		ofSetColor(borderCol);
		ofNoFill();
		ofDrawRectRounded(sl.rect, 2);
		ofFill();
	}
}

float ofApp_ControlWindow::getBlendValue(int si, int pi) const {
	if (si < 0 || si >= (int)sharedState->sliceBlend.size()) return 1.f;
	const auto & b = sharedState->sliceBlend[si];
	switch (pi) {
	case 0:
		return b.brightness;
	case 1:
		return b.luminanceR;
	case 2:
		return b.luminanceG;
	case 3:
		return b.luminanceB;
	}
	return 1.f;
}

void ofApp_ControlWindow::setBlendValue(int si, int pi, float val) {
	if (si < 0 || si >= (int)sharedState->sliceBlend.size()) return;
	auto & b = sharedState->sliceBlend[si];
	switch (pi) {
	case 0:
		b.brightness = val;
		break;
	case 1:
		b.luminanceR = val;
		break;
	case 2:
		b.luminanceG = val;
		break;
	case 3:
		b.luminanceB = val;
		break;
	}
	b.dirty = true;
}

void ofApp_ControlWindow::applyMouseToBlendSlider(int idx, float mx) {
	const auto & sl = blendSliders[idx];
	float t = ofClamp((mx - sl.rect.x) / sl.rect.width, 0.f, 1.f);
	setBlendValue(sl.sliceIdx, sl.paramIdx, sl.minVal + t * (sl.maxVal - sl.minVal));
}



void ofApp_ControlWindow::buildWarpCopyPanel() {
	int n = sharedState->getSliceCount();
	if (n > 0) {
		warpCopySrc = ofClamp(warpCopySrc, 0, n - 1);
		warpCopyDst = ofClamp(warpCopyDst, 0, n - 1);
	}

	float px = ofGetWidth() - WC_W - 12.f;
	float py = 12.f;


	float innerW = WC_W - WC_LBL_OFF - WC_BTN_W * 2.f - 16.f;
	float bx = px + WC_LBL_OFF;


	float ry0 = py + 22.f;
	wcp.srcPrev.set(bx, ry0 + 2.f, WC_BTN_W, WC_ROW_H - 4.f);
	wcp.srcLabel.set(bx + WC_BTN_W + 4.f, ry0 + 2.f, innerW, WC_ROW_H - 4.f);
	wcp.srcNext.set(wcp.srcLabel.getRight() + 4.f, ry0 + 2.f, WC_BTN_W, WC_ROW_H - 4.f);


	float ry1 = ry0 + WC_ROW_H + 2.f;
	wcp.dstPrev.set(bx, ry1 + 2.f, WC_BTN_W, WC_ROW_H - 4.f);
	wcp.dstLabel.set(bx + WC_BTN_W + 4.f, ry1 + 2.f, innerW, WC_ROW_H - 4.f);
	wcp.dstNext.set(wcp.dstLabel.getRight() + 4.f, ry1 + 2.f, WC_BTN_W, WC_ROW_H - 4.f);


	float copyY = ry1 + WC_ROW_H + 6.f;
	wcp.copyBtn.set(px + 8.f, copyY, WC_W - 16.f, 26.f);

	wcp.panel.set(px, py, WC_W, copyY + 26.f + 10.f - py);
}

void ofApp_ControlWindow::drawWarpCopyPanel() {
	int n = sharedState->getSliceCount();
	if (n < 1) return;


	ofSetColor(20, 20, 38);
	ofDrawRectRounded(wcp.panel, 6);
	ofSetColor(60, 60, 90);
	ofNoFill();
	ofDrawRectRounded(wcp.panel, 6);
	ofFill();


	ofSetColor(190, 190, 210);
	ofDrawBitmapString("── Warp Copy ──", wcp.panel.x + 10.f, wcp.panel.y + 14.f);


	float srcMidY = wcp.srcPrev.y + wcp.srcPrev.height * 0.5f + 5.f;
	ofSetColor(130, 130, 150);
	ofDrawBitmapString("Source:", wcp.panel.x + 7.f, srcMidY);

	ofSetColor(70, 70, 80);
	ofDrawRectRounded(wcp.srcPrev, 3);
	ofSetColor(220, 220, 220);
	ofDrawBitmapString("<", wcp.srcPrev.x + 7.f, wcp.srcPrev.y + 15.f);

	ofSetColor(35, 35, 60);
	ofDrawRectRounded(wcp.srcLabel, 3);
	ofSetColor(220, 220, 220);
	ofDrawBitmapString("Slice " + std::to_string(warpCopySrc + 1),
		wcp.srcLabel.x + 6.f, wcp.srcLabel.y + 15.f);

	ofSetColor(70, 70, 80);
	ofDrawRectRounded(wcp.srcNext, 3);
	ofSetColor(220, 220, 220);
	ofDrawBitmapString(">", wcp.srcNext.x + 7.f, wcp.srcNext.y + 15.f);


	float dstMidY = wcp.dstPrev.y + wcp.dstPrev.height * 0.5f + 5.f;
	ofSetColor(130, 130, 150);
	ofDrawBitmapString("Target:", wcp.panel.x + 7.f, dstMidY);

	ofSetColor(70, 70, 80);
	ofDrawRectRounded(wcp.dstPrev, 3);
	ofSetColor(220, 220, 220);
	ofDrawBitmapString("<", wcp.dstPrev.x + 7.f, wcp.dstPrev.y + 15.f);

	ofSetColor(35, 35, 60);
	ofDrawRectRounded(wcp.dstLabel, 3);
	ofSetColor(220, 220, 220);
	ofDrawBitmapString("Slice " + std::to_string(warpCopyDst + 1),
		wcp.dstLabel.x + 6.f, wcp.dstLabel.y + 15.f);

	ofSetColor(70, 70, 80);
	ofDrawRectRounded(wcp.dstNext, 3);
	ofSetColor(220, 220, 220);
	ofDrawBitmapString(">", wcp.dstNext.x + 7.f, wcp.dstNext.y + 15.f);


	bool same = (warpCopySrc == warpCopyDst);
	ofSetColor(same ? ofColor(50, 30, 60) : ofColor(90, 40, 130));
	ofDrawRectRounded(wcp.copyBtn, 4);
	ofSetColor(same ? ofColor(100, 80, 110) : ofColor(255));
	ofDrawBitmapString("Copy Control Points",
		wcp.copyBtn.x + 12.f, wcp.copyBtn.y + 17.f);
}



void ofApp_ControlWindow::mousePressed(int x, int y, int button) {
	int numPorts = (int)sharedState->discoveredPorts.size();
	int n = sharedState->getSliceCount();

	//── Port-mapping buttons ────────────────────────────────────────────
	for (int i = 0; i < n && i < (int)sliceRows.size(); ++i) {
		if (sliceRows[i].prevBtn.inside(x, y)) {
			portSelectorForSlice[i] = (portSelectorForSlice[i] <= -1)
				? (numPorts - 1)
				: (portSelectorForSlice[i] - 1);
			if (portSelectorForSlice[i] < -1) portSelectorForSlice[i] = numPorts - 1;
			return;
		}
		if (sliceRows[i].nextBtn.inside(x, y)) {
			portSelectorForSlice[i]++;
			if (portSelectorForSlice[i] >= numPorts) portSelectorForSlice[i] = -1;
			return;
		}
	}
	if (applyBtn.inside(x, y)) {
		applyMapping();
		return;
	}
	if (saveBtn.inside(x, y)) {
		applyMapping();
		sharedState->saveConfig();
		return;
	}


	for (int i = 0; i < (int)blendSliders.size(); ++i) {
		if (blendSliders[i].rect.inside(x, y)) {
			activeBlendSlider = i;
			applyMouseToBlendSlider(i, (float)x);
			return;
		}
	}


	if (wcp.srcPrev.inside(x, y)) {
		warpCopySrc = (warpCopySrc - 1 + n) % n;
		return;
	}
	if (wcp.srcNext.inside(x, y)) {
		warpCopySrc = (warpCopySrc + 1) % n;
		return;
	}
	if (wcp.dstPrev.inside(x, y)) {
		warpCopyDst = (warpCopyDst - 1 + n) % n;
		return;
	}
	if (wcp.dstNext.inside(x, y)) {
		warpCopyDst = (warpCopyDst + 1) % n;
		return;
	}
	if (wcp.copyBtn.inside(x, y)) {
		if (warpCopySrc != warpCopyDst) {
			sharedState->warpCopyJob = { warpCopySrc, warpCopyDst, true };
			ofLogNotice("WarpCopy") << "Queued copy: slice "
									<< (warpCopySrc + 1) << " → " << (warpCopyDst + 1);
		} else {
			ofLogWarning("WarpCopy") << "Source and target slice are the same — skipped.";
		}
		return;
	}


	if (!sharedState->ready || sharedState->getSliceCount() < 2) return;
	for (int i = 1; i < sharedState->getSliceCount(); ++i) {
		float lx = imageRect.x + imageRect.width * sharedState->sliceEdges[i];
		if (std::abs(lx - (float)x) <= 6.f) {
			activeEdgeIndex = i;
			return;
		}
	}
}

void ofApp_ControlWindow::mouseDragged(int x, int y, int button) {

	if (activeBlendSlider >= 0) {
		applyMouseToBlendSlider(activeBlendSlider, (float)x);
		return;
	}

	// Slice-edge drag
	if (activeEdgeIndex < 1 || activeEdgeIndex >= sharedState->getSliceCount()) return;
	float norm = (static_cast<float>(x) - imageRect.x) / imageRect.width;
	float minX = sharedState->sliceEdges[activeEdgeIndex - 1] + minEdgeSpacing;
	float maxX = sharedState->sliceEdges[activeEdgeIndex + 1] - minEdgeSpacing;
	sharedState->sliceEdges[activeEdgeIndex] = ofClamp(norm, minX, maxX);
}

void ofApp_ControlWindow::mouseReleased(int x, int y, int button) {
	activeEdgeIndex = -1;
	activeBlendSlider = -1;
}

void ofApp_ControlWindow::keyPressed(int key) {
	if (key == 's' || key == 'S') {
		applyMapping();
		sharedState->saveConfig();
	}
	if (key == 'a' || key == 'A') {
		applyMapping();
	}
}

void ofApp_ControlWindow::windowResized(int w, int h) {
	updateImageRect();
	buildMappingPanel();
	buildBlendPanel();
	buildWarpCopyPanel();
}

void ofApp_ControlWindow::updateImageRect() {
	if (!sharedState || !sharedState->texture.isAllocated()) {
		imageRect.set(0, 0, ofGetWidth(), ofGetHeight());
		return;
	}
	float iw = sharedState->texture.getWidth();
	float ih = sharedState->texture.getHeight();
	float ww = (float)ofGetWidth();
	float wh = (float)ofGetHeight();
	if (iw <= 0.f || ih <= 0.f) {
		imageRect.set(0, 0, ww, wh);
		return;
	}

	float availX = PANEL_X + PANEL_W + 20.f;
	float availW = ww - availX - 10.f;
	float scale = std::min(availW / iw, wh / ih);
	float dw = iw * scale, dh = ih * scale;
	imageRect.set(availX + (availW - dw) * 0.5f, (wh - dh) * 0.5f, dw, dh);
}


void ofApp_ControlWindow::keyReleased(int key) { }
void ofApp_ControlWindow::mouseMoved(int x, int y) { }
void ofApp_ControlWindow::mouseEntered(int x, int y) { }
void ofApp_ControlWindow::mouseExited(int x, int y) { }
void ofApp_ControlWindow::gotMessage(ofMessage msg) { }
void ofApp_ControlWindow::dragEvent(ofDragInfo di) { }
