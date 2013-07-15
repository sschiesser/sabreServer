/*
 *  sabreServer.cpp
 * 
 *  SABRe-server
 *  © 2012/2013 ICST / ZHdK  
 *
 *  @author Jan Schacher
 *  @date 20120617
 *
 */

#include "sabreServer.h"

void sabreServer::setup()
{
	ofSetEscapeQuitsApp(false);
	ofEnableAlphaBlending();
	titleString = "sabreServer version 0.9a1";
	
	serialThreadObject = new(threadedSerial);
	OSCThreadObject = new(threadedOSC);
    
    OSCThreadObject->serialObject = serialThreadObject; // tell OSCThread about serialThread

	TTF.loadFont("lucidagrande.ttf", 8, 1, 1, 0);
	TTFsmall.loadFont("lucidagrande.ttf", 8, 1, 0, 0);
	serialThreadObject->TTF.loadFont("lucidagrande.ttf", 8, 1, 1, 0);
	
	texScreen.allocate(440, 700, GL_RGB);
	
	windowChanged = 1;
	drawValues = 0;
    serialThreadObject->drawValues = 0;
	menuState = 0;
	
	serialThreadObject->serialport = "/dev/tty.usbserial-A7005Ghs";
	serialThreadObject->baudrate = 230400;
	OSCThreadObject->sendIP  = serialThreadObject->sendIP = "127.0.0.1";
	OSCThreadObject->sendport = serialThreadObject->sendport = 40000;;
    
	receiveport = 40001;
	serialThreadObject->debounceTimeOut = 0;

	status = readPrefs();	
	if(status) {
		status3 = "Success reading settings in \"sabreServer.xml\"";		
	}else{
		status3 = "Failed reading opening file \"sabreServer.xml\" ";
	}
	readMidicodes();
	lastTime = ofGetElapsedTimef();
	
	receiver.setup( receiveport );
	
	framerate = 20;
	ofSetFrameRate( framerate ); // cap the glut callback rate
//	ofSetVerticalSync( true );
//	ofBackground( 224, 224, 224);
	ofBackground( 238, 238, 238);
	redrawFlag = 1;
	// redrawInterval = redrawValues[display]; // in seconds
	firstflag = 1;
	
	runOnce = 1;
	runOnceDelay = 2.0f;
	lastRedraw = runOnceStart = ofGetElapsedTimef();
	
 	getSerialDeviceList();
	
	startSerial();
    if(serialThreadObject->fullspeedOSC){
        startOSCfullspeed();
    }else{
        startOSC();
	}
	ofSetWindowPosition(0,44);

    dumpPrefs();
}


void sabreServer::update()
{
	float now = ofGetElapsedTimef();
	
//	if(runOnce == 1) {
//		if(now > (runOnceStart + runOnceDelay) )
//		{
//            startSerial();
//            startOSC();
//            runOnce = 0;
//		}
//	}
	receiveOSC();
}


