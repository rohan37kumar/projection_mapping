#include "MonitorPortUtility.h"
#include "SharedSliceState.h"
#include "ofApp1.h"
#include "ofApp_ControlWindow.h"
#include "ofMain.h"

int main() {


	auto sharedState = std::make_shared<SharedSliceState>();
	sharedState->discoveredPorts = MonitorPortUtility::enumeratePorts();


	if (!sharedState->loadConfig("projector_config.json")) {
		sharedState->setupSlices(2);
	} else {
		sharedState->outputWindows.resize(sharedState->getSliceCount());
	}

	int numSlices = sharedState->getSliceCount();

	ofGLFWWindowSettings settings;
	settings.setGLVersion(3, 2);
	settings.windowMode = OF_WINDOW;

	// control window
	settings.setSize(1280, 720);
	settings.decorated = true;
	settings.setPosition(ofVec2f(50, 50));
	auto controlWindow = ofCreateWindow(settings);

	// Output windows (one per slice)
	//settings.decorated = false;
	settings.shareContextWith = controlWindow;

	std::vector<std::shared_ptr<ofAppBaseWindow>> outputWindowHandles;

	for (int i = 0; i < numSlices; ++i) {

		settings.setPosition(ofVec2f(1000 + i * 100, 0));
		settings.setSize(500, 500);


		const std::string & key = (i < (int)sharedState->sliceToPortKey.size())
			? sharedState->sliceToPortKey[i]
			: "";
		const PortInfo * port = key.empty() ? nullptr : sharedState->findPort(key);
		if (port) {
			settings.setPosition(ofVec2f(port->desktopX, port->desktopY));
			settings.setSize(port->width, port->height);
		}

		auto win = ofCreateWindow(settings);
		outputWindowHandles.push_back(win);
		ofRunApp(win, std::make_shared<ofApp1>(sharedState, i));
	}

	ofRunApp(controlWindow, std::make_shared<ofApp_ControlWindow>(sharedState));

	ofRunMainLoop();
}
