/*
 *  threadedSerial.cpp
 *  oscSerialSender
 *
 *  Created by jasch on 9/11/11.
 *  Copyright 2011 j45ch. All rights reserved.
 *
 */

#include "threadedSerial.h"

threadedSerial::threadedSerial()
{
	int i;
	
	scale10 = 1.0 / 1024.0;
	scale11 = 1.0 / 2048.0;
	scale12 = 1.0 / 4096.0;
	scale13 = 1.0 / 8192.0;
	scale16 = 1.0 / 65536.0;
	//	scale32 = 1.0 / 2147483648;
	
	accelResolution = 4;
	accelOffset = 1024;
	accelScale = scale10;
	
	gyroResolution = 16;
	gyroOffset = 32768;
	gyroScale = scale16;
	
	magnetoResolution = 8;
	magnetoOffset = 2047;
	magnetoScale = scale12;
	
	tempOffset = 32768;
	tempScale = scale16;
	
	accelResolution = 4;
	headingLowpassFactor = 0.2;
	headingOld_x = headingOld_y = 0.0;
	
	// set the default address
	for(i = 0; i < 25; i++) {
		address[0] = "/sabre/key/"+ofToString(i)+"/continuous";
	}
	
	TTF.loadFont("lucidagrande.ttf", 8, 1, 1, 0);
	calibrateAll = 0;
	for(i = 0; i < MAXNUM; i++) {
		calibrate[i] = 0;
	}
}

threadedSerial::~threadedSerial()
{
	
	if( isThreadRunning() ) { 
		stopThread();
	}
}


void threadedSerial::start()
{
	startThread(true, false);   // blocking, verbose
}

void threadedSerial::stop()
{
	stopThread();
}

//--------------------------
void threadedSerial::threadedFunction() 
{
	while( isThreadRunning() != 0 ){
		if( lock() ){
			haveResult[0] = 0;
			haveResult[1] = 0;
			haveResult[2] = 0;
			haveResult[3] = 0;
			
			readSerial(); // this is the threaded serial polling call 
			sendOSC();
			
			ofSleepMillis(1);
			unlock();
			
		}
	}
}

void threadedSerial::readSerial()
{
	long timesRead = 0;
	long nRead  = 0;	
	unsigned char bytesReturned[1];
	
	while( (nRead = serial.readBytes(bytesReturned, 1)) > 0){
		nTimesRead++;	
		nBytesRead = nRead;
		serialparse(bytesReturned);
	}
}

void threadedSerial::serialparse(unsigned char *c)
{
	long i;
	long sum;
	int size = PATTERNLENGTH;
	
	// push bytes forward in the circular buffer "seen"
	for (i = 0; i < size-1; i++) {
		seen[i] = seen[i+1];
	}
	seen[size-1] = c[0]; // append new byte to buffer
	
	// pattern matching
	if (seen[0] == 65) { // start packet
		if(seen[1] == 240) {	// start left hand packet
			if(seen[2] == 28) {
				if(seen[31] == 90) {
					for(i = 0; i < 27; i++) {
						result[0][i] = seen[i+3];
					}
					if(calcChecksum(0, 27) == seen[30]) {
						haveResult[0] = true;
						parseLeft();
						calcKeycode();
					}
				}
			}
		} else if(seen[1] == 241) { // start right hand packet
			if(seen[2] == 28) {
				if(seen[31] == 90) {
					for(i = 0; i < 27; i++) {
						result[1][i] = seen[i+3];
					}
					if(calcChecksum(1, 27) == seen[30]) {
						haveResult[1] = true;
						parseRight();
						calcKeycode();
					}
				}
			}
		} else if(seen[1] == 242) { // start IMU packet
			if(seen[2] == 21) {
				if(seen[24] == 90) {
					for(i = 0; i < 20; i++) {
						result[2][i] = seen[i+3];
					}
					if(calcChecksum(2, 20) == seen[23]) {
						haveResult[2] = true;
						parseIMU();
					}
				}
			}
		} else if(seen[1] == 243) { // // start air packet
			if(seen[2] == 9) {
				// 65 243 9 p1 p2 p3 p4 t1 t2 t3 t4 X 90
				if(seen[12] == 90) {
					for(i = 0; i < 8; i++) {
						result[3][i] = seen[i+3];
					}
//					if(calcChecksum(3, 8) == seen[11]) {
						haveResult[3] = true;
						parseAir();
//					}
					// 65 243 9 p1 p2 p3 p4 t1 t2 t3 t4 X Y 90
				}else if(seen[13] == 90) {
					for(i = 0; i < 8; i++) {
						result[3][i] = seen[i+3];
					}
					haveResult[3] = true;
					parseAir();
				}
			}
		}
	} // end patternmatching
}

