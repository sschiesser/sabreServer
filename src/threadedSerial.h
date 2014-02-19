/*
 *  threadedSerial.h
 * 
 *  SABRe-server
 *
 *  Copyright © 2012/2013 Zurich University of the Arts. All Rights Reserved.
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
 *  @date 20121030
 *
 */

#ifndef _THREADED_OBJECT
#define _THREADED_OBJECT

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxOsc.h"


#include "sabreKeys.h"
#include "sabreMidiNote.h"
#include "sabreAir.h"
//#include "OSCsender.h"

#define DATAFRAMELENGTH 105
#define MAXNUM 64 // maximum number messages to allocate
#define PATTERNLENGTH_1 23 // maximumnumber of bytes in a left hand message
#define PATTERNLENGTH_2 42 // maximumnumber of bytes in a right hand message
#define PATTERNLENGTH_3 15 // maximumnumber of bytes in a air-mems message
#define FILTER_CHANGE // comment out in order to build without the redundancy check
#define NUMOSCSENDERS 4
#define CALIBRATEOFFSET 15 // value to add/remove to calibrated max/min to avoid key flattering at rest

class threadedSerial : public ofThread
{
	
public:
	
	threadedSerial();
	~threadedSerial();
	
	void start();
	void stop();
	void threadedFunction(); 
	
	void readSerial();
	void serialparse(unsigned char *c);
//	unsigned char calcChecksum(int which, int length);
	void parseLeft();
	void parseRight();
	void parseIMU();
	void parseAir();
	void calcKeycode();
	void calcHeadingTilt();
    
	void sendOSC(int ID, bool resetFlags);
    
	void calcResetID();

	ofSerial	serial;
	
	ofxXmlSettings	XML;
	string str1;
	
	ofTrueTypeFont TTF;
    
	ofxOscSender    sender[NUMOSCSENDERS];
    bool            senderActive[NUMOSCSENDERS];
    int             senderMode[NUMOSCSENDERS];
    string          sendIP[NUMOSCSENDERS];
	int             sendport[NUMOSCSENDERS];
    int             resetID;
    
    ofxOscMessage m[DATAFRAMELENGTH]; // static amount of messages in one dataframe

	vector <ofSerialDeviceInfo> deviceList;


    
	string		serialport;
	int			baudrate;
	
    // reading the serial buffer
	char		bytesRead[3];				// data from serial, we will be trying to read 3
	char		bytesReadString[4];			// a string needs a null terminator, so we need 3 + 1 bytes
	int			nBytesRead;					// how much did we read?
	int			nTimesRead;					// how many times did we read?
	float		readTime;					// when did we last read?		
	 
	unsigned char serialStream[3][PATTERNLENGTH_2]; // the size of the largest expected packet 
	int         streamSize[3];
    
	unsigned char input[3][PATTERNLENGTH_2];    // working buffer for each packet type
	bool		haveInput[3];				// flags to signal a full packet was parsed successfully
	bool        fullspeedOSC;               // flag for outputting OSC from serial or from OSC thread 0 = OSC thread 1 = serialThread
    bool        sendRawValues;
    
	sabreKeys	keys[32];
	sabreMidiNote midiNote[128];
    sabreAir    airValue;
    
//    bool        airCalibrationFlag;
//    int         aircalibrationCounter;
//    double      aircalibrationValue;
//    bool        calibrateAirPressureRange;

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

	double		heading;
	double		tilt;
	double		headingOld_x;
	double		headingOld_y;
  	double		headingLowpassFactor;
    
    long		timestampLeft;
	long		timestampRight;
	long		timestampAir;
    
    int         batteryLevelRight;
    int         batteryLevelAir;
    
    int         linkQualityLeft;
    int         linkQualityRight;
    int         linkQualityAir;
	
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
	
    
	bool		calibrateSwitch;
    bool        calibrateSingle;
	bool		calibrate[MAXNUM];
	double		lowThresh;
	double		highThresh;
	long		debounceTimeOut;
	
	bool		status;
	int			displaySpeed;
	int			display;
	bool		windowChanged;
	
    long		keycode;
	long		keycodeOld;
	bool		keycodeChanged;
	int			midinote;
	bool		validMidiNote;

	long		systimeL;
	long		systimeR;
	long		oldSystimeL;
	long		oldSystimeR;
    long		deltaTimeL;
    long		deltaTimeR;

	long		systemTimestamp;
    
    // OSC sender addresses
	string		imuaddresses[12];
	string		buttonaddresses[3];
	string		airaddresses[2];
    string      timestampAddressServer;
	string		timestampAddressLeft;
	string		timestampAddressRight;
	string		timestampAddressAir;
	string		keycodeaddress;
	string		midinoteaddress;
	string		headingaddress;
    string      batteryAddressMain;
    string      batteryAddressAir;
    string      linkQualityAddressLeft;
    string      linkQualityAddressRight;
    string      linkQualityAddressAir;
    
    bool        drawValues;

    int         OSCsendingInterval;
    long        OSCtime;
    long        OSCprevTime;
    int         OSCcounter;
    int         numOSCloops;
    
    bool        sendFullFrame;
    
    // debug variables
    bool dLhParsing, dRhParsing, dAmParsing;
    unsigned long long dParserStartT, dParserStopT, dParserSum;
    unsigned long long dLhStartT, dLhStopT;
    unsigned long long dRhStartT, dRhStopT;
    unsigned long long dAmStartT, dAmStopT;
    ofBuffer dBuffer;
};

#endif
