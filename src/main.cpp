#include "ofMain.h"
#include "MonitorPortUtility.h"
#include "ofApp_ControlWindow.h"
#include "ofApp1.h"
#include "ofApp2.h"

int main(){

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
		return 0;
	}

	// common settings for all windows
	ofGLFWWindowSettings settings;
	settings.setGLVersion(3, 2);
	//settings.setSize(800, 500);
	settings.windowMode = OF_FULLSCREEN;
	settings.decorated = false;

	// window 1 settings
	settings.setPosition(ofVec2f(50, 100));
	settings.monitor = 0;
	auto window1 = ofCreateWindow(settings);

	// window 2 settings
	settings.setPosition(ofVec2f(2000, 200));
	settings.monitor = 1;
	settings.shareContextWith = window1;
	auto window2 = ofCreateWindow(settings);

	ofRunApp(window1, std::make_shared<ofApp1>());
	ofRunApp(window2, std::make_shared<ofApp2>());

	ofRunMainLoop();

}