void sabreServer::draw()
{
	int i;
	int anchorx = 15;
	int anchory = 66;
	int stepsize = 18;
	int columnwidth = 200;
	int rightColumn = 270;
	int leftColumn = 10;
	int width;
	int height;
	double yy;
    int pos_x;
	
	if(windowChanged == 1) {
		if(drawValues == 0) {
			width = 550;
			height = 52;
			ofSetWindowShape(width, height);
			windowChanged = 0;
		} else if(drawValues == 1) {
			width = 746;
			height = 514; // 790
			timeOut = 5.0;
			ofSetWindowShape(width, height);
			windowChanged = 0;
		}
	}
	
	if(redrawFlag == 1) // drawn once after first update
	{
		// header frame background
//		ofFill();        
//		ofSetColor(191, 191, 191, 255);
//        ofRect(0, 0, width, 49);

		
//		ofSetColor(255, 191, 191, 127);
//		ofRect(0, 0, width, height);
		
		if(serialThreadObject->status) {
			ofSetColor(0, 0, 0, 255);
		} else {
			ofSetColor(127, 127, 127, 255);
		}
		TTFsmall.drawString(status1, anchorx, 34);
		TTFsmall.drawString(status2, anchorx, 48);

		// separator lines
//		ofSetColor(240, 240, 240, 127);
//		ofLine(0, anchory-14, width, anchory-14);	
//		ofSetColor(127, 127, 127, 127);
//		ofLine(0, anchory-13, width, anchory-13);		
		
		ofSetColor(200, 200, 200, 255);
		
		// Menu
		ofFill();
		ofSetColor(232, 232, 232);
		ofRect(leftColumn, 3, 188, 18);
		ofNoFill();
		ofSetColor(127, 127, 127);
		ofRect(leftColumn, 3, 188, 18);
		ofFill();
		ofTriangle(leftColumn+177,7,leftColumn+185, 7, leftColumn+181, 13);
		ofSetColor(0, 0, 0);
		TTFsmall.drawString(serialThreadObject->serialport,leftColumn+5, 16);
		

		// start/stop button
		if(serialThreadObject->status) {
			ofFill();
			ofSetColor(255, 255, 255);
			ofRect(rightColumn, 3, 124, 18);
			ofNoFill();
			ofSetColor(127, 127, 127);
			ofRect(rightColumn, 3, 124, 18);
			ofSetColor(0, 0, 0);
			TTFsmall.drawString("Stop", rightColumn+48, 16);
		} else {
			ofFill();
			ofSetColor(232, 232, 232);
			ofRect(rightColumn, 3, 124, 18);
			ofNoFill();
			ofSetColor(127, 127, 127);
			ofRect(rightColumn, 3, 124, 18);
			ofSetColor(0, 0, 0);
			TTFsmall.drawString("Start", rightColumn+48, 16);
		}
		
		// show values button
        if( !drawValues ) {
            ofFill();
            ofSetColor(232, 232, 232);
            ofRect(rightColumn+126, 3, 124, 18);
            ofNoFill();
            ofSetColor(127, 127, 127);
            ofRect(rightColumn+126, 3, 124, 18);
            ofSetColor(0, 0, 0);		
            TTFsmall.drawString("Show Values", rightColumn+28+126, 16);
        }else{
            ofFill();
            ofSetColor(255, 255, 255);
            ofRect(rightColumn+126, 3, 124, 18);
            ofNoFill();
            ofSetColor(127, 127, 127);
            ofRect(rightColumn+126, 3, 124, 18);
            ofSetColor(0, 0, 0);
            TTF.drawString("Hide Values", rightColumn+32+126, 16);
		}
		// Calibrate Button
		ofFill();
		ofSetColor(232, 232, 232);
		ofRect(375, 480, 124, 20);
		ofNoFill();
		ofSetColor(127, 127, 127);
		ofRect(375, 480, 124, 20);
		ofSetColor(0, 0, 0);
		TTFsmall.drawString("Calibrate Keys", 375+20, 480+14);
        
        // Calibrate Button
		ofFill();
		ofSetColor(232, 232, 232);
		ofRect(502, 480, 124, 20);
		ofNoFill();
		ofSetColor(127, 127, 127);
		ofRect(502, 480, 124, 20);
		ofSetColor(0, 0, 0);
		TTFsmall.drawString("Calibrate Air", 500+24, 480+14);

		
		// value display left column
		for(i = 0; i < 25; i++) { // stripes
			if((i % 2) == 0){
				ofFill();
				ofSetColor(255, 255, 255, 255);
				ofRect(anchorx-2, anchory+((i-1) * stepsize)+7, 94, 16);
				ofSetColor(0, 0, 0, 255);
			}			
		}
		
        for(i = 0; i < 12; i++) { // stripes
			if((i % 3) == 0){
				ofFill();
				ofSetColor(255, 255, 255, 255);
				ofRect(anchorx-2 + 360, anchory+((i-1) * stepsize)+7, 140, 16);
				ofSetColor(0, 0, 0, 255);
			}
		}
		
		ofSetColor(0, 0, 0, 191);
		for(i = 0; i < 25; i++) { // key addresses
			TTFsmall.drawString(serialThreadObject->keys[i].oscaddress, anchorx, anchory+((i) * stepsize) );
		}
		for(i = 0; i < 9; i++) { // imu addresses
			std::string str = serialThreadObject->imuaddresses[i/3];
			std::string::size_type end = str.find_last_of('/');
			if(end != str.npos)
				str = str.substr(0, end);
			TTFsmall.drawString(str, anchorx + 360, anchory+((i) * stepsize) );
//			TTFsmall.drawString(str, anchorx + 360, anchory+((i+25) * stepsize) );
//			TTFsmall.drawString(serialThreadObject->imuaddresses[i/3], anchorx, anchory+5+((i+25) * stepsize) );
		}
        // air addresses
		TTFsmall.drawString(serialThreadObject->airaddresses[0], anchorx + 360, anchory+(10 * stepsize) );
        
		for(i = 0; i < 1; i++) { // first button address truncated
			char temp[64];
			strncpy(temp, serialThreadObject->buttonaddresses[0].c_str(), serialThreadObject->buttonaddresses[0].size()-2);
			temp[serialThreadObject->buttonaddresses[0].size()-2] = 0;
//			TTFsmall.drawString(temp, anchorx + 360, anchory+((i+35) * stepsize) );
			TTFsmall.drawString(temp, anchorx + 360, anchory+((i+9) * stepsize) );
		}
        // battery
        //        TTFsmall.drawString("battery level", anchorx, anchory+((i+35) * stepsize) );
//        TTFsmall.drawString("battery: ", 265, 38);
//        TTF.drawString( "main: "+ofToString((int)(serialThreadObject->batteryLevelRight*12.5))+"%", 380, 40 );
//        TTF.drawString( "mouthpiece: "+ofToString((int)(serialThreadObject->batteryLevelAir*12.5))+"%", 440, 40 );

		texScreen.loadScreenData(0,0, 440, 700);
		drawTex = 1;
	} else {
		if(drawTex) {
			texScreen.draw(0, 0, 440, 266);
			drawTex = 0;
		}
	}
    
#pragma mark draw levels
    
    TTFsmall.drawString( "battery: main       air", 280, 34 );

    // battery display
    for(i = 0; i < 16; i++){
        pos_x = 360;
        if(serialThreadObject->batteryLevelRight*12.5 >= (i * 6.25) ) {
            ofSetColor(127, 127, 127, 255);
            ofRect(pos_x+i*2, 25, 2, 10);
        }
    }
//    ofRect
    for(i = 0; i < 16; i++){
        pos_x = 425;
        if(serialThreadObject->batteryLevelAir*12.5 >= (i * 6.25) ) {
            ofSetColor(127, 127, 127, 255);
            ofRect(pos_x+i*2, 25, 2, 10);
        }
    }
    ofSetColor(191, 191, 191, 255);
    ofNoFill();
    ofRect(360, 25, 33, 10);
    ofRect(425, 25, 33, 10);
    ofRect(393, 27, 2, 6);
    ofRect(458, 27, 2, 6);

    ofFill();

    
    ofSetColor(0, 0, 0, 191); 
    TTFsmall.drawString( "wireless: left       right      air", 280, 48 );

    for(i = 0; i < 8; i++){
        pos_x = 360;
        if( (CLAMP(serialThreadObject->linkQualityLeft, 50, 200) - 50 ) >= (i * 18) ) {
            ofSetColor(127, 127, 127, 255);
        }else{
            ofSetColor(212, 212, 212, 255);
            
        }
        ofRect(pos_x+i*4, 36+(10-i), 2, 2+i);
    }
    for(i = 0; i < 8; i++){
        pos_x = 430;
        if( (CLAMP(serialThreadObject->linkQualityRight, 50, 200) - 50 ) >= (i * 18) ) {
            ofSetColor(127, 127, 127, 255);
        }else{
            ofSetColor(212, 212, 212, 255);
            
        }
        ofRect(pos_x+i*4, 36+(10-i), 2, 2+i);
    }
    for(i = 0; i < 8; i++){
        pos_x = 485;
        if( (CLAMP(serialThreadObject->linkQualityAir, 50, 200) - 50 ) >= (i * 18) ) {
            ofSetColor(127, 127, 127, 255);
        }else{
            ofSetColor(212, 212, 212, 255);
            
        }
        ofRect(pos_x+i*4, 36+(10-i), 2, 2+i);
    }

//    serialThreadObject->batteryLevelAir*6.25
//    serialThreadObject->batteryLevelRight*6.25
//     
//    serialThreadObject->linkQualityLeft*0.390625
//    serialThreadObject->linkQualityRight*0.390625
//    serialThreadObject->linkQualityAir*0.390625

    

    
	if(drawValues) {
		serialThreadObject->draw();
	}
	
	if(menuState) {
		numMenuItems = (int)serialThreadObject->deviceList.size() / 2;

		ofSetColor(232, 232, 232);
		ofFill();
		ofRect(leftColumn, 21, 188, numMenuItems*18);

		vector<ofSerialDeviceInfo>::iterator it;
		int i;
		for ( it = serialThreadObject->deviceList.begin(), i = 0 ; it < serialThreadObject->deviceList.end(); it++) {
			if(!strncmp(it->getDeviceName().c_str(), "tty", 3)){
				ofNoFill();
				ofSetColor(232, 232, 232);
				ofRect(leftColumn, 21+(i*18), 188, 18);
				ofSetColor(0, 0, 0);
				TTFsmall.drawString(it->getDeviceName(), leftColumn+4, 35+i*18 );

				i++;
			}
		}
		
		ofNoFill();
		ofSetColor(127, 127, 127);
		ofRect(leftColumn, 21, 188, numMenuItems*18);
	}
    
}

