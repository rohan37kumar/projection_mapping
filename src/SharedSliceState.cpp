#include "SharedSliceState.h"
#include "ofLog.h"


void SharedSliceState::setupSlices(int n) {
	if (n < 1) n = 1;

	sliceEdges.resize(n + 1);
	for (int i = 0; i <= n; ++i)
		sliceEdges[i] = static_cast<float>(i) / static_cast<float>(n);


	outputWindows.resize(n);
	sliceToPortKey.resize(n, "");
	sliceBlend.resize(n); 
	warpDataCache.resize(n); 
}

int SharedSliceState::getSliceCount() const {
	if (sliceEdges.size() < 2) return 0;
	return static_cast<int>(sliceEdges.size()) - 1;
}

ofRectangle SharedSliceState::getSliceArea(int sliceIndex) const {
	int n = getSliceCount();
	if (sliceIndex < 0 || sliceIndex >= n || !texture.isAllocated())
		return ofRectangle(0, 0, 0, 0);

	float tw = texture.getWidth();
	float th = texture.getHeight();

	float x0 = sliceEdges[sliceIndex] * tw;
	float x1 = sliceEdges[sliceIndex + 1] * tw;

	return ofRectangle(x0, 0.f, x1 - x0, th);
}



const PortInfo * SharedSliceState::findPort(const std::string & key) const {
	if (key.empty()) return nullptr;
	for (const auto & p : discoveredPorts)
		if (p.portKey == key) return &p;
	return nullptr;
}

int SharedSliceState::findPortIndex(const std::string & key) const {
	if (key.empty()) return -1;
	for (int i = 0; i < (int)discoveredPorts.size(); ++i)
		if (discoveredPorts[i].portKey == key) return i;
	return -1;
}



void SharedSliceState::applyPortMapping() {
	int n = getSliceCount();
	for (int i = 0; i < n; ++i) {
		if (i >= (int)sliceToPortKey.size() || sliceToPortKey[i].empty()) continue;
		const PortInfo * port = findPort(sliceToPortKey[i]);
		if (!port) continue;
		if (i < (int)outputWindows.size() && outputWindows[i]) {
			outputWindows[i]->setWindowPosition(port->desktopX, port->desktopY);
			outputWindows[i]->setWindowShape(port->width, port->height);
		}
	}
}



bool SharedSliceState::saveConfig(const std::string & filename) const {
	ofJson j;
	j["sliceCount"] = getSliceCount();

	ofJson edges = ofJson::array();
	for (float e : sliceEdges)
		edges.push_back(e);
	j["sliceEdges"] = edges;

	ofJson keys = ofJson::array();
	for (const auto & k : sliceToPortKey)
		keys.push_back(k);
	j["sliceToPortKey"] = keys;

	bool ok = ofSaveJson(filename, j);
	if (ok)
		ofLogNotice("SharedSliceState") << "Config saved to " << filename;
	else
		ofLogError("SharedSliceState") << "Failed to save config to " << filename;
	return ok;
}



bool SharedSliceState::loadConfig(const std::string & filename) {
	if (!ofFile::doesFileExist(filename)) {
		ofLogWarning("SharedSliceState") << "Config file not found: " << filename;
		return false;
	}

	ofJson j = ofLoadJson(filename);
	if (j.is_null() || j.empty()) {
		ofLogError("SharedSliceState") << "Failed to parse config: " << filename;
		return false;
	}


	if (!j.contains("sliceCount") || !j["sliceCount"].is_number()) {
		ofLogError("SharedSliceState") << "Config missing 'sliceCount': " << filename;
		return false;
	}
	int n = j["sliceCount"].get<int>();
	if (n < 1) n = 1;


	if (j.contains("sliceEdges") && j["sliceEdges"].is_array()
		&& (int)j["sliceEdges"].size() == n + 1) {
		sliceEdges.resize(n + 1);
		for (int i = 0; i <= n; ++i)
			sliceEdges[i] = j["sliceEdges"][i].get<float>();

		sliceEdges[0] = 0.f;
		sliceEdges[n] = 1.f;
		for (int i = 1; i < n; ++i)
			sliceEdges[i] = ofClamp(sliceEdges[i], 0.f, 1.f);
	} else {
		sliceEdges.resize(n + 1);
		for (int i = 0; i <= n; ++i)
			sliceEdges[i] = static_cast<float>(i) / static_cast<float>(n);
	}


	sliceToPortKey.assign(n, "");
	if (j.contains("sliceToPortKey") && j["sliceToPortKey"].is_array()) {
		int numKeys = (int)j["sliceToPortKey"].size();
		for (int i = 0; i < n && i < numKeys; ++i)
			sliceToPortKey[i] = j["sliceToPortKey"][i].get<std::string>();
	}


	sliceBlend.resize(n);
	warpDataCache.resize(n);

	ofLogNotice("SharedSliceState")
		<< "Config loaded from " << filename
		<< "  (" << n << " slices)";
	return true;
}
