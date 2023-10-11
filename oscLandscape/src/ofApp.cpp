// draw a landscape on a 3d plane, different z-values are summed towards
// a list of osc messages that are sent out, osc-value areas work kinda
// like overlapping faders
//
//
// dirk dullmaier, creative coding 2, h_da sommersemester 2023

#ifdef TARGET_OSX
//ofSystem();
#endif

#ifdef TARGET_WIN32
//string execString = ofToDataPath() + "cc2folder.exe"; for later?
//ofSystem(execString);
#endif

#include "ofApp.h"
// TODO mousewheel radiuschange
// TODO better evaluation
//--------------------------------------------------------------
void ofApp::setup() {

	mainMesh.setMode(OF_PRIMITIVE_TRIANGLES);

	drawingRadius = 5; // HAS TO BE odd number currently
	dist = (drawingRadius - 1) / 2;
	timeDivisor = 90.0;

	ofEnableDepthTest();
	// set the width and height for our mesh
	width = 50;
	height = 50;

	kernel1name = "Convolution Kernel 1";
	kernel2name = "Convolution Kernel 2";

	for (int i = 0; i < width * height; i++) {
		//controlData[i] = ofRandom(-0.2, 0.15);
		controlData[i] = 0;
	}

	for (int i = 0; i < 8; i++) {
		oscSum[i] = 0;
	}

	// osc sender to maxpatch
	// 77305 -> outgoing
	// 77306 <- incoming
	OSCSender.setup("127.0.0.1", 77305);
	OSCReceiver.setup(77306);

	drawWireFrame = true;
	// set the initial values to use for our perlinNoise
	// now this is gonna be controlled via OSC from the maxpatch
	perlinRange = 1.1;
	perlinHeight = 0.1;

	ofBackground(85); // set the window background to dark grey
	ofSetColor(199); // grid color bright grey

	// fixed camera position that is nice to look at
	glm::quat quat = glm::quat(0.902, 0.416, 0.103, -0.0477);
	mainCam.setGlobalOrientation(quat);
	glm::vec3 pos = glm::vec3(7.747, -40.194, 33.41);
	mainCam.setGlobalPosition(pos);
	// pitch = 57.333 dont need the pitch when using quaternion

	mainCam.disableMouseInput(); // want fixed perspective

	// here we make the points inside our mesh
	// add one vertex to the mesh for width and height value
	// we use these x and y values to set the x and y co-ordinates of the mesh
	// adding a z value of intially zero for 3d pos
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			mainMesh.addVertex(ofPoint(x - width / 2, y - height / 2, 0)); // mesh index = x + y * width
			mainMesh.addColor(ofFloatColor(0, 0, 50));
		}
	}

	for (int y = 0; y < height - 1; y++) {
		for (int x = 0; x < width - 1; x++) {
			// connect everything
			mainMesh.addIndex(x + y * width);
			mainMesh.addIndex((x + 1) + y * width);
			mainMesh.addIndex(x + (y + 1) * width);
			mainMesh.addIndex((x + 1) + y * width);
			mainMesh.addIndex((x + 1) + (y + 1) * width);
			mainMesh.addIndex(x + (y + 1) * width);
		}
	}

	//buttons:

	int buttonSize = 30;

	bt1norm.load("assets/button_1_normal.png");
	bt1hov.load("assets/button_1_hover.png");
	bt1click.load("assets/button_1_active.png");
	b1active = true;
	float bt1x = 10;
	float bt1y = 10;
	float bt1width = buttonSize;
	float bt1height = buttonSize;
	b1rect.set(bt1x, bt1y, bt1width, bt1height);

	bt2norm.load("assets/button_2_normal.png");
	bt2hov.load("assets/button_2_hover.png");
	bt2click.load("assets/button_2_active.png");
	b2active = true;
	float bt2x = 2 * 10 + buttonSize;
	float bt2y = 10;
	float bt2width = buttonSize;
	float bt2height = buttonSize;
	b2rect.set(bt2x, bt2y, bt2width, bt2height);

	bt3norm.load("assets/button_3_normal.png");
	bt3hov.load("assets/button_3_hover.png");
	bt3click.load("assets/button_3_active.png");
	b3active = true;
	float bt3x = 10;
	float bt3y = 2 * 10 + buttonSize;
	float bt3width = buttonSize;
	float bt3height = buttonSize;
	b3rect.set(bt3x, bt3y, bt3width, bt3height);

	bt4norm.load("assets/button_4_normal.png");
	bt4hov.load("assets/button_4_hover.png");
	bt4click.load("assets/button_4_active.png");
	b4active = true;
	float bt4x = 2 * 10 + buttonSize;
	float bt4y = 2 * 10 + buttonSize;
	float bt4width = buttonSize;
	float bt4height = buttonSize;
	b4rect.set(bt4x, bt4y, bt4width, bt4height);

}