void sabreServer::exit()
{
	free(serialThreadObject);
	
	XML.pushTag("sabre", 0);
	XML.removeTag("display", 0);
	XML.setValue("display", drawValues, 0);
	XML.popTag();
	XML.saveFile("sabreServer.xml");
	return;
	// printf("exit\n");
}

void sabreServer::startSerial()
{
//	if(serialThreadObject->status) {
//		serialThreadObject->serial.close();
//		serialThreadObject->stop(); // the thread
//	}
	
	serialThreadObject->status = serialThreadObject->serial.setup(serialThreadObject->serialport, serialThreadObject->baudrate);
    
    // check if port is REALLY open, how?
    
	if(serialThreadObject->status) {
		status1 = "Device open ";//+serialThreadObject->serialport+" "+ofToString(serialThreadObject->baudrate);
// 		ofSetWindowTitle(serialThreadObject->serialport);
		ofSetWindowTitle(titleString+" - Connection OK");
		serialThreadObject->start(); // the serial thread
	} else {
		status1 = "Unable to open ";//+serialThreadObject->serialport;
		serialThreadObject->stop(); // the thread
		ofSetWindowTitle(titleString+" - No Connection");

		ofSystemAlertDialog("SABRe Server \n"+status1+serialThreadObject->serialport);
	}
	redrawFlag = 1;
}

void sabreServer::stopSerial()
{
	if(serialThreadObject->status) {
		serialThreadObject->serial.close();
		serialThreadObject->stop(); // the serial thread
		serialThreadObject->status = false;
	}
	redrawFlag = 1;
}


