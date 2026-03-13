#pragma once

#include <string>
#include <vector>
#include <map>
#include <iostream>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wingdi.h> 

struct PortInfo {
	std::string portKey;
	std::string friendlyName;
	std::string outputTech;
	int instanceIndex = 0;
	int desktopX = 0;
	int desktopY = 0;
	int width = 0;
	int height = 0;
	bool isActive = false;
};

class MonitorPortUtility {
public:
	static std::vector<PortInfo> enumeratePorts();
	static std::string techToString(DISPLAYCONFIG_VIDEO_OUTPUT_TECHNOLOGY tech);
	//here tech is the technology used at that port HDMI/DP etc
};