unsigned char threadedSerial::calcChecksum(int which, int length)
{
	unsigned char checksum = 0;
	for (int i = 0; i < length; i++) {
		checksum ^= result[which][i];
	}
	return checksum;	
}


void threadedSerial::parseLeft()
{
	int temp;
	int i, j;
	
	if(haveResult[0]) {
		int now = ofGetElapsedTimeMillis();
		
		for (int i = 0; i < 13; i++) { // 13 keys
			j = i;
			result[0][i*2] = result[0][i*2] & 0x03; // mask upper bits to filter out noise
			keys[j].raw = (result[0][i*2] * 256) + result[0][i*2+1];

			if(keys[j].inverted) {
				keys[j].raw = 1023 - keys[j].raw;
			}
			
			if(calibrate[j]) {
				if(keys[j].raw < keys[j].minimum){
					keys[j].minimum = keys[j].raw;
				}
				if(keys[j].raw > keys[j].maximum){
					keys[j].maximum = keys[j].raw;
				}
				if(keys[j].maximum != keys[j].minimum) {
					keys[j].scale = 1.0 / (keys[j].maximum - keys[j].minimum);
				} else {
					keys[j].scale = 0.0;
				}
			}
		
			keys[j].continuous = (float)( keys[j].raw - keys[j].minimum ) * keys[j].scale;
			keys[j].continuous = CLAMP(keys[j].continuous, 0.0, 1.0);

			if(keys[j].continuous < keys[j].threshDown) {
				keys[j].binary = 0;
			} else if(keys[j].continuous > keys[j].threshUp) {
				if(now - keys[j].lastTriggerTime > debounceTimeOut) {
					keys[j].binary = 1;
					keys[j].lastTriggerTime = now;
				}
			}

#ifdef FILTER_CHANGE
			if(keys[j].raw != keys[j].rawOld) {
				keys[j].changed = 1;

				if(keys[j].binary != keys[j].binaryOld) {
					keys[j].binaryOld = keys[j].binary;
					keys[j].binaryChanged = 1;
					// printf("left hand activated: key binary changed: keys[%d].binary is %d\n", j, keys[j].binary);
				} else {
					keys[j].binaryChanged = 0;
					// printf("left hand activated: below lower thresh\n");

				}
				keys[j].rawOld = keys[j].raw;
			} else {
				keys[j].changed = 0;
			}
#endif
		}
		button[0] = result[0][26] & 1;
		button[1] = result[0][26] & 2;
		button[2] = result[0][26] & 4;
		
		for(int i = 0; i < 3; i++) {
			if(button[i] != buttonOld[i]) {
				buttonChanged[i] = 1;
				buttonOld[i] = button[i];
			} else {
				buttonChanged[i] = 0;
			}
		}
	} // end haveResult check
}

void threadedSerial::parseRight()
{
	int temp;
	int i, j;
	
	if(haveResult[1]) {
		int now = ofGetElapsedTimeMillis();
		
		for (int i = 0; i < 12; i++) { // 12 keys
			j = i+13; // the keys 13 to 25
			
			result[1][i*2] = result[1][i*2] & 0x03; // mask upper bits to filter out noise
			keys[j].raw = (result[1][i*2] * 256) + result[1][i*2+1];
			
			if(keys[j].inverted) {
				keys[j].raw = 1023 - keys[j].raw;
			}
			
			if(calibrate[j]) {
				if(keys[j].raw < keys[j].minimum){
					keys[j].minimum = keys[j].raw;
				}
				if(keys[j].raw > keys[j].maximum){
					keys[j].maximum = keys[j].raw;
				}
				if(keys[j].maximum != keys[j].minimum) {
					keys[j].scale = 1.0 / (keys[j].maximum - keys[j].minimum);
				} else {
					keys[j].scale = 0.0;
				}
			}
			
			keys[j].continuous = (float)( keys[j].raw - keys[j].minimum ) * keys[j].scale;
			keys[j].continuous = CLAMP(keys[j].continuous, 0.0, 1.0);
			
			if(keys[j].continuous < keys[j].threshDown) {
				keys[j].binary = 0;
			} else if(keys[j].continuous > keys[j].threshUp){
				if(now - keys[j].lastTriggerTime > debounceTimeOut) {
					keys[j].binary = 1;
					keys[j].lastTriggerTime = now;
				}
			}

#ifdef FILTER_CHANGE
			if(keys[j].raw != keys[j].rawOld) {
				keys[j].changed = 1;
				
				if(keys[j].binary != keys[j].binaryOld) {
					keys[j].binaryOld = keys[j].binary;
					keys[j].binaryChanged = 1;
					// printf("right hand activated: key binary changed: keys[%d].binary is %d\n", j, keys[j].binary);

				} else {
					keys[j].binaryChanged = 0;
				}
				keys[j].rawOld = keys[j].raw;
			} else {
				keys[j].changed = 0;
			}
#endif
		}
		timestamp = (result[1][27] * 65536) + (result[1][28] * 256) + result[1][29];
	}
}