void sabreServer::startOSC()
{
	if(OSCThreadObject->status) {
        OSCThreadObject->stop(); // stops the thread, deletes the object which also closes the socket
	}
    // open an outgoing connection to sendIP:PORT
	OSCThreadObject->status = OSCThreadObject->oscSender.setup( serialThreadObject->sendIP.c_str(), serialThreadObject->sendport);
	if(OSCThreadObject->status) {
        OSCThreadObject->start(); // the OSC thread
		status2 = "Sending OSC to "+serialThreadObject->sendIP+" on port "+ofToString(serialThreadObject->sendport);
	} else {
        ofSystemAlertDialog("Sabre Server Unable to open Network "+serialThreadObject->sendIP+" on port "+ofToString(serialThreadObject->sendport));
		status2 = "Unable to open Network "+serialThreadObject->sendIP+" on port "+ofToString(serialThreadObject->sendport);
	}
	redrawFlag = 1;
}

void sabreServer::stopOSC()
{
	if(OSCThreadObject->status) {
        OSCThreadObject->stop(); // stops the thread, deletes the object which also closes the socket
		OSCThreadObject->status = false;
	}
	redrawFlag = 1;
}

void sabreServer::startOSCfullspeed()
{
	if(serialThreadObject->status) {
        serialThreadObject->stop(); // stops the thread, deletes the object which also closes the socket
	}
    
    // open an outgoing connection to sendIP:PORT
	serialThreadObject->status = serialThreadObject->sender.setup( serialThreadObject->sendIP.c_str(), serialThreadObject->sendport);
	if(serialThreadObject->status) {
        serialThreadObject->start(); // the OSC thread
		status2 = "Sending OSC to "+serialThreadObject->sendIP+" on port "+ofToString(serialThreadObject->sendport);
	} else {
        ofSystemAlertDialog("Sabre Server Unable to open Network "+serialThreadObject->sendIP+" on port "+ofToString(serialThreadObject->sendport));
		status2 = "Unable to open Network "+serialThreadObject->sendIP+" on port "+ofToString(serialThreadObject->sendport);
	}
	redrawFlag = 1;
}

void sabreServer::stopOSCfullspeed()
{
	if(serialThreadObject->status) {
        serialThreadObject->stop(); // stops the thread, deletes the object which also closes the socket
		serialThreadObject->status = false;
	}
	redrawFlag = 1;
}

void sabreServer::getSerialDeviceList()
{
	serialThreadObject->deviceList = serialThreadObject->serial.getDeviceList();
	vector<ofSerialDeviceInfo>::iterator it;
	for ( it = serialThreadObject->deviceList.begin() ; it < serialThreadObject->deviceList.end(); it++) {
		string devPath = it->getDevicePath();
		string devName = it->getDeviceName();
		if(!strncmp(devName.c_str(), "tty.", 4)){
			// printf("device %s %s\n", devPath.c_str(), devName.c_str());
		}
	}
}

void sabreServer::receiveOSC()
{
	int i;
	string temp;
	
	while( receiver.hasWaitingMessages()) {
		ofxOscMessage m;
		receiver.getNextMessage( &m );
		temp = m.getAddress();
		
		if ( !strcmp( temp.c_str(), "/sabre/framerate" )) {
//			framerate = m.getArgAsInt32( 0 );
//			status2 = "sending OSC to "+serialThreadObject->sendIP+" on port "+ofToString(serialThreadObject->sendport);
		}else if ( strcmp( temp.c_str(), "/sabre/display" ) == 0 ) {
			display = m.getArgAsInt32( 0 );
			windowChanged = 1;
		}else if ( strcmp( temp.c_str(), "/sabre/reset" ) == 0 ) {
		}else if ( strcmp( temp.c_str(), "/sabre/writePrefs" ) == 0 ) {
			sabreServer::writePrefs();
		}else if ( strcmp( temp.c_str(), "/sabre/readPrefs" ) == 0 ) {
			sabreServer::readPrefs();
		}else if ( strcmp( temp.c_str(), "/sabre/network/receiver/port" ) == 0 ) {
			receiveport = m.getArgAsInt32( 0 );
			receiver.setup( receiveport );
		}else if ( strcmp( temp.c_str(), "/sabre/network/sender/port" ) == 0 ) {
			serialThreadObject->sendport = m.getArgAsInt32( 0 );
            // TODO switch between fulspeed or OSc thread
//			serialThreadObject->sender.setup( serialThreadObject->sendIP, serialThreadObject->sendport );
//			status2 = "sending OSC to "+serialThreadObject->sendIP+" on port "+ofToString(serialThreadObject->sendport);
		}else if ( strcmp( temp.c_str(), "/sabre/network/sender/IP" ) == 0 ) {
			serialThreadObject->sendIP = m.getArgAsString(0);
            // TODO switch between fulspeed or OSc thread
//			serialThreadObject->sender.setup( m.getArgAsString(0), serialThreadObject->sendport );
//			status2 = "sending OSC to "+serialThreadObject->sendIP+" on port "+ofToString(serialThreadObject->sendport);
		}else if ( strcmp( temp.c_str(), "/sabre/exit" ) == 0 ) {
			sabreServer().exit();
		}else if ( strcmp( temp.c_str(), "/sabre/calibrateAll" ) == 0 ) {
			if(serialThreadObject->calibrateAll == 1) { // before we swith it off
				for(i = 0; i < MAXNUM; i++) {
					serialThreadObject->calibrate[i] = 0;
				}
				writeScaling(); // we write the values into the prefs
			}
			serialThreadObject->calibrateAll = !serialThreadObject->calibrateAll;
			for(i = 0; i < MAXNUM; i++) {
				serialThreadObject->calibrate[i] = !serialThreadObject->calibrate[i];
			}
			printf("calibrate is %d\n", serialThreadObject->calibrateAll);
		}else if ( strcmp( temp.c_str(), "/sabre/calibrate" ) == 0 ) {
			int which = m.getArgAsInt32( 0 );
			serialThreadObject->calibrate[which] = m.getArgAsInt32( 1 );
			if(serialThreadObject->calibrate[which] == 1) { // reset calibration values
				serialThreadObject->keys[which].minimum = 1023;
				serialThreadObject->keys[which].maximum = 0;
			}
			
			if(serialThreadObject->calibrate[which] == 0) { // before we switch it off
				writeScaling(); // we write the values into the prefs
			}
			printf("calibrate[%d] is %d\n", which, serialThreadObject->calibrate[which]);
		}else if ( strcmp( temp.c_str(), "/sabre/calibrate/air" ) == 0 ) {
			int which = m.getArgAsInt32( 0 );
			serialThreadObject->airValue.calibratePressureRange = m.getArgAsInt32( 1 );
			if(serialThreadObject->airValue.calibratePressureRange == 1) { // reset calibration values
				serialThreadObject->keys[which].minimum = 32768;
				serialThreadObject->keys[which].maximum = -32768;
			}
			
			if(serialThreadObject->airValue.calibratePressureRange == 0) { // before we switch it off
				writeScaling(); // we write the values into the prefs
			}
			printf("calibrateAir is %d\n", serialThreadObject->airValue.calibratePressureRange);
		}
	}
}