//--------------------------------------------------------------
void ofApp::update() {

	while (OSCReceiver.hasWaitingMessages()) {
		ofxOscMessage m;
		OSCReceiver.getNextMessage(m);

		string msgString;
		msgString = m.getAddress();

		if (msgString == "/perlin") {
			perlinHeight = m.getArgAsFloat(0);
		}

		if (msgString == "/kernel1") {
			kernel1name = m.getArgAsString(0);
		}

		if (msgString == "/kernel2") {
			kernel2name = m.getArgAsString(0);
		}
	}

	ofColor newColor;
	ofVec3f newPosition, oldPosition;

	float time = ofGetSystemTimeMillis() / timeDivisor; // time as parameter?
	int i = 0;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			newPosition = mainMesh.getVertex(i);
			// map x,y so we can see different ranges, then with height for amplitude
			newPosition.z = controlData[i] + ofNoise(ofMap(x + time, 0, width, 0, perlinRange), ofMap(y, 0, height, 0, perlinRange)) * perlinHeight;
			newColor.setHsb(150, 255 - ofMap(newPosition.z, 0, perlinHeight, 0, 255), 255);
			mainMesh.setVertex(i, newPosition); // update vertex pos
			mainMesh.setColor(i, newColor);
			i++;
		}
	}

	// want to be able to hold the mousebutton, not each click
	if (leftClicked) {
		terraformGrid(0.9, drawingRadius, 0);
		collectAndSendOsc(nearestIndex);
	}

	if (rightClicked) {
		terraformGrid(0.9, drawingRadius, 1);
		collectAndSendOsc(nearestIndex);
	}
}

// just a function to test some noisy values
void ofApp::randomizeEverything() {
	for (int i = 0; i < width * height; i++) {
		controlData[i] = ofRandom(-5.0, 10.0);
	}
}

// this function lowers or raises points in the grid
// needs to check if we are at the boundaries
void ofApp::terraformGrid(float val, int radius, bool updown) {
	int up = dist;
	while ((up * width) + (nearestIndex) > width * height) up--;
	int down = dist;
	while (nearestIndex - (down * width) < 0) down--;
	int left = dist;
	while ((nearestIndex % width) - left < 0) left--;
	int right = dist;
	while ((nearestIndex % width) + right > 49) right--;

	checkAndChange(nearestIndex,val,updown);
	for (int i = 0; i < up; i++) {
		checkAndChange(nearestIndex + ((i + 1) * width), val * 0.8, updown);
	}
	for (int i = 0; i < down; i++) {
		checkAndChange(nearestIndex - ((i + 1) * width), val * 0.8, updown);
	}
	for (int i = 0; i < left; i++) {
		checkAndChange(nearestIndex - (i + 1), val * 0.8, updown);
	}
	for (int i = 0; i < right; i++) {
		checkAndChange(nearestIndex + (i + 1), val * 0.8, updown);
	}

}

// keep the values inside certain levels
void ofApp::checkAndChange(int index, float val, bool updown) {
	if (updown == 0) {
		if (controlData[index] < 10) {
			controlData[index] += val;
		}
	}
	if (updown == 1) {
		if (controlData[index] > -5) {
			controlData[index] -= val;
		}
	}
}