void threadedSerial::parseIMU()
{
	int i;
	if(haveResult[2]) {
		
		raw[0] = (result[2][0] * 256) + result[2][1];
		raw[1] = (result[2][2] * 256) + result[2][3];
		raw[2] = (result[2][4] * 256) + result[2][5];
		
		raw[3] = (result[2][6] * 256) + result[2][7];
		raw[4] = (result[2][8] * 256) + result[2][9];
		raw[5] = (result[2][10] * 256) + result[2][11];
		
		raw[6] = (result[2][12] * 256) + result[2][13];
		raw[7] = (result[2][14] * 256) + result[2][15];
		raw[8] = (result[2][16] * 256) + result[2][17];
		
		raw[9] = (result[2][18] * 256) + result[2][19];
		
		// accelerometer
		for(i = 0; i < 3; i++) {
			if( raw[i] >= 32768 ) {
				raw[i] -= 65535;
			}
			rawIMU[i] = raw[i] + accelOffset;
			IMU[i] = rawIMU[i] * accelScale;
			IMU[i] = CLAMP(IMU[i], 0.0 , 1.0);
		}
		
		// gyroscope
		for(i = 3; i < 6; i++) {
			if( raw[i] >= 32768 ) {
				raw[i] -= 65535;
			}
			rawIMU[i] = raw[i] + gyroOffset;
			IMU[i] = rawIMU[i] * gyroScale;
			IMU[i] = CLAMP(IMU[i], 0.0 , 1.0);
		}		
		
		// magnetometer
		for(i = 6; i < 9; i++) {
			if( raw[i] >= 32768 ) {
				raw[i] -= 65535;
			}
			rawIMU[i] = raw[i] + magnetoOffset;
			IMU[i] = rawIMU[i] * magnetoScale;
			IMU[i] = CLAMP(IMU[i], 0.0 , 1.0);
		}
		// temperature
		i = 9;
		if( raw[i] >= 32768 ) {
			raw[i] -= 65535;
		}
		// 280 LSB / degree
		// offset 13200 is 35 degree
		rawIMU[i] = raw[i] + 13200;
		
		IMU[i] = (rawIMU[i] / 280.) + 35.0;
		
		// calc the sums
		// TODO: calc abs-diff-sigma
		
		
		summedIMU[0] = ( fabs(IMU[0] - 0.5) + fabs(IMU[1] - 0.5) + fabs(IMU[2] - 0.5) ) * 0.6666666666666666666666666;
		summedIMU[1] = ( fabs(IMU[3] - 0.5) + fabs(IMU[4] - 0.5) + fabs(IMU[5] - 0.5) ) * 0.6666666666666666666666666;
		summedIMU[2] = ( fabs(IMU[6] - 0.5) + fabs(IMU[7] - 0.5) + fabs(IMU[8] - 0.5) ) * 0.6666666666666666666666666;
		
		calcHeadingTilt();
	}
}

void threadedSerial::parseAir()
{
	if(haveResult[3]) {
		airLong[0] = result[3][0] * 16777216 + result[3][1] * 65536 + result[3][2] * 256 + result[3][3];
		airLong[1] = result[3][4] * 16777216 + result[3][5] * 65536 + result[3][6] * 256 + result[3][7];
		
		air[0] = ((double)(airLong[0] / 100.0));	// + 1.0) * 0.5; 
		air[1] = ((double)(airLong[1] / 100.0));	// + 1.0) * 0.5; 
	}
}

