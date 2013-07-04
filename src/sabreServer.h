/*
 *  testApp.h
 * 
 *  SABRe-server
 *  © 2012/2013 ICST / ZHdK  
 *
 *  @author Jan Schacher
 *  @date 20130617
 *
 */
 
#pragma once

#include "ofMain.h"

#include "ofxOsc.h"
#include "ofxXmlSettings.h"
#include "threadedSerial.h"
#include "threadedOSC.h"

//--------------------------------------------------------
class sabreServer : public ofSimpleApp
{
	
public:
	void setup();
	void update();
	void draw();
	void exit();
	
	void startSerial();
	void stopSerial();
	void getSerialDeviceList();
    
	void startOSC();
	void stopOSC();
    
    void startOSCfullspeed();
	void stopOSCfullspeed();
	
	void keyReleased(int key);	

	void writePrefs();
	void dumpPrefs();
	bool readPrefs();
	void readMidicodes();

	void resetCalibrate();

	void writeScaling();

	void receiveOSC();
	
	void mouseMoved( int x, int y);
	void mouseDragged( int x, int y, int button);
	void mousePressed( int x, int y, int button);
	void mouseReleased();
	
	// class vars
	threadedSerial	* serialThreadObject;
    threadedOSC	* OSCThreadObject;

    ofxOscReceiver receiver;
	int		receiveport;
	
//	ofSerial serial;
	
	ofxXmlSettings XML;
	ofxXmlSettings XMLmidi;
	string str1;
	
	ofTrueTypeFont TTF;
	ofTrueTypeFont TTFsmall;
	ofTexture texScreen;
	
	int		framerate;
	bool	status;
	int		display;
	bool	windowChanged;
	
	
	bool	prefStatus;
	string	status1;
	string	status2;
	string	status3;
	string	tempStr[1024];
	
	string	titleString;
	
	bool	senderStatus;
	bool	drawValues;

	float	timeOut;
	float	lastTime;
	
	bool	runOnce;
	float	runOnceDelay;
	float	runOnceStart;
	
	bool	redrawFlag;
	float	redrawInterval;
	float	lastRedraw;
	float   redrawValues[6];
	bool	firstflag;
	bool	drawTex;
	
	bool	menuState;
	int		numMenuItems;
	ofPoint menuClickPos;
};