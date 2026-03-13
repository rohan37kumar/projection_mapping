#include "MonitorPortUtility.h"

// QueryDisplayConfig and GetDisplayConfigBufferSizes both live in User32.lib
#pragma comment(lib, "User32.lib")

std::string MonitorPortUtility::techToString(DISPLAYCONFIG_VIDEO_OUTPUT_TECHNOLOGY tech) {
	switch (tech) {
	case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_HDMI:
		return "HDMI";
	case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DISPLAYPORT_EXTERNAL:
		return "DisplayPort";
	case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DISPLAYPORT_EMBEDDED:
		return "eDP";
	case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DVI:
		return "DVI";
	default:
		return "OTHER_" + std::to_string((int)tech);
	}
}

std::vector<PortInfo> MonitorPortUtility::enumeratePorts() {
	std::vector<PortInfo> results;

	DPI_AWARENESS_CONTEXT previousDpiContext = SetThreadDpiAwarenessContext(
		DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	UINT32 pathCount = 0, modeCount = 0;

	LONG bufferResult = GetDisplayConfigBufferSizes(
		QDC_ONLY_ACTIVE_PATHS,
		&pathCount,
		&modeCount);

	if (bufferResult != ERROR_SUCCESS) {
		std::cout << "[ERROR] GetDisplayConfigBufferSizes failed.\n"
				  << "        Returned : " << bufferResult << "\n";
		SetThreadDpiAwarenessContext(previousDpiContext);
		return results;
	}

	std::vector<DISPLAYCONFIG_PATH_INFO> paths(pathCount);
	std::vector<DISPLAYCONFIG_MODE_INFO> modes(modeCount);

	LONG queryResult = QueryDisplayConfig(
		QDC_ONLY_ACTIVE_PATHS,
		&pathCount, paths.data(),
		&modeCount, modes.data(),
		NULL
	);


	SetThreadDpiAwarenessContext(previousDpiContext);

	if (queryResult != ERROR_SUCCESS) {
		std::cout << "[ERROR] QueryDisplayConfig failed, code: " << queryResult << "\n";
		return results;
	}

	std::map<std::string, int> instanceCounter;

	for (auto & path : paths) {
		PortInfo info;
		info.isActive = (path.flags & DISPLAYCONFIG_PATH_ACTIVE) != 0;
		info.outputTech = techToString(path.targetInfo.outputTechnology);

		LUID luid = path.targetInfo.adapterId;
		UINT32 targetId = path.targetInfo.id;
		info.portKey = "LUID_" + std::to_string(luid.LowPart)
			+ "_TGT_" + std::to_string(targetId);

		std::string techKey = std::to_string(luid.LowPart) + "_" + info.outputTech;
		info.instanceIndex = ++instanceCounter[techKey];
		info.friendlyName = info.outputTech + " Port " + std::to_string(info.instanceIndex);


		UINT32 srcModeIdx = path.sourceInfo.modeInfoIdx;
		if (srcModeIdx != DISPLAYCONFIG_PATH_MODE_IDX_INVALID
			&& srcModeIdx < modeCount
			&& modes[srcModeIdx].infoType == DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE) {

			auto & srcMode = modes[srcModeIdx].sourceMode;
			info.desktopX = srcMode.position.x;
			info.desktopY = srcMode.position.y;
			info.width = srcMode.width;
			info.height = srcMode.height;
		}

		std::cout << "[PORT] " << info.friendlyName
				  << " | Key: " << info.portKey
				  << " | Pos: (" << info.desktopX << ", " << info.desktopY << ")"
				  << " | " << info.width << "x" << info.height
				  << " | Active: " << (info.isActive ? "YES" : "NO") << "\n";

		results.push_back(info);
	}

	return results;
}