void threadedSerial::calcKeycode()
{	
	int i;
	
//		if(haveResult[0] || haveResult[1]) {
	keycode = 0;
	for(i = 0; i < 25; i++) { 
		if(keys[i].binary == 1) {
			keycode += pow((double)2, (double)i);
		}
	}			

	if (keycode != keycodeOld) {
		keycodeOld = keycode;
		keycodeChanged = 1;
	}else{
		keycodeChanged = 0;
	}
	validMidiNote = false;
	midinote = -1;
	for(i = 0; i < 128; i++) {
		if(keycode == midiNote[i].keycode) {
			midinote = midiNote[i].note;
			validMidiNote = true;
			break;
		}
	}
//		}
}

void threadedSerial::calcHeadingTilt()
{
	double x, y, xx, yy;
	double ax, ay, bx, by;

	x = (IMU[1] - 0.5) * DEG_TO_RAD;
	y = (IMU[2] - 0.5) * DEG_TO_RAD;
	
	// old = old + (new - old) * factor;	
	x = headingOld_x + ((x - headingOld_x) * 0.2);
	y = headingOld_y + ((y - headingOld_y) * 0.2);
	
	headingOld_x = x;
	headingOld_y = y;
	
	ax = sin(x);	
	ay = sin(y);
	
	bx = tan(x);
	bx = bx * bx;
	
	by = tan(y);
	by = by * by;
	
	heading = atan2(ax, ay) * RAD_TO_DEG;
	tilt = atan( sqrt(bx + by) ) * RAD_TO_DEG * 10.0;

}

void threadedSerial::sendOSC()
{	
	// timestamps & keys
	if(haveResult[1] || haveResult[0]) { // right hand triggers sending
		
		systime = ofGetElapsedTimeMillis();
		systemTimestamp = systime - oldSystime;
		oldSystime = systime;
		
		m[25].clear();
		m[25].setAddress( systemaddress ); // timestamp
		m[25].addFloatArg( timestamp );
		m[25].addFloatArg( systemTimestamp );
		sender.sendMessage( m[25] );

		for(int i = 0; i < 25; i++) { // continuous key values
			if(keys[i].changed) {
				m[i].clear();
				m[i].setAddress( keys[i].oscaddress+"/continuous");
				m[i].addFloatArg( keys[i].continuous );
				sender.sendMessage( m[i] );
			}
		}
		for(int i = 0; i < 25; i++) { // binary key values
			if(keys[i].binaryChanged) {
				m[i].clear();
				m[i].setAddress( keys[i].oscaddress+"/down");
				m[i].addIntArg( keys[i].binary );
				sender.sendMessage( m[i] );
			}
		}	
		for(int i = 0; i < 25; i++) { // raw key values
			if(keys[i].changed) {
				m[i].clear();
				m[i].setAddress( keys[i].oscaddress+"/raw");
				m[i].addIntArg( keys[i].raw );
				sender.sendMessage( m[i] );
			}
		}	
		if(keycodeChanged) { // keycode
			m[0].clear();
			m[0].setAddress( keycodeaddress );
			m[0].addIntArg( keycode );
			sender.sendMessage( m[0] );
			keycodeChanged = false;
			// printf("sending keycode %d\n", keycode);
		}
		if(validMidiNote) {	
			m[0].clear();	// midinote derived from keycode
			m[0].setAddress( midinoteaddress );
			m[0].addIntArg( midinote );
			sender.sendMessage( m[0] );	
			validMidiNote = false;
		}
		for(int i = 0; i < 3; i++) { // buttons
			if(buttonChanged[i]) {
				m[i].clear();
				m[i].setAddress( buttonaddresses[2-i] );
				m[i].addIntArg( button[i] );
				sender.sendMessage( m[i] );	
			}
		}
		if(haveResult[0]) haveResult[0] = false;
		if(haveResult[1]) haveResult[1] = false;
	}
	//IMU
	if(haveResult[2]) {
		
		m[0].clear();
		m[0].setAddress( imuaddresses[0] ); // IMU accelero scaled
		m[0].addFloatArg( IMU[0] );
		m[0].addFloatArg( IMU[1] );
		m[0].addFloatArg( IMU[2] );
		sender.sendMessage( m[0] );	
		
		m[1].clear();
		m[1].setAddress( imuaddresses[1] ); // IMU gyro scaled
		m[1].addFloatArg( IMU[3] );
		m[1].addFloatArg( IMU[4] );
		m[1].addFloatArg( IMU[5] );
		sender.sendMessage( m[1] );	
		
		m[2].clear();
		m[2].setAddress( imuaddresses[2] ); // IMU magneto scaled
		m[2].addFloatArg( IMU[6] );
		m[2].addFloatArg( IMU[7] );
		m[2].addFloatArg( IMU[8] );
		sender.sendMessage( m[2] );	
		
		m[3].clear();
		m[3].setAddress( imuaddresses[3] ); // IMU accelero raw
		m[3].addFloatArg( rawIMU[0] );
		m[3].addFloatArg( rawIMU[1] );
		m[3].addFloatArg( rawIMU[2] );
		sender.sendMessage( m[3] );	
		
		m[4].clear();
		m[4].setAddress( imuaddresses[4] ); // IMU gyro raw
		m[4].addFloatArg( rawIMU[3] );
		m[4].addFloatArg( rawIMU[4] );
		m[4].addFloatArg( rawIMU[5] );
		sender.sendMessage( m[4] );	
		
		m[5].clear();
		m[5].setAddress( imuaddresses[5] ); // IMU magneto raw
		m[5].addFloatArg( rawIMU[6] );
		m[5].addFloatArg( rawIMU[7] );
		m[5].addFloatArg( rawIMU[8] );
		sender.sendMessage( m[5] );	
		
		m[6].clear();
		m[6].setAddress( imuaddresses[6] ); // IMU accelero summed
		m[6].addFloatArg( summedIMU[0] );
		sender.sendMessage( m[6] );	
		
		m[7].clear();
		m[7].setAddress( imuaddresses[7] ); // IMU gyro summed
		m[7].addFloatArg( summedIMU[1] );
		sender.sendMessage( m[7] );	
		
		m[8].clear();
		m[8].setAddress( imuaddresses[8] ); // IMU magneto summed
		m[8].addFloatArg( summedIMU[2] );
		sender.sendMessage( m[8] );
		
		m[9].clear();
		m[9].setAddress( imuaddresses[10] ); // IMU heading from accelerometer
		m[9].addFloatArg( heading );
		sender.sendMessage( m[9] );	
		
		m[10].clear();
		m[10].setAddress( imuaddresses[11] ); // IMU tilt from accelerometer
		m[10].addFloatArg( tilt );
		sender.sendMessage( m[10] );
		
		m[11].clear();
		m[11].setAddress( imuaddresses[9] ); // IMU temperature in degreee celsius 
		m[11].addFloatArg( IMU[9] );
		sender.sendMessage( m[11] );	

		haveResult[2] = false;
	}
	if(haveResult[3]) {
		m[11].clear();
		m[11].setAddress( airaddresses[0] ); // air pressure
		m[11].addFloatArg( air[0]);
		sender.sendMessage( m[11] );	
		
		m[12].clear();
		m[12].setAddress( airaddresses[1] ); // air temperature
		m[12].addFloatArg( air[1]);
		sender.sendMessage( m[12] );
		
		haveResult[3] = false;
	}
}