// when this function is called it checks for the
// area in which changes are being made and then
// collects and sends the summed up values of those
// areas to the maxpatch
void ofApp::collectAndSendOsc(int index) {
	// oscSum map:
	// 0-> conv1
	// 1-> conv2
	// 2-> conv3
	// 3-> conv4
	// 4-> drywet (the well)
	// 5-> fadespeed
	// 6-> the river north, hipass
	// 7-> the river east, lopass

	if (index % width < 25 && (index / height) < 25) {
		//cout << "lower left quadrant ";
		//conv1 L -3125, 6250
		oscSum[0] = 0;
		ofxOscMessage m;
		m.clear();
		m.setAddress("/conv1");
		for (int i = 0; i < width / 2; i++) {
			for (int j = 0; j < height / 2; j++) {
				oscSum[0] += controlData[(j * height) + i];
			}
		}
		m.addFloatArg(oscSum[0]);
		OSCSender.sendMessage(m, false);
	}

	if (index % width > 25 && (index / height) < 25) {
		//cout << "lower right quadrant ";
		//conv2 R
		oscSum[1] = 0;
		ofxOscMessage m;
		m.clear();
		m.setAddress("/conv2");
		for (int i = 0; i < width / 2; i++) {
			for (int j = 0; j < height / 2; j++) {
				oscSum[1] += controlData[(j * height) + i + 25];
			}
		}
		m.addFloatArg(oscSum[1]);
		OSCSender.sendMessage(m, false);
	}

	if (index % width < 25 && (index / height) > 25) {
		//cout << "upper left quadrant ";
		//conv3 L
		oscSum[2] = 0;
		ofxOscMessage m;
		m.clear();
		m.setAddress("/conv3");
		for (int i = 0; i < width / 2; i++) {
			for (int j = 0; j < height / 2; j++) {
				oscSum[2] += controlData[((j + 25) * height) + i];
			}
		}
		m.addFloatArg(oscSum[2]);
		OSCSender.sendMessage(m, false);
	}

	if (index % width > 25 && (index / height) > 25) {
		//cout << "upper right quadrant ";
		//conv3 L
		oscSum[3] = 0;
		ofxOscMessage m;
		m.clear();
		m.setAddress("/conv4");
		for (int i = 0; i < width / 2; i++) {
			for (int j = 0; j < height / 2; j++) {
				oscSum[3] += controlData[((j + 25) * height) + i + 25];
			}
		}
		m.addFloatArg(oscSum[3]);
		OSCSender.sendMessage(m, false);
	}

	if (index % width > 20 && index % width < 30 && (index / height) > 20 && (index / height) < 30) {
		//the well = drywet regulation -500, 1000
		oscSum[4] = 0;
		ofxOscMessage m;
		m.clear();
		m.setAddress("/drywet");
		for (int i = 0; i < width / 5; i++) {
			for (int j = 0; j < height / 5; j++) {
				oscSum[4] += controlData[((j + 20) * height) + i + 20];
			}
		}
		m.addFloatArg(oscSum[4]);
		OSCSender.sendMessage(m, false);
	}

	if (index % width < 2 || (index / height) > 47 || index % width > 47) {
		//the mountains = speed of fade between convolutions -1520, 3040
		oscSum[5] = 0;
		ofxOscMessage m;
		m.clear();
		m.setAddress("/fadespeed");
		for (int i = 0; i < height; i++) {
			oscSum[5] += controlData[i * height];
			oscSum[5] += controlData[(i * height) + 1];
			oscSum[5] += controlData[i * height + 48];
			oscSum[5] += controlData[(i * height) + 49];
			oscSum[5] += controlData[(48 * height) + i];
			oscSum[5] += controlData[(49 * height) + i];
		}
		m.addFloatArg(oscSum[5]);
		OSCSender.sendMessage(m, false);
	}

	if (index % width > 22 && index % width < 27) {
		//river north, hipass -1000, 2000
		oscSum[6] = 0;
		ofxOscMessage m;
		m.clear();
		m.setAddress("/hipass");
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < 4; j++) {
				oscSum[6] += controlData[(i * height) + 23 + j];
			}
		}
		m.addFloatArg(oscSum[6]);
		OSCSender.sendMessage(m, false);
	}

	if (index / height > 22 && index / height < 27) {
		//river east, lopass -1000, 2000
		oscSum[7] = 0;
		ofxOscMessage m;
		m.clear();
		m.setAddress("/lopass");
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < 4; j++) {
				oscSum[7] += controlData[(j + 23 * height) + i];
			}
		}
		m.addFloatArg(oscSum[7]);
		OSCSender.sendMessage(m, false);
	}



}
//--------------------------------------------------------------
void ofApp::draw() {

	glPointSize(5);
	mainCam.begin();

	if (drawWireFrame) { // wires or pionts?
		mainMesh.drawWireframe();
	}
	else {
		mainMesh.drawVertices();
	}

	mainCam.end();

	// draw buttons
	if (b1hover) bt1hov.draw(b1rect);
	else if (b1active) bt1click.draw(b1rect);
	else bt1norm.draw(b1rect);

	if (b2hover) bt2hov.draw(b2rect);
	else if (b2active) bt2click.draw(b2rect);
	else bt2norm.draw(b2rect);

	if (b3hover) bt3hov.draw(b3rect);
	else if (b3active) bt3click.draw(b3rect);
	else bt3norm.draw(b3rect);

	if (b4hover) bt4hov.draw(b4rect);
	else if (b4active) bt4click.draw(b4rect);
	else bt4norm.draw(b4rect);

	// write the names of the samples currently used
	// for convolution
	ofDrawBitmapString(kernel1name, 90, 30);
	ofDrawBitmapString(kernel2name, 90, 70);

	// unless we are hovering over a button we want
	// to draw a line to the nearest point in our mesh
	if (!(b1hover || b2hover || b3hover || b4hover)) {
		int n = mainMesh.getNumVertices();
		float nearestDistance = 0;
		glm::vec2 nearestVertex;
		nearestIndex = 0;
		glm::vec3 mouse(mouseX, mouseY, 0);
		for (int i = 0; i < n; i++) {
			glm::vec3 cur = mainCam.worldToScreen(mainMesh.getVertex(i));
			float distance = glm::distance(cur, mouse);
			if (i == 0 || distance < nearestDistance) {
				nearestDistance = distance;
				nearestVertex = cur;
				nearestIndex = i;
			}
		}

		ofSetColor(ofColor::gray);
		ofDrawLine(nearestVertex, mouse);
		ofNoFill();
		ofSetLineWidth(1);
		glm::vec2 offset(10, -10);
	}
}