bool sabreServer::readPrefs()
{	
	int numTags;
	int numPtTags;
	int totalToRead;
	int i;
	int ID;
	int bitwidth;
	
	bool result = XML.loadFile("sabreServer.xml");
	if(result) {
		str1 = XML.getValue("sabre:serialport", "/dev/tty.usbserial-A6007WWR");
		serialThreadObject->serialport = str1;
		
		OSCThreadObject->sendIP = serialThreadObject->sendIP = XML.getValue("sabre:network:sender:IP", "127.0.0.1");
		OSCThreadObject->sendport = serialThreadObject->sendport = XML.getValue("sabre:network:sender:port", 40000);
                
		receiveport = XML.getValue("sabre:network:receiver:port", 40001);

		serialThreadObject->baudrate = XML.getValue("sabre:baudrate", 57600);
		framerate = XML.getValue("sabre:framerate", 20);
		drawValues = XML.getValue("sabre:display", 0);
        serialThreadObject->drawValues = drawValues = CLAMP(drawValues, 0, 1);
		
		serialThreadObject->threshDown = XML.getValue("sabre:thresholds:down", 0.2);
		serialThreadObject->threshUp = XML.getValue("sabre:thresholds:up", 0.8);
		serialThreadObject->debounceTimeOut = XML.getValue("sabre:debounce-timeout", 0);
        
        serialThreadObject->fullspeedOSC = XML.getValue("sabre:OSCsender:fullspeed", 0);
        OSCThreadObject->OSCInterval = XML.getValue("sabre:OSCsender:interval", 0);

		for(i = 0; i < MAXNUM; i++) {
			serialThreadObject->keys[i].threshDown = serialThreadObject->threshDown;
			serialThreadObject->keys[i].threshUp = serialThreadObject->threshUp;
		}
		
		serialThreadObject->accelResolution = XML.getValue("sabre:accel-resolution", 4);
		// 	4g/8g/16g 10/11/12/13bit resolution
		//  2g (10bit) : 512, 4g (11bit) : 1024, 8g (12bit) :  2048, 16g (13bit) : 4096
		if(serialThreadObject->accelResolution == 2){
			bitwidth = 10;
		} else if(serialThreadObject->accelResolution == 4){
			bitwidth = 11;
		} else if(serialThreadObject->accelResolution == 8){
			bitwidth = 12;
		} else if(serialThreadObject->accelResolution == 16){
			bitwidth = 13;			
		}
		
		double rangeVal = pow((double)2.0, (double)bitwidth);
		serialThreadObject->accelOffset = rangeVal * 0.5;
		serialThreadObject->accelScale = 1.0 / rangeVal;
				
		numTags = XML.getNumTags("sabre:key");
		if(numTags > 0) {
			XML.pushTag("sabre", numTags-1);
			serialThreadObject->numKeyAddr = numPtTags = XML.getNumTags("key");
			if(numPtTags > 0) {
				totalToRead = MIN(numPtTags, 64);
				for(i = 0; i < totalToRead; i++) {
					ID = XML.getAttribute("key", "id", 0, i);
					ID--;
					str1 = XML.getValue("key:oscaddress", "/sabre/key/0", i);
					if(str1.length() > 0) {
						serialThreadObject->keys[ID].oscaddress = str1;
					}
					serialThreadObject->keys[ID].inverted = XML.getValue("key:invert", 0, i);
					serialThreadObject->keys[ID].minimum = XML.getValue("key:minimum", 0, i);
					serialThreadObject->keys[ID].maximum = XML.getValue("key:maximum", 1023, i);
//					keys[ID].range = keys[ID].maximum - keys[ID].minimum;
					if(serialThreadObject->keys[ID].maximum != serialThreadObject->keys[ID].minimum) {
						serialThreadObject->keys[ID].scale = 1.0 / (serialThreadObject->keys[ID].maximum - serialThreadObject->keys[ID].minimum);
					}else{
						serialThreadObject->keys[ID].scale = 0.0;
					}
					serialThreadObject->keys[ID].threshUp = XML.getValue("key:threshold:up", serialThreadObject->threshUp, i);
					serialThreadObject->keys[ID].threshDown = XML.getValue("key:threshold:down", serialThreadObject->threshDown, i);
				}
			}
			
			serialThreadObject->numImuAddr = numPtTags = XML.getNumTags("imu");
			if(numPtTags > 0) {
				totalToRead = MIN(numPtTags, 12);
				for(i = 0; i < totalToRead; i++) {
					ID = XML.getAttribute("imu", "id", 0, i);

					str1 = XML.getValue("imu:oscaddress", "/sabre/motion/0", i);
					if(str1.length() > 0) {
						serialThreadObject->imuaddresses[ID] = str1;
					}
				}
			}
			
			serialThreadObject->numButtonAddr = numPtTags = XML.getNumTags("button");
			if(numPtTags > 0) {
				totalToRead = MIN(numPtTags, 3);
				for(i = 0; i < totalToRead; i++) {
					ID = XML.getAttribute("button", "id", 0, i);

					str1 = XML.getValue("button:oscaddress", "/sabre/button/0", i);
					if(str1.length() > 0) {
						serialThreadObject->buttonaddresses[ID] = str1;
					}
				}
			}
			
			serialThreadObject->numAirAddr = numPtTags = XML.getNumTags("air");
			if(numPtTags > 0) {
				totalToRead = MIN(numPtTags, 2);
				for(i = 0; i < totalToRead; i++) {
					ID = XML.getAttribute("air", "id", 0, i);

					str1 = XML.getValue("air:oscaddress", "/sabre/air/0", i);
					if(str1.length() > 0) {
						serialThreadObject->airaddresses[ID] = str1;
					}
				}
                serialThreadObject->airValue.minimum = XML.getValue("air:minimum", 0, 0);
                serialThreadObject->airValue.maximum = XML.getValue("air:maximum", 120, 0);
                if(serialThreadObject->airValue.maximum != serialThreadObject->airValue.minimum) {
                    serialThreadObject->airValue.scale = 1.0 / (serialThreadObject->airValue.maximum - serialThreadObject->airValue.minimum);
                }else{
                    serialThreadObject->airValue.scale = 0.0;
                }
			}						
			
			ID = XML.getAttribute("system", "id", 0, i);
			
			str1 = XML.getValue("system:oscaddress", "/sabre/systime", i);
			if(str1.length() > 0) {
				serialThreadObject->timestampAddressServer = str1+"/server";
                serialThreadObject->timestampAddressLeft = str1+"/left";
                serialThreadObject->timestampAddressRight = str1+"/right";
                serialThreadObject->timestampAddressAir = str1+"/air";  
			}	
			
//			ID = XML.getAttribute("keycode", "id", 0, i);
			
			str1 = XML.getValue("keycode:oscaddress", "/sabre/keycode", i);
			if(str1.length() > 0) {
				serialThreadObject->keycodeaddress = str1;
			}
			
//			ID = XML.getAttribute("midinote", "id", 0, i);
			
			str1 = XML.getValue("midinote:oscaddress", "/sabre/note", i);
			if(str1.length() > 0) {
				serialThreadObject->midinoteaddress = str1;
			}	
			
//			ID = XML.getAttribute("heading", "id", 0, i);
			
			str1 = XML.getValue("heading:oscaddress", "/sabre/heading", i);
			if(str1.length() > 0) {
				serialThreadObject->headingaddress = str1;
			}	
			
			XML.popTag();
		}
        ofLog(OF_LOG_NOTICE, "success reading Settings File");

	}
	return result;
}

