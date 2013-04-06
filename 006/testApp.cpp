#include "testApp.h"

void testApp::setup()
{
	ofSetEscapeQuitsApp(false);
	ofEnableAlphaBlending();
	titleString = "sabreServer v0.5";
	
	serialThreadObject = new(threadedSerial);
	
	TTF.loadFont("lucidagrande.ttf", 8, 1, 1, 0);
	TTFsmall.loadFont("lucidagrande.ttf", 8, 1, 0, 0);
	serialThreadObject->TTF.loadFont("lucidagrande.ttf", 8, 1, 1, 0);
	
	texScreen.allocate(440, 681, GL_RGB);
	
	windowChanged = 1;
	drawValues = 0;
	menuState = 0;
	
	serialThreadObject->serialport = "/dev/tty.usbserial-A7005Ghs";
	serialThreadObject->baudrate = 115200;
	serialThreadObject->sendIP = "127.0.0.1";
	serialThreadObject->sendport = 40000;
	
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
	
	// open an outgoing connection to sendIP:PORT
	
	status = serialThreadObject->sender.setup( serialThreadObject->sendIP.c_str(), serialThreadObject->sendport);
	if(status) {
		status2 = "Sending OSC to "+serialThreadObject->sendIP+" on port "+ofToString(serialThreadObject->sendport);
	} else {
		status2 = "Unable to open Network "+serialThreadObject->sendIP+" on port "+ofToString(serialThreadObject->sendport);
	}
	
	receiver.setup( receiveport );
	
	framerate = 25;
	ofSetFrameRate( framerate ); // cap the glut callback rate
	ofSetVerticalSync( true );
	//ofEnableAlphaBlending();
	ofBackground( 200, 200, 200);	
	//ofSetBackgroundAuto(false);
	redrawFlag = 1;
	// redrawInterval = redrawValues[display]; // in seconds
	firstflag = 1;
	
	runOnce = 1;
	runOnceDelay = 1.0f;
	lastRedraw = runOnceStart = ofGetElapsedTimef();
	
	
//	serialThreadObject->deviceList = serialThreadObject->serial.getDeviceList();
//	vector<ofSerialDeviceInfo>::iterator it;
//	for ( it = serialThreadObject->deviceList.begin() ; it < serialThreadObject->deviceList.end(); it++) {
//		string devPath = it->getDevicePath();
//		string devName = it->getDeviceName();
//		if(!strncmp(devName.c_str(), "tty.", 4)){
//			printf("device %s %s\n", devPath.c_str(), devName.c_str());
//		}
//	}

 	getSerialDeviceList();
	
	startSerial();
	
//	serialThreadObject->status = serialThreadObject->serial.setup(serialThreadObject->serialport, serialThreadObject->baudrate);
//	if(serialThreadObject->status) {
//		status1 = "Device open "+serialThreadObject->serialport+" "+ofToString(serialThreadObject->baudrate);
//		ofSetWindowTitle(serialThreadObject->serialport);
//		serialThreadObject->start();
//	} else {
//		status1 = "Unable to open "+serialThreadObject->serialport;
//		serialThreadObject->start();
//		serialThreadObject->stop();
//		ofSystemAlertDialog("SABRe Server \n"+status1);
//	}
	
	ofSetWindowPosition(0,44);
}


void testApp::update()
{
	float now = ofGetElapsedTimef();
	
	if(runOnce == 1) {
		if(now > (runOnceStart + runOnceDelay) )
		{
		}
	}
	receiveOSC();
}


