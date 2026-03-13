#include "ofApp_ControlWindow.h"

void ofApp_ControlWindow::setup() {
}

void ofApp_ControlWindow::exit() {
	
}

void ofApp_ControlWindow::update() {

}

void ofApp_ControlWindow::draw() {
	
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
}

void ofApp_ControlWindow::mousePressed(int x, int y, int button) {
}

void ofApp_ControlWindow::mouseReleased(int x, int y, int button) {
}

void ofApp_ControlWindow::mouseEntered(int x, int y) {
}

void ofApp_ControlWindow::mouseExited(int x, int y) {
}

void ofApp_ControlWindow::windowResized(int w, int h) {
}

void ofApp_ControlWindow::gotMessage(ofMessage msg) {
}

void ofApp_ControlWindow::dragEvent(ofDragInfo dragInfo) {
}


