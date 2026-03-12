#include "ofMain.h"
#include "ofApp1.h"
#include "ofApp2.h"

int main(){

	// common settings for all windows
	ofGLFWWindowSettings settings;
	settings.setGLVersion(3, 2);
	settings.setSize(800, 500);
	settings.windowMode = OF_WINDOW;
	//settings.decorated = false;

	// window 1 settings
	settings.setPosition(ofVec2f(0, 20));
	//settings.monitor = 0;
	auto window01 = ofCreateWindow(settings);

	// window 2 settings
	settings.setPosition(ofVec2f(100, 100));
	//settings.monitor = 1;
	settings.shareContextWith = window01;
	auto window02 = ofCreateWindow(settings);

	ofRunApp(window01, std::make_shared<ofApp1>());
	ofRunApp(window02, std::make_shared<ofApp2>());
	ofRunMainLoop();

}

