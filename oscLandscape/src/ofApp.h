#pragma once

#include "ofMain.h"
#include "ofxOsc.h"

class ofApp : public ofBaseApp{
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);

    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseDragged(int x, int y, int button);
    void mouseMoved(int x, int y);

    //void raiseGrid(float val, int radius);
    //void lowerGrid(float val, int radius);
    void terraformGrid(float val, int radius, bool updown);
    void checkAndChange(int index, float val, bool updown);
    void collectAndSendOsc(int index);
    void randomizeEverything();
    
    ofMesh mainMesh;
    ofEasyCam mainCam;

    int width, height, dist;
    bool drawWireFrame;
    float perlinRange, perlinHeight;
    float controlData[2500];
    float oscSum[8];
    float timeDivisor;

    int nearestIndex;
    int drawingRadius;

    bool leftClicked;
    bool rightClicked;

    ofxOscReceiver OSCReceiver;
    ofxOscSender OSCSender;

    ofImage bt1norm, bt1hov, bt1click;
    ofRectangle b1rect;
    bool b1hover, b1active;
    ofImage bt2norm, bt2hov, bt2click;
    ofRectangle b2rect;
    bool b2hover, b2active;
    ofImage bt3norm, bt3hov, bt3click;
    ofRectangle b3rect;
    bool b3hover, b3active;
    ofImage bt4norm, bt4hov, bt4click;
    ofRectangle b4rect;
    bool b4hover, b4active;

    string kernel1name;
    string kernel2name;
};
