/*
 *  threadedOSC.h
 * 
 *  SABRe-server
 *  © 2012 ICST / ZHdK  
 *
 *  @author Jan Schacher
 *  @date 20130617
 *
 */

#ifndef _THREADED_OSC_OBJECT
#define _THREADED_OSC_OBJECT

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxXmlSettings.h"

#include "threadedSerial.h"


#define MAXNUM 64

class threadedOSC : public ofThread
{
	
public:
	
	threadedOSC();
	~threadedOSC();
	
	void start();
	void stop();
	void threadedFunction(); 

	void sendOSC();
    
    threadedSerial	* serialObject;
    
	ofxXmlSettings	XML;
	string str1;
	
	ofTrueTypeFont TTF;
	ofxOscSender sender;
	
    string		sendIP;
	int			sendport;		
	int			framerate;
	int			receiveport;
	
	char		bytesRead[3];				// data from serial, we will be trying to read 3
	char		bytesReadString[4];			// a string needs a null terminator, so we need 3 + 1 bytes
	int			nBytesRead;					// how much did we read?
	int			nTimesRead;					// how many times did we read?
	float		readTime;					// when did we last read?		
	
    unsigned char input[4][MAXNUM];         // for the four different types of packets
	bool		haveInput[4];				// flags to signal a full packet was parsed successfully
	bool        status;
	
	int			numKeyAddr;
	int			numImuAddr;
	int			numButtonAddr;
	int			numAirAddr;
	
	// OSC sender stuff
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
	
	long		keycode;
	long		keycodeOld;
	bool		keycodeChanged;
	int			midinote;
	bool		validMidiNote;
	
	string		address[64];
	ofxOscMessage m[MAXNUM];
	bool		senderStatus;
	
	
};

#endif
