#include "ofApp.h"
using namespace ofxCv;


void ofApp::setup()
{
    sender.setup(HOST, PORT);
    ofSetLogLevel(OF_LOG_VERBOSE);
    ofSetFrameRate(30);
    ofLog(OF_LOG_NOTICE, "---------------Loading Stream---------------");
    IPCameraDef cam("http://" + ip + "/axis-cgi/mjpg/video.cgi");
    std::shared_ptr<Video::IPVideoGrabber> c = std::make_shared<Video::IPVideoGrabber>();
    c->setCameraName(cam.getName());
    c->setURI(cam.getURL());
    c->connect(); // connect immediately
    grabber = c;
    tracker.setup();
    tracker.setTolerance(0.1);
    fbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGB);
    pixelSum.allocate(400, 500, OF_IMAGE_COLOR);
    pixelAverage.allocate(400, 500, OF_IMAGE_COLOR);
    faceVec.resize(60000);

}
void ofApp::update()
{
    // update the cameras
    
    grabber->update();
    if(grabber -> isFrameNew()) {
        tracker.update(toCv(grabber -> getPixels()));//convert ofPixels into a cv:Mat
    }
    
}
void ofApp::draw()
{
    ofBackground(0,0,0);
    
    ofSetHexColor(0xffffff);
    grabber -> draw(0,0,w,h);
    ofSetColor(255);
    ofSetLineWidth(2);
    tracker.draw();
    ofDrawBitmapString(ofToString(tracker.getDirection()), 10, 20);
     ofDrawBitmapString(ofToString(same), 20, 20);
    if(tracker.getFound()){
        rect = tracker.getHaarRectangle();
        int x = rect.getCenter().x;
        int y = rect.getCenter().y;
        int z = 50*ofGetHeight()/rect.getHeight();
        //osc
        ofxOscMessage m;
        m.setAddress(string(MESSAGE));
        m.addFloatArg((float)x);
        m.addFloatArg((float)y);
        sender.sendMessage(m, false);
        currentTimeZoomOut = ofGetElapsedTimeMillis();
        differenceZoomOut = currentTimeZoomOut - startTimeZoomOut;
        if(zoomedIn == false && same == false && differenceZoomOut > 4000){
            startTimeZoomIn = ofGetElapsedTimeMillis();
            ofHttpResponse zoomIn = ofLoadURL("http://" + ip + "/axis-cgi/com/ptz.cgi?areazoom=" + ofToString(x) +","+ ofToString(y) + "," + ofToString(z));
            cout << zoomIn.data << endl;
            zoomedIn = true;
        }
    }
    same = true;
    if(tracker.getFound() == false){
        same = false;
    }
    if(zoomedIn == true){
        currentTimeZoomIn = ofGetElapsedTimeMillis();
        differenceZoomIn = currentTimeZoomIn - startTimeZoomIn;
        if(differenceZoomIn >8000){
            tracker.update(toCv(grabber -> getPixels()));
            if(tracker.getFound()){
                ofLog() << "here" << endl;
                //cropping face according to the eyes (rotation included)
                ofVec2f leftEye = tracker.getImageFeature(ofxFaceTracker::LEFT_EYE).getCentroid2D();
                ofVec2f rightEye = tracker.getImageFeature(ofxFaceTracker::RIGHT_EYE).getCentroid2D();
                
                float rotation = (rightEye - leftEye).angle(ofVec2f(1, 0));
                
                float eyeDistance = leftEye.distance(rightEye);
                float scale = eyeDistanceConstant / eyeDistance;

                fbo.begin();
                ofClear(255,255);
                ofPushMatrix();

                ofTranslate(leftEye.x,leftEye.y);
              
                ofRotate(rotation);

                ofTranslate(-leftEye.x,-leftEye.y);

                img.setFromPixels(grabber -> getPixels(), w, h, OF_IMAGE_COLOR);

                img.setAnchorPoint(leftEye.x, leftEye.y);
                


                //img.crop(rect.getMinX(),rect.getMinY(),rect.getWidth(),rect.getHeight());
                ofTranslate(ofGetWidth()/2,ofGetHeight()/2);
                ofScale(scale, scale);
                
              
                img.draw(0,0);

                ofDrawCircle(leftEye.x, leftEye.y, 10);
                
                ofPopMatrix();
                ofDrawLine(ofGetWidth()/2, 0, ofGetWidth()/2, ofGetHeight());
                ofDrawLine(0, ofGetHeight()/2, ofGetWidth(), ofGetHeight()/2);
              
                
                fbo.end();
                fbo.readToPixels(pix);

                
                pix.crop(ofGetWidth()/2 - 2 * eyeDistanceConstant, ofGetHeight()/2 - 2 * eyeDistanceConstant, 4 * eyeDistanceConstant, 5 * eyeDistanceConstant);
                
                ofColor c = ofColor(pix[0], pix[1],pix[2]);
                
                cout << int(c.r) << endl;
                cout << int(c.g) << endl;
                cout << int(c.b) << endl;

                numberOfFaces += 1;
                
                int i = 0;
                for(int y = 0; y < 5 * eyeDistanceConstant; y++){
                    for(int x = 0; x < 4 * eyeDistanceConstant; x++){
                        pixelSum[i] += int(pix[i]);
                        pixelSum[i + 1] += int(pix[i + 1]);
                        pixelSum[i + 2] += int(pix[i + 2]);
                        
                        pixelAverage[i] = int(pixelSum[i]/numberOfFaces);
                        pixelAverage[i + 1] = int(pixelSum[i + 1]/numberOfFaces);
                        pixelAverage[i + 2] = int(pixelSum[i + 2]/numberOfFaces);
                        
                        i += 3;

                    }
                }
                
                pixelAverage.setImageType(OF_IMAGE_COLOR);
                
                cv::imshow("Display frame", toCv(pixelAverage));
                
                time_t t = time(0);   // get time now
                struct tm * now = localtime( & t );
                string fileName =  "face_" + ofToString(now->tm_year + 1900) + "-" + ofToString(now->tm_mon + 1) + "-" + ofToString(now->tm_mday) + "_" + ofToString(now->tm_hour) + "-" + ofToString(now->tm_min) + "-" +
                ofToString(now->tm_sec) + ".png";
                
                ofSaveImage(pixelAverage,"alligned/"+fileName);

            }
            ofHttpResponse zoomOut = ofLoadURL("http://" + ip + "/axis-cgi/com/ptz.cgi?move=home");
            cout << zoomOut.data << endl;
            zoomedIn = false;
            startTimeZoomOut = ofGetElapsedTimeMillis();
        }
    }
}

void ofApp::keyPressed(int key) {
    if(key == 'r') {
        tracker.reset();
    }
    if(key == 'h'){
        ofHttpResponse moveHome = ofLoadURL("http://" + ip + "/axis-cgi/com/ptz.cgi?move=home");
        cout << moveHome.data << endl;
        
    }
}