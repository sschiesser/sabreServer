/*
 *  threadedSerial.h
 * 
 *  SABRe-server
 *  © 2012 ICST / ZHdK  
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

#define MAXNUM 64 // maximum number messages to allocate
#define PATTERNLENGTH_1 23 // maximumnumber of bytes in a left hand message
#define PATTERNLENGTH_2 40 // maximumnumber of bytes in a right hand message
#define PATTERNLENGTH_3 15 // maximumnumber of bytes in a air-mems message
#define FILTER_CHANGE // comment out in order to build without the redundancy check

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
    
	void sendOSC();
    
	void draw();

	ofSerial	serial;
	
	ofxXmlSettings	XML;
	string str1;
	
	ofTrueTypeFont TTF;
	ofxOscSender sender;
    ofxOscMessage m[MAXNUM];
	vector <ofSerialDeviceInfo> deviceList;

    string		sendIP;
	int			sendport;
    
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

	long		systime;
	long		oldSystime;
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
    
};

#endif