void sabreServer::dumpPrefs()
{
	int i;
	
	printf("serialport %s\n", serialThreadObject->serialport.c_str());
	printf("sender IP %s\n", serialThreadObject->sendIP.c_str());
	printf("sender port %d\n", serialThreadObject->sendport);
	printf("receive port %d\n", receiveport);
	printf("baudrate %d\n", serialThreadObject->baudrate);
	printf("framerate %d\n", framerate);
	printf("display %d\n", display);
	printf("threshDown %f\n", serialThreadObject->threshDown);
	printf("threshUp %f\n", serialThreadObject->threshUp);
	printf("accelResolution %d\n", serialThreadObject->accelResolution);
	printf("accelOffset %ld\n", serialThreadObject->accelOffset);
	printf("accelScale %2.12f\n", serialThreadObject->accelScale);
	printf("OSCfullspeed %d\n", serialThreadObject->fullspeedOSC);
	printf("OSCinterval %d\n", OSCThreadObject->OSCInterval);
    
	for(i = 0; i < serialThreadObject->numKeyAddr; i++) {
		printf("key %d\n", i);
		printf("    oscaddress %s\n", serialThreadObject->keys[i].oscaddress.c_str());
		printf("    inverted %d\n", serialThreadObject->keys[i].inverted);
		printf("    minimum %ld\n", serialThreadObject->keys[i].minimum);
		printf("    maximum %ld\n", serialThreadObject->keys[i].maximum);
		printf("    threshUp %f\n", serialThreadObject->keys[i].threshUp);
		printf("    threshDown %f\n", serialThreadObject->keys[i].threshDown);
	}
	printf("keycode\n    oscaddress %s\n", serialThreadObject->keycodeaddress.c_str());

    for(i = 0; i < serialThreadObject->numKeyAddr; i++) {
		printf("airValues %d\n", i);
		printf("    minimum %f\n", serialThreadObject->airValue.minimum);
		printf("    maximum %f\n", serialThreadObject->airValue.maximum);
    }
    
	printf("imu\n");		   
	for(i = 0; i < serialThreadObject->numImuAddr; i++){
		printf("    oscaddress %d %s\n",i, serialThreadObject->imuaddresses[i].c_str() );
	}

	printf("button\n");		   
	for(i = 0; i < serialThreadObject->numButtonAddr; i++){
		printf("    oscaddress %d %s\n", i, serialThreadObject->buttonaddresses[i].c_str());
	}
	printf("air\n");		   

	for(i = 0; i < serialThreadObject->numAirAddr; i++){
		printf("    oscaddress %d %s\n", i, serialThreadObject->airaddresses[i].c_str());

	}
	printf("timestamp\n    oscaddress server %s\n", serialThreadObject->timestampAddressServer.c_str());
    printf("    oscaddress left %s\n", serialThreadObject->timestampAddressLeft.c_str());
	printf("    oscaddress right %s\n", serialThreadObject->timestampAddressRight.c_str());
	printf("    oscaddress air %s\n", serialThreadObject->timestampAddressAir.c_str());

    printf("link quality\n    oscaddress left %s\n", serialThreadObject->linkQualityAddressLeft.c_str());
    printf("    oscaddress right %s\n", serialThreadObject->linkQualityAddressRight.c_str());
	printf("    oscaddress air %s\n", serialThreadObject->linkQualityAddressAir.c_str());

    printf("battery\n    oscaddress main %s\n", serialThreadObject->batteryAddressMain.c_str());
    printf("    oscaddress air %s\n", serialThreadObject->batteryAddressAir.c_str());

    
}


