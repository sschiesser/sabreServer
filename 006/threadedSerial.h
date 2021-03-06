/*
 *  threadedSerial.h
 *  oscSerialSender
 *
 *  Created by jasch on 9/11/11.
 *  Copyright 2011 j45ch. All rights reserved.
 *
 */


#ifndef _THREADED_OBJECT
#define _THREADED_OBJECT

#include "ofMain.h"
#include "ofxThread.h"
#include "ofxOsc.h"
#include "ofxXmlSettings.h"

#include "sabreKeys.h"
#include "sabreMidiNote.h"

#define MAXNUM 64 // maximum number of input fields from serial stream
#define PATTERNLENGTH 32
#define FILTER_CHANGE

class threadedSerial : public ofxThread
{
	
public:
	
	threadedSerial();
	~threadedSerial();
	
	void start();
	void stop();
	void threadedFunction(); 
	
	void readSerial();
	void serialparse(unsigned char *c);
	unsigned char calcChecksum(int which, int length);
	void parseLeft();
	void parseRight();
	void parseIMU();
	void parseAir();
	void calcKeycode();
	void calcHeadingTilt();
	void sendOSC();
	void draw();

	ofSerial	serial;
	
	ofxXmlSettings	XML;
	string str1;
	
	ofTrueTypeFont TTF;
	ofxOscSender sender;
	
	vector <ofSerialDeviceInfo> deviceList;

	string		serialport;
	int			baudrate;
	string		sendIP;
	int			sendport;		
	int			framerate;
	int			receiveport;
	
	char		bytesRead[3];				// data from serial, we will be trying to read 3
	char		bytesReadString[4];			// a string needs a null terminator, so we need 3 + 1 bytes
	int			nBytesRead;					// how much did we read?
	int			nTimesRead;					// how many times did we read?
	float		readTime;					// when did we last read?		
	
	unsigned char seen[PATTERNLENGTH];
	unsigned char want[PATTERNLENGTH];
	
	unsigned char result[4][MAXNUM];		// for the four different types of packets
	bool		haveResult[4];				// flags to signal a full packet was parsed successfully
	
	sabreKeys	keys[32];
	sabreMidiNote midiNote[128];
	
	int			numKeyAddr;
	int			numImuAddr;
	int			numButtonAddr;
	int			numAirAddr;
	
	int			raw[10];
	long		rawIMU[10];
	double		IMU[10];
	double		summedIMU[3];
	long		airLong[2];
	unsigned long airULong[2];
	double		air[2];
	bool		button[3];
	bool		buttonOld[3];
	bool		buttonChanged[3];
	long		timestamp;
	double		heading;
	double		tilt;
	double		headingOld_x;
	double		headingOld_y;
  	double		headingLowpassFactor;
	
	int			keymin[MAXNUM];
	int			keymax[MAXNUM];
	bool		keyInverted[MAXNUM];
	
	double		scale10;	
	double		scale11;
	double		scale12;
	double		scale13;
	double		scale16;
	double		scale32;
	
	double		threshDown;
	double		threshUp;
	
	int			accelResolution;
	long		accelOffset;
	double		accelScale;
	
	int			gyroResolution;
	long		gyroOffset;
	double		gyroScale;
	
	int			magnetoResolution;
	long		magnetoOffset;
	double		magnetoScale;
	
	long		tempOffset;
	double		tempScale;
	
	bool		calibrateAll;
	bool		calibrate[MAXNUM];
	double		lowThresh;
	double		highThresh;
	long		debounceTimeOut;
	
	bool		status;
	int			displaySpeed;
	int			display;
	bool		windowChanged;
	
	// OSC sender stuff
	string		imuaddresses[12];
	string		buttonaddresses[3];
	string		airaddresses[2];
	string		systemaddress;
	string		keycodeaddress;
	string		midinoteaddress;
	string		headingaddress;
	
	long		keycode;
	long		keycodeOld;
	bool		keycodeChanged;
	int			midinote;
	bool		validMidiNote;
	
	string		address[64];
	ofxOscMessage m[MAXNUM];
	bool		senderStatus;
	
	long		systime;
	long		oldSystime;
	long		systemTimestamp;
	
};

#endif
