#include "ofMain.h"
#include "MonitorPortUtility.h"
#include "ofApp_ControlWindow.h"
#include "ofApp1.h"
#include "ofApp2.h"
#include "SharedSliceState.h"

int main(){

	bool discoveryMode = false;
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
		return 0;
	}

	auto sharedState = std::make_shared<SharedSliceState>();
	sharedState->setupSlices(2);

	// common settings for all windows
	ofGLFWWindowSettings settings;
	settings.setGLVersion(3, 2);

	// control window settings
	settings.setSize(1280, 720);
	settings.windowMode = OF_WINDOW;
	settings.decorated = true;
	settings.setPosition(ofVec2f(50, 50));
	auto controlWindow = ofCreateWindow(settings);

	// window 1 settings
	settings.windowMode = OF_FULLSCREEN;
	settings.decorated = false;
	settings.setPosition(ofVec2f(50, 100));
	settings.monitor = 0;
	settings.shareContextWith = controlWindow;
	auto window1 = ofCreateWindow(settings);

	// window 2 settings
	settings.setPosition(ofVec2f(2000, 200));
	settings.monitor = 1;
	settings.shareContextWith = controlWindow;
	auto window2 = ofCreateWindow(settings);

	ofRunApp(controlWindow, std::make_shared<ofApp_ControlWindow>(sharedState));
	ofRunApp(window1, std::make_shared<ofApp1>(sharedState, 0));
	ofRunApp(window2, std::make_shared<ofApp2>(sharedState, 1));

	ofRunMainLoop();

}