void sabreServer::writePrefs()
{
    // only store GUI changeable control parameters:
	XML.setValue("sabre:display",					display);
	XML.setValue("sabre:serialport",				serialThreadObject->serialport);

	XML.saveFile("sabreServer.xml");
	return;
}

void sabreServer::readMidicodes()
{	
	int numTags;
	int numPtTags;
	bool result = XMLmidi.loadFile("sabreMidicodes.xml");
	// printf("midiCodes loaded with result %d\n", result);
	if(result) {

		numTags = XMLmidi.getNumTags("sabre:note");
		// printf("readMidicodes numTags %d\n", numTags);
		if(numTags > 0) {
			XMLmidi.pushTag("sabre", numTags-1);
			numPtTags = XMLmidi.getNumTags("note");
			// printf("readMidicodes numPtTags %d\n", numPtTags);

//			printf("midiCodes loaded with %d note\n", numPtTags);
			for(int i = 0; i < 128; i++) {
				serialThreadObject->midiNote[i].note = 0;
				serialThreadObject->midiNote[i].keycode = -1;
			}
			for(int i = 0; i < numPtTags; i++) {
				serialThreadObject->midiNote[i].note = XMLmidi.getValue("note:midi", 0, i);
				serialThreadObject->midiNote[i].keycode = XMLmidi.getValue("note:code", 0, i);
//				printf("midinote ID %d note %d code %ld\n", i, serialThreadObject->midiNote[i].note, serialThreadObject->midiNote[i].keycode); 
			}
			XML.popTag();
		}
        ofLog(OF_LOG_NOTICE, "success reading MIDI-codes");
	}
}

void sabreServer::writeScaling()
{	
	XML.pushTag("sabre", 0);

	for(int i = 0; i < serialThreadObject->numKeyAddr; i++) {
		XML.pushTag("key", i);
		
		XML.removeTag("minimum", 0);
		XML.removeTag("maximum", 0);
		XML.setValue("minimum", (int)serialThreadObject->keys[i].minimum, i);
		XML.setValue("maximum", (int)serialThreadObject->keys[i].maximum, i);
		// printf("key %d min %d max %d\n", i, (int)serialThreadObject->keys[i].minimum, (int)serialThreadObject->keys[i].maximum);
		XML.popTag();
	}
    XML.removeTag("air:minimum", 0);
    XML.removeTag("air:maximum", 0);
    XML.setValue("air:minimum", (int)serialThreadObject->airValue.minimum, 0);
    XML.setValue("air:maximum", (int)serialThreadObject->airValue.maximum, 0);
    
	XML.popTag();
	XML.saveFile("sabreServer.xml");
	return;
}

void sabreServer::resetCalibrate()
{
	for(int i = 0; i < serialThreadObject->numKeyAddr; i++) 
	{
		serialThreadObject->keys[i].minimum = 1023;
		serialThreadObject->keys[i].maximum = 0;
        
        serialThreadObject->airValue.minimum = 32768;
        serialThreadObject->airValue.maximum = -32768;
	}
}

