/*
 *  sabreServer.h
 *
 *  SABRe-server
 *
 *  Copyright © 2012-2014 Zurich University of the Arts. All Rights Reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *
 *  3. The name of the author may not be used to endorse or promote products
 *  derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY [LICENSOR] "AS IS" AND ANY EXPRESS OR IMPLIED
 *  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 *  EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 *  @author Jan Schacher
 *  @date 20130617
 *
 */

#pragma once

#include "ofMain.h"

#include "ofxOsc.h"
#include "ofxXmlSettings.h"
#include "threadedHID.h"

#define NUMOSCSENDERS 4


//--------------------------------------------------------
class sabreServer : public ofSimpleApp
{
	
public:
	void setup();
	void update();
	void draw();
	void exit();
	
	void startHID();
	void stopHID();
	void getHIDDeviceList();
    
	void startOSC();
	void stopOSC();

	void keyReleased(int key);

	void writePrefs();
	void dumpPrefs();
	bool readPrefs();
	void readMidicodes();

	void resetCalibrate();
	void resetSingleCalibrate(int i);
    void resetAirCalibrate();

	void writeScaling();

	void receiveOSC();
	
	void mouseMoved( int x, int y);
	void mouseDragged( int x, int y, int button);
	void mousePressed( int x, int y, int button);
	void mouseReleased();
	
	// class vars
	threadedHID	* rawHIDobject;

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
	string	status2[NUMOSCSENDERS];
    int     whichStatus;
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
    bool    sendOscToSC;
};