void testApp::draw()
{
	int i;
	int anchorx = 15;
	int anchory = 64;	
	int stepsize = 18;
	int columnwidth = 200;
	int rightColumn = 295;
	int leftColumn = 105;
	int width;
	int height;
	double yy;
	
	if(windowChanged == 1) {
		if(drawValues == 0) {
			width = 430;
			height = 49;
			ofSetWindowShape(width, height);
			windowChanged = 0;
		} else if(drawValues == 1) {
			width = 430;
			height = 722; // 790
			timeOut = 5.0;
			ofSetWindowShape(width, height);
			windowChanged = 0;

		}
	}
	
	if(redrawFlag == 1) // drawn once after first update
	{
		// marker colors
		ofFill();
		ofSetColor(0, 91, 255, 127);
		ofRect(0, 0, 4, 49);
		ofRect(width-4, 0, 4, 49);
		ofRect(4, 0, width-8, 4);
		ofRect(4, 45, width-8, 4);
		
		ofSetColor(200, 191, 191, 127);
		ofRect(0, 0, width, height);
		
		if(serialThreadObject->status) {
			ofSetColor(0, 0, 0, 255);
		} else {
			ofSetColor(191, 0, 0, 255);
		}
		TTF.drawString(status1, anchorx, 16);
		TTF.drawString(status2, anchorx,  38);

		// separator lines
		ofSetColor(240, 240, 240, 127);
		ofLine(0, anchory-14, width, anchory-14);	
		ofSetColor(127, 127, 127, 127);
		ofLine(0, anchory-13, width, anchory-13);		
		
		ofSetColor(200, 200, 200, 255);
		
		// Menu
		ofFill();
		ofSetColor(208, 208, 208);
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
			ofSetColor(232, 232, 232);
			ofRect(rightColumn, 3, 124, 18);
			ofNoFill();
			ofSetColor(127, 127, 127);
			ofRect(rightColumn, 3, 124, 18);
			ofSetColor(0, 0, 0);
			TTFsmall.drawString("Stop", rightColumn+48, 16);
		} else {
			ofFill();
			ofSetColor(208, 208, 208);
			ofRect(rightColumn, 3, 124, 18);
			ofNoFill();
			ofSetColor(127, 127, 127);
			ofRect(rightColumn, 3, 124, 18);
			ofSetColor(0, 0, 0);
			TTFsmall.drawString("Start", rightColumn+48, 16);
		}
		
		// show values button
		ofFill();
		ofSetColor(208, 208, 208);
		ofRect(rightColumn, 23, 124, 20);
		ofNoFill();
		ofSetColor(127, 127, 127);
		ofRect(rightColumn, 23, 124, 20);
		ofSetColor(0, 0, 0);		
		TTFsmall.drawString("Show Values", rightColumn+28, 38);
		
		// Calibrate Button
		ofFill();
		ofSetColor(208, 208, 208);
		ofRect(rightColumn, 690, 124, 20);
		ofNoFill();
		ofSetColor(127, 127, 127);
		ofRect(rightColumn, 690, 124, 20);
		ofSetColor(0, 0, 0);
		TTFsmall.drawString("Calibrate", rightColumn+40, 704);

		
		// value display left column
		for(i = 0; i < 36; i++) { // stripes
			if((i % 2) == 0){
				ofFill();
				ofSetColor(216, 216, 216, 255);
				ofRect(anchorx-2, anchory+((i-1) * stepsize)+7, 141, 16);
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
			TTFsmall.drawString(str, anchorx, anchory+((i+25) * stepsize) );
//			TTFsmall.drawString(serialThreadObject->imuaddresses[i/3], anchorx, anchory+5+((i+25) * stepsize) );
		}
		TTFsmall.drawString(serialThreadObject->airaddresses[0], anchorx, anchory+(34 * stepsize) );
		for(i = 0; i < 1; i++){ // first button address truncated
			char temp[64];
			strncpy(temp, serialThreadObject->buttonaddresses[0].c_str(), serialThreadObject->buttonaddresses[0].size()-2);
			temp[serialThreadObject->buttonaddresses[0].size()-2] = 0;
			TTFsmall.drawString(temp, anchorx, anchory+((i+35) * stepsize) );
		}

		texScreen.loadScreenData(0,0, 440, 681);
		drawTex = 1;
	} else {
		if(drawTex) {
			texScreen.draw(0, 0, 440, 266);
			drawTex = 0;
		}
		
	}
	
	if(drawValues) {
		serialThreadObject->draw();
	}
	
	if(menuState) {
		numMenuItems = (int)serialThreadObject->deviceList.size() / 2;

		ofSetColor(208, 208, 208);
		ofFill();
		ofRect(leftColumn, 21, 188, numMenuItems*18);

		vector<ofSerialDeviceInfo>::iterator it;
		int i;
		for ( it = serialThreadObject->deviceList.begin(), i = 0 ; it < serialThreadObject->deviceList.end(); it++) {
			if(!strncmp(it->getDeviceName().c_str(), "tty", 3)){
				ofNoFill();
				ofSetColor(191, 191, 191);
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

void testApp::exit()
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

void testApp::startSerial()
{
	if(serialThreadObject->status) {
		serialThreadObject->serial.close();
		serialThreadObject->stop(); // the thread
	}
	
	serialThreadObject->status = serialThreadObject->serial.setup(serialThreadObject->serialport, serialThreadObject->baudrate);
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

void testApp::stopSerial()
{
	if(serialThreadObject->status) {
		serialThreadObject->serial.close();
		serialThreadObject->stop(); // the serial thread
		serialThreadObject->status = false;
	}
	redrawFlag = 1;
}

void testApp::getSerialDeviceList()
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

void testApp::receiveOSC()
{
	int i;
	string temp;
	
	while( receiver.hasWaitingMessages()) {
		ofxOscMessage m;
		receiver.getNextMessage( &m );
		temp = m.getAddress();
		
		if ( !strcmp( temp.c_str(), "/sabre/framerate" )) {
			framerate = m.getArgAsInt32( 0 );
			status2 = "sending OSC to "+serialThreadObject->sendIP+" on port "+ofToString(serialThreadObject->sendport);
		}else if ( strcmp( temp.c_str(), "/sabre/display" ) == 0 ) {
			display = m.getArgAsInt32( 0 );
			windowChanged = 1;
		}else if ( strcmp( temp.c_str(), "/sabre/reset" ) == 0 ) {
		}else if ( strcmp( temp.c_str(), "/sabre/writePrefs" ) == 0 ) {
			testApp::writePrefs();
		}else if ( strcmp( temp.c_str(), "/sabre/readPrefs" ) == 0 ) {
			testApp::readPrefs();
		}else if ( strcmp( temp.c_str(), "/sabre/network/receiver/port" ) == 0 ) {
			receiveport = m.getArgAsInt32( 0 );
			receiver.setup( receiveport );
		}else if ( strcmp( temp.c_str(), "/sabre/network/sender/port" ) == 0 ) {
			serialThreadObject->sendport = m.getArgAsInt32( 0 );
			serialThreadObject->sender.setup( serialThreadObject->sendIP, serialThreadObject->sendport );
			status2 = "sending OSC to "+serialThreadObject->sendIP+" on port "+ofToString(serialThreadObject->sendport);
		}else if ( strcmp( temp.c_str(), "/sabre/network/sender/IP" ) == 0 ) {
			serialThreadObject->sendIP = m.getArgAsString(0);
			serialThreadObject->sender.setup( m.getArgAsString(0), serialThreadObject->sendport );
			status2 = "sending OSC to "+serialThreadObject->sendIP+" on port "+ofToString(serialThreadObject->sendport);
		}else if ( strcmp( temp.c_str(), "/sabre/exit" ) == 0 ) {
			testApp().exit();
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
		}
	}
}

bool testApp::readPrefs()
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
		
		serialThreadObject->sendIP = XML.getValue("sabre:network:sender:IP", "127.0.0.1");
		serialThreadObject->sendport = XML.getValue("sabre:network:sender:port", 40000);
		receiveport = XML.getValue("sabre:network:receiver:port", 40001);

		serialThreadObject->baudrate = XML.getValue("sabre:baudrate", 57600);
		framerate = XML.getValue("sabre:framerate", 200);
		drawValues = XML.getValue("sabre:display", 0);
		drawValues = CLAMP(drawValues, 0, 1);
		
		serialThreadObject->threshDown = XML.getValue("sabre:thresholds:down", 0.2);
		serialThreadObject->threshUp = XML.getValue("sabre:thresholds:up", 0.8);
		serialThreadObject->debounceTimeOut = XML.getValue("sabre:debounce-timeout", 0);

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
			}						
			
			ID = XML.getAttribute("system", "id", 0, i);
			
			str1 = XML.getValue("system:oscaddress", "/sabre/systime", i);
			if(str1.length() > 0) {
				serialThreadObject->systemaddress = str1;
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
	}
	return result;
}

void testApp::dumpPrefs()
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

	for(i = 0; i < serialThreadObject->numKeyAddr; i++) {
		printf("key %d\n", i);
		printf("    oscaddress %s\n", serialThreadObject->keys[i].oscaddress.c_str());
		printf("    inverted %d\n", serialThreadObject->keys[i].inverted);
		printf("    minimum %ld\n", serialThreadObject->keys[i].minimum);
		printf("    maximum %ld\n", serialThreadObject->keys[i].maximum);
		printf("    threshUp %f\n", serialThreadObject->keys[i].threshUp);
		printf("    threshDown %f\n", serialThreadObject->keys[i].threshDown);
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
	printf("system\n    oscaddress %s\n", serialThreadObject->systemaddress.c_str());
	printf("keycode\n    oscaddress %s\n", serialThreadObject->keycodeaddress.c_str());
}


void testApp::writePrefs()
{
	XML.setValue("sabre:framerate",					framerate);

	XML.setValue("sabre:display",					display);
	XML.setValue("sabre:network:receiver:port",		receiveport);
	XML.setValue("sabre:network:sender:port",		serialThreadObject->sendport);
	XML.setValue("sabre:network:sender:IP",			serialThreadObject->sendIP);
	XML.setValue("sabre:serialport",				serialThreadObject->serialport);
	XML.setValue("sabre:baudrate",					serialThreadObject->baudrate);

	XML.saveFile("sabreServer.xml");
	return;
}

void testApp::readMidicodes()
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

			printf("midiCodes loaded with %d note\n", numPtTags);
			for(int i = 0; i < 128; i++) {
				serialThreadObject->midiNote[i].note = 0;
				serialThreadObject->midiNote[i].keycode = -1;
			}
			for(int i = 0; i < numPtTags; i++) {
				serialThreadObject->midiNote[i].note = XMLmidi.getValue("note:midi", 0, i);
				serialThreadObject->midiNote[i].keycode = XMLmidi.getValue("note:code", 0, i);
				printf("midinote ID %d note %d code %ld\n", i, serialThreadObject->midiNote[i].note, serialThreadObject->midiNote[i].keycode); 
			}
			XML.popTag();
		}
	}
}