void ofApp::mouseMoved(int x, int y) {
	// check if mouse is over one of our buttons
	if (b1rect.inside(ofRectangle(x, y, 1, 1))) b1hover = true; else b1hover = false;
	if (b2rect.inside(ofRectangle(x, y, 1, 1))) b2hover = true; else b2hover = false;
	if (b3rect.inside(ofRectangle(x, y, 1, 1))) b3hover = true; else b3hover = false;
	if (b4rect.inside(ofRectangle(x, y, 1, 1))) b4hover = true; else b4hover = false;
}

void ofApp::mousePressed(int x, int y, int button) {
	// unless we are clicking a button we want to use
	// the mouse clicks for raising/lowering the grid
	if (!(b1hover || b2hover || b3hover || b4hover)) {
		if (button == 0) {
			leftClicked = true;
		}
		else if (button == 2) {
			rightClicked = true;
		}
	}

	// send trigger messages to change the current
	// sample being recorded into the buffers
	if (b1hover) {
		ofxOscMessage m;
		m.clear();
		m.setAddress("/prev1");
		m.addTriggerArg();
		OSCSender.sendMessage(m, false);
	}

	if (b2hover) {
		ofxOscMessage m;
		m.clear();
		m.setAddress("/next1");
		m.addTriggerArg();
		OSCSender.sendMessage(m, false);
	}

	if (b3hover) {
		ofxOscMessage m;
		m.clear();
		m.setAddress("/prev2");
		m.addTriggerArg();
		OSCSender.sendMessage(m, false);
	}

	if (b4hover) {
		ofxOscMessage m;
		m.clear();
		m.setAddress("/next2");
		m.addTriggerArg();
		OSCSender.sendMessage(m, false);
	}
}

// release the click
void ofApp::mouseReleased(int x, int y, int button) {
	if (button == 0) {
		leftClicked = false;
	}
	else if (button == 2) {
		rightClicked = false;
	}
}

// did i want to do something for dragging?
void ofApp::mouseDragged(int x, int y, int button) {
}

// toggles for fullscreen and wireframe
void ofApp::keyPressed(int key) {
	switch (key) {
	case 'f':
		ofToggleFullscreen();
		break;
	case 'w':
		drawWireFrame = !drawWireFrame;
		break;
	}
}

void ofApp::keyReleased(int key) {

}