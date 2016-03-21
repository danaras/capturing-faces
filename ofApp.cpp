// =============================================================================
//
// IP camera video grabber: Copyright (c) 2009-2016 Christopher Baker <http://christopherbaker.net>
//
// =============================================================================


#include "ofApp.h"
using namespace ofxCv;


void ofApp::setup()
{
    ofSetLogLevel(OF_LOG_VERBOSE);
    ofSetFrameRate(30);
    loadCameras();
    
    // initialize connection
    for (std::size_t i = 0; i < NUM_CAMERAS; i++)
    {
        IPCameraDef& cam = getNextCamera();

        std::shared_ptr<Video::IPVideoGrabber> c = std::make_shared<Video::IPVideoGrabber>();

        // if your camera uses standard web-based authentication, use this
        // c->setUsername(cam.username);
        // c->setPassword(cam.password);
        
        // if your camera uses cookies for authentication, use something like this:
        // c->setCookie("user", cam.username);
        // c->setCookie("password", cam.password);
        
        c->setCameraName(cam.getName());
        c->setURI(cam.getURL());
        c->connect(); // connect immediately

        // if desired, set up a video resize listener
        ofAddListener(c->videoResized, this, &ofApp::videoResized);
        
        grabbers.push_back(c);

    }
    tracker.setup();
    tracker.setTolerance(0.1);
}


IPCameraDef& ofApp::getNextCamera()
{
    nextCamera = (nextCamera + 1) % ipcams.size();
    return ipcams[nextCamera];
}


void ofApp::loadCameras()
{
    
    // all of these cameras were found using this google query
    // http://www.google.com/search?q=inurl%3A%22axis-cgi%2Fmjpg%22
    // some of the cameras below may no longer be valid.
    
    // to define a camera with a username / password
    //ipcams.push_back(IPCameraDef("http://148.61.142.228/axis-cgi/mjpg/video.cgi", "username", "password"));

    ofLog(OF_LOG_NOTICE, "---------------Loading Streams---------------");
    IPCameraDef def("http://ip/axis-cgi/mjpg/video.cgi");


            std::string logMessage = "STREAM LOADED: " + def.getName() +
			" url: " +  def.getURL() +
			" username: " + def.getUsername() +
			" password: " + def.getPassword();
            
            ofLogNotice() << logMessage;
            
            ipcams.push_back(def);
  
    ofLog(OF_LOG_NOTICE, "-----------Loading Streams Complete----------");
    
    nextCamera = ipcams.size();
}


void ofApp::videoResized(const void* sender, ofResizeEventArgs& arg)
{
    // find the camera that sent the resize event changed
    for (std::size_t i = 0; i < NUM_CAMERAS; ++i)
    {
        if (sender == &grabbers[i])
        {
            std::stringstream ss;
            ss << "videoResized: ";
            ss << "Camera connected to: " << grabbers[i]->getURI() + " ";
            ss << "New DIM = " << arg.width << "/" << arg.height;
            ofLogVerbose("ofApp") << ss.str();
        }
    }
}


void ofApp::update()
{
    // update the cameras
    for (std::size_t i = 0; i < grabbers.size(); ++i)
    {
        grabbers[i]->update();
        if(grabbers[i] -> isFrameNew()) {
            tracker.update(toCv(grabbers[i] -> getPixels()));//convert ofPixels into a cv:Mat
        }
    }
}


void ofApp::draw()
{
    ofBackground(0,0,0);

    ofSetHexColor(0xffffff);
    
    
    
    
    int w = ofGetWidth();
    int h = ofGetHeight();

    grabbers[0] -> draw(0,0,w,h);
    
    ofSetColor(255);
    ofSetLineWidth(2);
    tracker.draw();
    ofDrawBitmapString(ofToString(tracker.getDirection()), 10, 20);
    
    if(tracker.getFound()) {
       

        
        if(tracker.getFound() != same && tracker.getDirection() == 0){
        
        
        string fileName = pre + ofToString(counter) + ".png";
        //tracker.draw();
 //       ofLog() << "Size: " << tracker.size() << endl;
        ofRectangle rect = tracker.getHaarRectangle();
        //img.grabScreen(rect.getMinX(),rect.getMinY(),rect.getWidth(),rect.getHeight());
        img.setFromPixels(grabbers[0] -> getPixels(), w, h, OF_IMAGE_COLOR);
        img.crop(rect.getMinX(),rect.getMinY(),rect.getWidth(),rect.getHeight());
        img.save(fileName);
        counter++;
            
        same = !same;
        
       

        }
    }else{
        if(tracker.getFound() != same){
            same = !same;
        }
    }
   // ofLog() << "Height:" <<  grabbers[0]->getHeight() << ", Width:" << grabbers[0]->getWidth() << endl;


}
void ofApp::keyPressed(int key) {
    if(key == 'r') {
        tracker.reset();
    }
}

//void zoom(){
//    ofHttpResponse resp = ofLoadURL("http://ip/axis-cgi/com/ptz.cgi?areazoom=" + 100,100,800);
//    cout << resp.data << endl;
//    
//    //http://<servername>/axiscgi/com/ptz.cgi?<parameter>=<value>[&<parameter>=<value>...]
//}


