// =============================================================================
//
// Video Grabber add-on: Copyright (c) 2009-2016 Christopher Baker <http://christopherbaker.net>
//
// =============================================================================


#pragma once


#include "ofMain.h"
#include "IPVideoGrabber.h"
#include "ofxCv.h"
#include "ofxFaceTracker.h"
#include "ofxOsc.h"

//Defaults for OSC:
#define HOST "localhost"
#define PORT 6448
#define MESSAGE "/wek/inputs"




class IPCameraDef
{
public:
    IPCameraDef()
    {
    }
    
    IPCameraDef(const std::string& url): _url(url)
    {
    }
    
    IPCameraDef(const std::string& name,
                const std::string& url,
                const std::string& username,
                const std::string& password):
    _name(name),
    _url(url),
    _username(username),
    _password(password)
    {
    }
    
    void setName(const std::string& name) { _name = name; }
    std::string getName() const { return _name; }
    
    void setURL(const std::string& url) { _url = url; }
    std::string getURL() const { return _url; }
    
    void setUsername(const std::string& username) { _username = username; }
    std::string getUsername() const { return _username; }
    
    void setPassword(const std::string& password) { _password = password; }
    std::string getPassword() const { return _password; }
    
    
private:
    std::string _name;
    std::string _url;
    std::string _username;
    std::string _password;
};


using namespace ofx;


class ofApp: public ofBaseApp
{
public:
    void setup();
    void update();
    void draw();
    void keyPressed(int key);
    string ip = "YOUR_IP_ADDRESS";
    std::shared_ptr<Video::IPVideoGrabber> grabber;
    IPCameraDef cam;
    int w = ofGetWidth();
    int h = ofGetHeight();
    ofRectangle rect;
    bool zoomedIn = false;
    bool same = true;
    int startTimeZoomIn = 0;
    int currentTimeZoomIn = 0;
    int differenceZoomIn = 0;
    int startTimeZoomOut = 0;
    int currentTimeZoomOut = 0;
    int differenceZoomOut = 0;
    ofPixels pix;
    ofPixels pixFinal;
    ofPixels pixelAverage;
    ofImage img;
    ofFbo fbo;
    int eyeDistanceConstant = 100;
    ofFloatPixels pixelSum;
    int numberOfFaces = 0;


    ofxFaceTracker tracker;
    //OSC sender:
    ofxOscSender sender;
    
};