void testApp::writeScaling()
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
	XML.popTag();
	XML.saveFile("sabreServer.xml");
	return;
}

void testApp::resetCalibrate()
{
	for(int i = 0; i < serialThreadObject->numKeyAddr; i++) 
	{
		serialThreadObject->keys[i].minimum = 1023;
		serialThreadObject->keys[i].maximum = 0;
	}
}

void testApp::keyReleased(int key)
{
	switch(key){
		case 'f': // f-key: switch winow size
			drawValues = !drawValues;
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
			// testApp::readPrefs();
			break;
		case 'F':
			drawValues = !drawValues;
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
			// testApp::writePrefs();
			break;
	}
}

void testApp::mouseMoved( int x, int y)
{
}

void testApp::mouseDragged( int x, int y, int button)
{
}

void testApp::mouseReleased()
{
}

void testApp::mousePressed(int x, int y, int button)
{
	int i;
	// printf("mousepressed at %d %d\n", x, y);
	ofRect(295, 36, 295+124, 36+20);
	
	// click in start/stope values
	if(x > 295 && x < 419 && y > 4 && y < 25) {
		// printf("start/stop clicked with status %d\n", serialThreadObject->status);
		if(serialThreadObject->status) {
			stopSerial();
			drawValues = 0;
//			serialThreadObject->status = false;
		} else {
			startSerial();
			drawValues = 1;
//			serialThreadObject->status = true;
		}
		
		windowChanged = 1;
		redrawFlag = 1;
	}
	// click in show/hide values
	if(x > 295 && x < 419 && y > 26 && y < 48) {
		if(drawValues != 0) {
			drawValues = 0;
			serialThreadObject->calibrateAll = 0;			
		} else {
			drawValues = 1;
		}
		windowChanged = 1;
		redrawFlag = 1;
	}
	// click in menu-original-textbox
	if(x > 105 && x < 293 && y > 3 && y < 21) {
		if(menuState != 0) {
			menuState = 0;
		} else {
			menuState = 1;
			getSerialDeviceList();
			stopSerial();
			drawValues = 1;
		}
		windowChanged = 1;
		redrawFlag = 1;
		// printf("menustate %d\n", menuState);
	}
	
	// click in menu?
	if(menuState) {
		if(x > 105 && x < 293 && y > 21 && y < 21+numMenuItems*18) {
			int clickedItem = (y - 21) / 18;
			
			// printf("clicked in menu at %d %d clickedItem %d\n", x, y, clickedItem);
			menuState = 0;

			vector<ofSerialDeviceInfo>::iterator it;
			int i = 0;
			for ( it = serialThreadObject->deviceList.begin(); it < serialThreadObject->deviceList.end(); it++) {
				if(!strncmp(it->getDeviceName().c_str(), "tty", 3)){
					if(i == clickedItem) {
						serialThreadObject->serialport = it->getDevicePath();
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
	// click in calibrate
	// ofRect(295, 690, 124, 20);
	if(x > 295 && x < 419 && y > 690 && y < 710) {
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
}