void threadedSerial::draw()
{
	
	int i;
	int anchorx = 12;
	int anchory = 64;
	int leftColumn = 156;
	int midColumn = 220;
	int rightColumn = 295;
	int farRightColumn = 404;
	
	int stepsize = 18;
	int columnwidth = 180;
	int width = 430;
	int height = 635;
	double yy;
	
	if( lock() )
	{
		if (status == 1) 
		{
			ofFill();
			ofSetColor(200, 200, 200, 255);
			ofRect(leftColumn-1, 79, width-leftColumn-5, height-anchory-15);
			
			for(i = 0; i < 35; i++) { // stripes
				if((i % 2) == 0){
					ofFill();
					ofSetColor(216, 216, 216, 255);
					ofRect(leftColumn-1, anchory+((i-1) * stepsize)+7, width-leftColumn-12, 16);
					ofSetColor(0, 0, 0, 255);
				}			
			}
			
			for(i = 0; i < 25; i++) { // keys
				ofSetColor(0, 0, 0, 255);
				yy = anchory+(i * stepsize);
				TTF.drawString(ofToString(keys[i].raw, 6), leftColumn, yy );
				TTF.drawString(ofToString(keys[i].continuous, 6), midColumn, yy);
				ofNoFill();
				ofSetColor(91, 91, 91, 255);
				ofRect(rightColumn, yy-9, 104, 12);
				ofFill();
				ofSetColor(0, 0, 0, 127);
				if(calibrate[i]) {
					ofSetColor(255, 127, 0, 191);					
					ofRect( rightColumn + (104 * keys[i].minimum*scale10), yy-7, (104 * (keys[i].maximum - keys[i].minimum) * scale10), 9);
					ofSetColor(0, 0, 0, 255);
					ofRect( rightColumn + (104 * (keys[i].raw*scale10)), yy-9, 2, 12);
					
				} else {
					ofRect( rightColumn + (104 * keys[i].continuous), yy-9, 2, 12);
					ofSetColor(91, 91, 91, 255);
					ofLine(rightColumn + (104 * keys[i].threshDown), yy-9, rightColumn + (104 * keys[i].threshDown), yy+4);
					ofLine(rightColumn + (104 * keys[i].threshUp), yy-9, rightColumn + (104 * keys[i].threshUp), yy+4);
				}
				// draw binary boxes
				ofNoFill();
				ofSetColor(91, 91, 91, 255);
				ofRect(farRightColumn, yy-9, 12, 12);
				
				if(keys[i].binary) {
					ofFill();
					ofSetColor(0, 0, 0, 255);
					ofRect(farRightColumn+2, yy-6, 7, 7);
				}
	
			}
			for(i = 0; i < 9; i++) { // imu
				ofSetColor(0, 0, 0, 255);
				yy = anchory+((i+25) * stepsize);
				TTF.drawString( ofToString(raw[i], 6) , leftColumn, yy );
				TTF.drawString(ofToString(IMU[i], 6), midColumn, yy);
				ofNoFill();
				ofSetColor(91, 91,91, 255);
				ofRect(rightColumn, yy-9, 104, 12);
				ofFill();
				ofSetColor(0, 0, 0, 255);
				ofRect( rightColumn + (104 * IMU[i]), yy-9, 2, 12);
				ofNoFill();
				ofSetColor(91, 91, 91, 255);
				ofLine(rightColumn+52, yy-9, rightColumn+52, yy+4);
			}	
			// air 
			ofSetColor(0, 0, 0, 255);
			yy = anchory+(34 * stepsize);
			TTF.drawString(ofToString(airLong[0], 1), leftColumn, yy );
			TTF.drawString(ofToString(air[0], 2), midColumn, yy);
			
			ofNoFill();
			ofSetColor(91, 91, 91, 255);
			ofRect(rightColumn, yy-9, 104, 12);
			ofFill();
			ofSetColor(0, 0, 0, 127);
			ofRect( rightColumn + (104 * (CLAMP( ((air[0] - 500.0) * 0.001), 0, 1))), yy-9, 2, 12);
			
			// buttons
			ofSetColor(0, 0, 0, 255);
			yy = anchory+((35) * stepsize);
			TTF.drawString( ofToString(button[2], 1), leftColumn, yy );
			TTF.drawString( ofToString(button[1], 1), leftColumn+12, yy );
			TTF.drawString( ofToString(button[0], 1), leftColumn+24, yy );
			
			ofNoFill();
			ofSetColor(91, 91, 91, 255);
			ofRect(midColumn, yy-9, 12, 12);			
			ofRect(midColumn+14, yy-9, 12, 12);
			ofRect(midColumn+28, yy-9, 12, 12);
			
			ofFill();
			ofSetColor(0, 0, 0, 255);
			if(button[2]) {
				ofRect(midColumn+2, yy-6, 7, 7);
			}				
			if(button[1]) {
				ofRect(midColumn+16, yy-6, 7, 7);
			}
			if(button[0]) {
				ofRect(midColumn+30, yy-6, 7, 7);
			}
			if(calibrateAll) {
				ofFill();
				ofSetColor(255, 127, 0);
				ofRect(rightColumn, 690, 124, 20);
				ofNoFill();
				ofSetColor(127, 127, 127);
				ofRect(rightColumn, 690, 124, 20);
				ofSetColor(0, 0, 0);
				TTF.drawString("Calibrating...", rightColumn+28, 704);
			}
			ofFill();
			ofSetColor(232, 232, 232);
			ofRect(rightColumn, 23, 124, 20);
			ofNoFill();
			ofSetColor(127, 127, 127);
			ofRect(rightColumn, 23, 124, 20);
			ofSetColor(0, 0, 0);		
			TTF.drawString("Hide Values", rightColumn+32, 38);
		}
		
		
		unlock();
	}else{
		//			str = "can't lock!\neither an error\nor the thread has stopped";
		//			ofDrawBitmapString(str, 50, 56);
	}
}
