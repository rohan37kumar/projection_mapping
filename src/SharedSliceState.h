#pragma once

#include "MonitorPortUtility.h"
#include "ofJson.h"
#include "ofMain.h"

#include <memory>
#include <string>
#include <vector>



struct SharedSliceState {


	ofTexture texture;
	bool ready = false;


	std::vector<float> sliceEdges;


	void setupSlices(int n);


	int getSliceCount() const;


	ofRectangle getSliceArea(int sliceIndex) const;


	std::vector<std::shared_ptr<ofAppBaseWindow>> outputWindows;


	std::vector<PortInfo> discoveredPorts; 
	std::vector<std::string> sliceToPortKey;

	const PortInfo * findPort(const std::string & key) const;


	int findPortIndex(const std::string & key) const;


	void applyPortMapping();


	bool saveConfig(const std::string & filename = "projector_config.json") const;


	bool loadConfig(const std::string & filename = "projector_config.json");


	struct BlendSettings {
		float brightness = 1.0f;
		float luminanceR = 1.0f;
		float luminanceG = 1.0f;
		float luminanceB = 1.0f;
		bool dirty = false;
	};
	std::vector<BlendSettings> sliceBlend;



	std::vector<ofJson> warpDataCache;

	struct WarpCopyJob {
		int src = -1;
		int dst = -1;
		bool pending = false;
	};
	WarpCopyJob warpCopyJob;
};