void sabreServer::keyReleased(int key)
{
	switch(key){
		case 'f': // f-key: switch winow size
			drawValues = !drawValues;
            serialThreadObject->drawValues = drawValues;
			windowChanged = 1;
			redrawFlag = 1;
//			if(!drawValues) {
//				serialThreadObject->calibrate = 0;
//				writeScaling();
//			} else {
//				resetCalibrate();
//			}
			break;
		case 'c': // c-key:
			/*
			serialThreadObject->calibrate != serialThreadObject->calibrate;
			if(serialThreadObject->calibrate == 1) { // before we switch it off
				writeScaling();  // we write the values into the prefs
			} else {
				resetCalibrate();
			}
			redrawFlag = 1;
			windowChanged = 1;
			lastRedraw = ofGetElapsedTimef();
			*/
			break;
		case 'r': // r key:
			break;
		case 'p':
			// sabreServer::readPrefs();
			break;
		case 'F':
			drawValues = !drawValues;
            serialThreadObject->drawValues = drawValues;
			windowChanged = 1;
			redrawFlag = 1;
//			if(!drawValues) {
//				serialThreadObject->calibrate = 0;			
//				writeScaling();
//			} else {
//				resetCalibrate();
//			}
			break;			
		case 'w':
			// sabreServer::writePrefs();
			break;
	}
}

void sabreServer::mouseMoved( int x, int y)
{
}

void sabreServer::mouseDragged( int x, int y, int button)
{
}

void sabreServer::mouseReleased()
{
}

void sabreServer::mousePressed(int x, int y, int button)
{
	int i;
	printf("mousepressed at %d %d\n", x, y);
	ofRect(295, 36, 295+124, 36+20);
	
	// click in start/stope values
	if(x > 270 && x < 394 && y > 4 && y < 25) {
		// printf("start/stop clicked with status %d\n", serialThreadObject->status);
		if(serialThreadObject->status) {
			stopSerial();
//			drawValues = 0;
//			serialThreadObject->status = false;
		} else {
            stopSerial();
            ofSleepMillis(5);
			startSerial();
			drawValues = 1;
            serialThreadObject->drawValues = 1;
//			serialThreadObject->status = true;
		}
		
		windowChanged = 1;
		redrawFlag = 1;
	}
	// click in show/hide values
	if(x > 397 && x < 520 && y > 3 && y < 24) {
		if(drawValues != 0) {
			drawValues = 0;
            serialThreadObject->drawValues = 0;
			serialThreadObject->calibrateAll = 0;
		} else {
			drawValues = 1;
            serialThreadObject->drawValues = 1;
		}
		windowChanged = 1;
		redrawFlag = 1;
	}
	// click in menu-original-textbox
	if(x > 10 && x < 200
       && y > 3 && y < 21) {
		if(menuState != 0) {
			menuState = 0;
		} else {
			menuState = 1;
			getSerialDeviceList();
			stopSerial();
			drawValues = 1;
            serialThreadObject->drawValues = 1;
		}
		windowChanged = 1;
		redrawFlag = 1;
		// printf("menustate %d\n", menuState);
	}
	
	// click in menu?
	if(menuState) {
		if(x > 15 && x < 203 && y > 21 && y < 21+numMenuItems*18) {
			int clickedItem = (y - 21) / 18;
			
			// printf("clicked in menu at %d %d clickedItem %d\n", x, y, clickedItem);
			menuState = 0;

			vector<ofSerialDeviceInfo>::iterator it;
			int i = 0;
			for ( it = serialThreadObject->deviceList.begin(); it < serialThreadObject->deviceList.end(); it++) {
				if(!strncmp(it->getDeviceName().c_str(), "tty", 3)){
					if(i == clickedItem) {
						serialThreadObject->serialport = it->getDevicePath();
                        stopSerial();
                        ofSleepMillis(5);
						startSerial();
					
						XML.pushTag("sabre", 0);
						XML.removeTag("serialport", 0);
						XML.setValue("serialport", serialThreadObject->serialport, 0);
						XML.popTag();
						XML.saveFile("sabreServer.xml");
						
						break;
					}
					i++;
				}
			}
			redrawFlag = 1;
		}
	}
	// click in calibrate keys
	// ofRect(295, 690, 124, 20);
	if(x > 375 && x < 500 && y > 480 && y < 500) {
		if(serialThreadObject->calibrateAll != 0){
			serialThreadObject->calibrateAll = 0;
			for(i = 0; i < MAXNUM; i++) {
				serialThreadObject->calibrate[i] = 0;

			}
			writeScaling();
		} else {
			serialThreadObject->calibrateAll = 1;
			for(i = 0; i < MAXNUM; i++) {
				serialThreadObject->calibrate[i] = 1;
			}
			resetCalibrate();
		}
//		printf("serialThreadObject->calibrateAll is %d\n", serialThreadObject->calibrate);
	}
    // click in calibrate air
    if(x > 502 && x < 627 && y > 480 && y < 500) {
		if(serialThreadObject->airValue.calibratePressureRange != 0){
			serialThreadObject->airValue.calibratePressureRange = 0;
            writeScaling();
        } else {
            serialThreadObject->airValue.calibratePressureRange = 1;
            resetCalibrate();
        }
    //		printf("serialThreadObject->calibrateAll is %d\n", serialThreadObject->calibrate);
        }
}

