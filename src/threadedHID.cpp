/*
 *  threadedHID.cpp
 *
 *  Copyright © 2014 Zurich University of the Arts. All Rights Reserved.
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
 *  @@date 20140727
 *
 */

#include "threadedHID.h"

threadedHID::threadedHID()
{
    numImuAddr = 24;
//	TTF.loadFont("inconsolata.ttf", 11, 1, 1, 0);
	
	streamSize[0] = PATTERNLENGTH_1;
    streamSize[1] = PATTERNLENGTH_2;
    streamSize[2] = PATTERNLENGTH_3;
    
	scale10 = 1.0 / 1024.0;
	scale11 = 1.0 / 2048.0;
	scale12 = 1.0 / 4096.0;
	scale13 = 1.0 / 8192.0;
	scale16 = 1.0 / 65536.0;
	//	scale32 = 1.0 / 2147483648;
	
    // v3.4 comm structure
	//	accelResolution = 4;
	//	accelOffset = 1024;
	//	accelScale = scale11;
    // v3.5 comm structure
	accelResolution = 4;
	accelOffset = 32768;
	accelScale = scale16;
	
	gyroResolution = 16;
	gyroOffset = 32768;
	gyroScale = scale16;
	
	magnetoResolution = 12;
	magnetoOffset = 2048;
	magnetoScale = scale12;
	
	tempOffset = 32768;
	tempScale = scale16;
	
	headingLowpassFactor = 0.2;
	headingOld_x = headingOld_y = 0.0;
	
	TTF.loadFont("lucidagrande.ttf", 8, 1, 1, 0);
	calibrateSwitch = 0;
	for(int i = 0; i < MAXNUM; i++) {
		calibrate[i] = 0;
	}
    
    batteryAddressMain = "/sabre/battery/main";
    batteryAddressAir = "/sabre/battery/air";
    
    timestampAddressLeft = "/sabre/timestamp/left";
    timestampAddressRight = "/sabre/timestamp/right";
    timestampAddressAir = "/sabre/timestamp/air";
    
    linkQualityAddressLeft = "/sabre/linkquality/left";
    linkQualityAddressRight = "/sabre/linkquality/right";
    linkQualityAddressAir = "/sabre/linkquality/air";
    
    airValue.calibrationFlag = 0;
    airValue.calibrationCounter = 0;
    airValue.calibrationValue = 0.0;
    
    raw[0] = 0;
    IMU[0] = 0.0f;
    raw[1] = 0;
    IMU[1] = 0.0f;
    raw[2] = 0;
    IMU[2] = 0.0f;
    
    raw[3] = 0;
    IMU[3] = 0.0f;
    raw[4] = 0;
    IMU[4] = 0.0f;
    raw[5] = 0;
    IMU[5] = 0.0f;
    
    raw[6] = 0;
    IMU[6] = 2147483648.0f;
    raw[7] = 0;
    IMU[7] = 2147483648.0f;
    raw[8] = 0;
    IMU[8] = 2147483648.0f;
    
    raw[9] = 0;
    IMU[9] = 0.0f;
    
    
    airLong[0] = 0;
    air[0] = 1000.0;
    airValue.continuous = 0.0f;
    
    button[0] = 0;
    button[1] = 0;
    button[2] = 0;
    
    batteryLevelRight = 0;
    batteryLevelAir = 0;
    
    linkQualityLeft = 0;
    linkQualityRight = 0;
    linkQualityAir = 0;
    
    airValue.calibrationFlag = true;
    airValue.calibrationValue = 0;
    airValue.calibrationCounter = 0;
    
    OSCprevTime = ofGetElapsedTimeMillis();
    
    keycode = 0;
    midinote = 67;
    validMidiNote = true;
    OSCcounter = 0;
    
    senderActive[0] = true;
    sendFullFrame = false; // TODO
    
//    dBuffer.clear();

}

threadedHID::~threadedHID()
{
	if( isThreadRunning() ) { 
		stopThread();
	}
    if(status){
        rawHID.closeDevice();
    }
}

void threadedHID::start()
{
	startThread(true);   // blocking
    oldSystimeL = oldSystimeR = ofGetElapsedTimeMicros();
}

void threadedHID::stop()
{
	stopThread();
}

//--------------------------
void threadedHID::threadedFunction() 
{

//	while (true) { // infinite loop
	while( isThreadRunning() != 0 )
	{
        // lock();

		haveInput[0] = 0;
		haveInput[1] = 0;
		haveInput[2] = 0;

        int num = rawHID.receivePacket();
		
		if (num < 0) {
			
			printf("\nerror reading, device went offline\n");
			
			rawhid_close(0);
			unplugged = true;
			return 0;
		}
		if (num > 0) {

			HIDparse();
            
            OSCtime = ofGetElapsedTimeMillis();
            if(OSCtime >= (OSCprevTime + OSCsendingInterval) ) {
                
                for(int i = 0; i < NUMOSCSENDERS; i++) {
                    if(senderActive[i]) {
                        if(i == resetID){
                            sendOSC( i, true );
                        }else{
                            sendOSC( i, false );
                        }
                    }
                }
                OSCprevTime = OSCtime;
            }
		}
		usleep(1000);
//		unlock();
   	}
}


void threadedHID::HIDparse()
{
	int i, j;
	long sum;
	char tempBuf[20];
	
//	dParserStartT = ofGetElapsedTimeMicros();	
//	dParserStopT = ofGetElapsedTimeMicros();
//	dParserSum += (dParserStopT - dParserStartT);
	
	// pattern matching
	if ( rawHID.buf[0] == 65) { // packet start marker
		
		if ( rawHID.buf[1] == 240) {	// left hand packet
			
			if(rawHID.buf[22] == 90) {
				
				for(i = 0; i < 20; i++) { // collect n-2 bytes into buffer
					input[0][i] = rawHID.buf[i+2];
				}
				
//				printf("LEFT - serial parsing time: %lld us\n", dParserSum);
//				dBuffer.append("LEFT; ");
//				sprintf(tempBuf, "%lld", dParserSum);
//				dBuffer.append(tempBuf);
//				dBuffer.append("; ");
//				dParserSum = 0;
//				dLhStartT = ofGetElapsedTimeMicros();
//				dLhParsing = true;
				
				haveInput[0] = true;
				parseLeft();
				calcKeycode();
			}

		} else if( rawHID.buf[1] == 241) { // right hand packet
			
			if( rawHID.buf[41] == 90) {
				
				for(i = 0; i < 39; i++) { // collect n-2 bytes into buffer
					input[1][i] = rawHID.buf[i+2];
				}
				
//				printf("RIGHT - serial parsing time: %lld us\n", dParserSum);
//				dBuffer.append("RIGHT; ");
//				sprintf(tempBuf, "%lld", dParserSum);
//				dBuffer.append(tempBuf);
//				dBuffer.append("; ");
//				dParserSum = 0;
//				dRhStartT = ofGetElapsedTimeMicros();
//				dRhParsing = true;
				
				haveInput[1] = true;
				parseRight();
				calcKeycode();
				parseIMU();
			}
			
		} else if( rawHID.buf[1] == 242) { // AirMems packet
			
			if( rawHID.buf[14] == 90) {
				
				for(i = 0; i < 12; i++) { // collect n-2 bytes into buffer
					input[2][i] = rawHID.buf[i+2];
				}
				
//				printf("AM - serial parsing time: %lld us\n", dParserSum);
//				dBuffer.append("airMEMS; ");
//				sprintf(tempBuf, "%lld", dParserSum);
//				dBuffer.append(tempBuf);
//				dBuffer.append("; ");
//				dParserSum = 0;
//				dAmStartT = ofGetElapsedTimeMicros();
//				dAmParsing = true;
				
				haveInput[2] = true;
				parseAir();
			}
		}
	}
}

void threadedHID::parseLeft()
{
	int temp;
	int i, j;
	
	if(haveInput[0]) {
		
        // parse out the 13 keys
        keys[0].raw = input[0][0]; // the LSBs
		keys[1].raw = input[0][1];
		keys[2].raw = input[0][2];
		keys[3].raw = input[0][3];
        
		keys[4].raw = input[0][4];
		keys[5].raw = input[0][5];
		keys[6].raw = input[0][6];
		keys[7].raw = input[0][7];
        
		keys[8].raw = input[0][8];
        keys[9].raw = input[0][9];
        keys[10].raw = input[0][10];
        keys[11].raw = input[0][11];
        
        keys[12].raw = input[0][12];
        
		keys[0].raw += (input[0][13] & 0xC0) << 2;
		keys[1].raw += (input[0][13] & 0x30) << 4;
		keys[2].raw += (input[0][13] & 0xC) << 6;
        keys[3].raw += (input[0][13] & 0x3) << 8;
        
        keys[4].raw += (input[0][14] & 0xC0) << 2;
		keys[5].raw += (input[0][14] & 0x30) << 4;
		keys[6].raw += (input[0][14] & 0xC) << 6;
        keys[7].raw += (input[0][14] & 0x3) << 8;
        
        keys[8].raw += (input[0][15] & 0xC0) << 2;
		keys[9].raw += (input[0][15] & 0x30) << 4;
		keys[10].raw += (input[0][15] & 0xC) << 6;
        keys[11].raw += (input[0][15] & 0x3) << 8;
        
        keys[12].raw += (input[0][16] & 0xC0) << 2;
		
        int now = ofGetElapsedTimeMillis();
		for (int i = 0; i < 13; i++) { // 13 keys
            
			if(keys[i].inverted) {
				keys[i].raw = 1023 - keys[i].raw;
			}
			
			if(calibrate[i]) {
				if(keys[i].raw < keys[i].minimum){
					//					keys[i].minimum = keys[i].raw;
					keys[i].minimum = (keys[i].raw + CALIBRATEOFFSET);
				}
				if(keys[i].raw > keys[i].maximum){
					//					keys[i].maximum = keys[i].raw;
					keys[i].maximum = (keys[i].raw - CALIBRATEOFFSET);
				}
				if(keys[i].maximum != keys[i].minimum) {
					keys[i].scale = 1.0 / (keys[i].maximum - keys[i].minimum);
				} else {
					keys[i].scale = 0.0;
				}
			}
			
			keys[i].continuous = (float)( keys[i].raw - keys[i].minimum ) * keys[i].scale;
			keys[i].continuous = CLAMP(keys[i].continuous, 0.0, 1.0);
			
			if(keys[i].continuous < keys[i].threshDown) {
				keys[i].binary = false;
			} else if(keys[i].continuous > keys[i].threshUp) {
				if(now - keys[i].lastTriggerTime > debounceTimeOut) {
					keys[i].binary = true;
					keys[i].lastTriggerTime = now;
				}
			}
			
#ifdef FILTER_CHANGE
			if(keys[i].raw != keys[i].rawOld) {
				keys[i].changed = true;
				
				if(keys[i].binary != keys[i].binaryOld) {
					keys[i].binaryChanged = true;
					keys[i].binaryOld = keys[i].binary;
				} else {
					//					keys[i].binaryChanged = false;
				}
				keys[i].rawOld = keys[i].raw;
			} else {
				//				keys[i].changed = false;
			}
#endif
		}
        
        // !! INSTRUMENT #1 (Graz) !!
		button[1] = (input[0][16] & 0x8) >> 3;
		button[0] = (input[0][16] & 0x10) >> 4;
        
        // !! INSTRUMENT #2 (Matthias) !!
		//        button[0] = (input[0][16] & 0x8) >> 3;
		//		button[1] = (input[0][16] & 0x10) >> 4;
        
        // common...
		button[2] = (input[0][16] & 0x20) >> 5;
		
		for(int i = 0; i < 3; i++) {
			if(button[i] != buttonOld[i]) {
				buttonChanged[i] = true;
				buttonOld[i] = button[i];
			} else {
				//				buttonChanged[i] = false;
			}
		}
        
        timestampLeft = input[0][17] + (input[0][18] << 8);
        linkQualityLeft = input[0][19];
        
        systimeL = ofGetElapsedTimeMillis();
        deltaTimeL = systimeL - oldSystimeL;
        oldSystimeL = systimeL;
		
	} // end haveInput check
}

void threadedHID::parseRight()
{
	int temp;
	int i;
	
	if(haveInput[1]) {
        
        keys[13].raw = input[1][0]; // the LSBs
		keys[14].raw = input[1][1];
		keys[15].raw = input[1][2];
		keys[16].raw = input[1][3];
        
		keys[17].raw = input[1][4];
		keys[18].raw = input[1][5];
		keys[19].raw = input[1][6];
		keys[20].raw = input[1][7];
        
		keys[21].raw = input[1][8];
        keys[22].raw = input[1][9];
        keys[23].raw = input[1][10];
        keys[24].raw = input[1][11];
        
		keys[13].raw += ((input[1][12] & 0xC0) << 2);
		keys[14].raw += ((input[1][12] & 0x30) << 4);
		keys[15].raw += ((input[1][12] & 0xC) << 6);
        keys[16].raw += ((input[1][12] & 0x3) << 8);
        
        keys[17].raw += ((input[1][13] & 0xC0) << 2);
		keys[18].raw += ((input[1][13] & 0x30) << 4);
		keys[19].raw += ((input[1][13] & 0xC) << 6);
        keys[20].raw += ((input[1][13] & 0x3) << 8);
        
        keys[21].raw += ((input[1][14] & 0xC0) << 2);
		keys[22].raw += ((input[1][14] & 0x30) << 4);
		keys[23].raw += ((input[1][14] & 0xC) << 6);
        keys[24].raw += ((input[1][14] & 0x3) << 8);
        
        int now = ofGetElapsedTimeMillis();
		for (int i = 13; i < 25; i++) { // 12 keys
			
			if(keys[i].inverted) {
				keys[i].raw = 1023 - keys[i].raw;
			}
			
			if(calibrate[i]) {
				if(keys[i].raw < keys[i].minimum){
					//					keys[i].minimum = keys[i].raw;
					keys[i].minimum = (keys[i].raw + CALIBRATEOFFSET);
				}
				if(keys[i].raw > keys[i].maximum){
					//					keys[i].maximum = keys[i].raw;
					keys[i].maximum = (keys[i].raw - CALIBRATEOFFSET);
				}
				if(keys[i].maximum != keys[i].minimum) {
					keys[i].scale = 1.0 / (keys[i].maximum - keys[i].minimum);
				} else {
					keys[i].scale = 0.0;
				}
			}
			
			keys[i].continuous = (float)( keys[i].raw - keys[i].minimum ) * keys[i].scale;
			keys[i].continuous = CLAMP(keys[i].continuous, 0.0, 1.0);
			
			if(keys[i].continuous < keys[i].threshDown) {
				keys[i].binary = false;
			} else if(keys[i].continuous > keys[i].threshUp){
				if(now - keys[i].lastTriggerTime > debounceTimeOut){
					keys[i].binary = true;
					keys[i].lastTriggerTime = now;
				}
			}
			
#ifdef FILTER_CHANGE
			if(keys[i].raw != keys[i].rawOld) {
				keys[i].changed = true;
				if(keys[i].binary != keys[i].binaryOld) {
					keys[i].binaryChanged = true;
                    keys[i].binaryOld = keys[i].binary;
				} else {
					//					keys[i].binaryChanged = false;
				}
				keys[i].rawOld = keys[i].raw;
			} else {
				//				keys[i].changed = false;
			}
#endif
		}
        // IMU parsing is done in separate function() using same input buffer
        
        // v3.5 comm structure
        batteryLevelRight = input[1][35] & 0xF;
		//        printf("batteryLevelRight = %x\n", batteryLevelRight);
        timestampRight = input[1][36] + (input[1][37] << 8);
        linkQualityRight  = input[1][38];
		
		//        // v3.4 comm structure
		//        batteryLevelRight = input[1][33] & 0xF;
		//        timestampRight = input[1][34] + (input[1][35] << 8);
		//        linkQualityRight  = input[1][36];
		
        systimeR = ofGetElapsedTimeMillis();
        deltaTimeR = systimeR - oldSystimeR;
        oldSystimeR = systimeR;
		
	}
}

void threadedHID::parseIMU()
{
	int i;
	if(haveInput[1]) {
		
		// v3.5 comm structure
		raw[0] = input[1][29] + (input[1][32] << 8); // accelerometer
		raw[1] = input[1][30] + (input[1][33] << 8);
		raw[2] = input[1][31] + (input[1][34] << 8);
		
		//        // v3.4 comm structure
		//		raw[0] = input[1][29] + ((input[1][32] & 0xE0) << 3); // accelerometer
		//		raw[1] = input[1][30] + ((input[1][32] & 0x1C) << 6);
		//		raw[2] = input[1][31] + ((input[1][33] & 0xE0) << 3);
        
		
		raw[3] = input[1][16] + (input[1][20] << 8); // gyroscope
		raw[4] = input[1][17] + (input[1][21] << 8);
		raw[5] = input[1][18] + (input[1][22] << 8);
		
		raw[9] = input[1][15] + (input[1][19] << 8); // temperature
		
		raw[6] = input[1][23] + (input[1][26] << 8); // compass / magneto
		raw[7] = input[1][24] + (input[1][27] << 8);
		raw[8] = input[1][25] + (input[1][28] << 8);
		
		
		// accelerometer
		for(i = 0; i < 3; i++) {
            // v3.4 comm structure
			//			if( raw[i] >= 1024 ) {
			//				raw[i] -= 2047;
			//			}
            // v3.5 comm structure
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
		summedIMU[0] = ( fabs(IMU[0] - 0.5) + fabs(IMU[1] - 0.5) + fabs(IMU[2] - 0.5) ) * 0.6666666666666666666666666;
		summedIMU[1] = ( fabs(IMU[3] - 0.5) + fabs(IMU[4] - 0.5) + fabs(IMU[5] - 0.5) ) * 0.6666666666666666666666666;
		summedIMU[2] = ( fabs(IMU[6] - 0.5) + fabs(IMU[7] - 0.5) + fabs(IMU[8] - 0.5) ) * 0.6666666666666666666666666;
		
		calcHeadingTilt();
	}
}

void threadedHID::parseAir()
{
	if(haveInput[2]) {
		airLong[0] = input[2][0] + (input[2][1] << 8) + (input[2][2] << 16) + (input[2][3] << 24) ; // pressure
		airLong[1] = input[2][4] + (input[2][5] << 8) + (input[2][6] << 16) + (input[2][7] << 24) ; // temperature
		
		air[0] = ((double)(airLong[0] / 100.0));    // + 1.0) * 0.5; // pressure
		air[1] = ((double)(airLong[1] / 100.0));	// + 1.0) * 0.5; // temprature
        
        batteryLevelAir = input[2][8] & 0xF;
        timestampAir = input[2][9] + (input[2][10] << 8);
        linkQualityAir = input[2][11];
        
        //collect atmospheric pressure value index 50-300 at each startup to have atmo-pressure offset
        if(airValue.calibrationFlag) {
            if(airValue.calibrationCounter == 0) {
                ofLog(OF_LOG_VERBOSE, "start calibrating atmospheric pressure after %lld ms\n", ofGetElapsedTimeMillis());
            }
            if(airValue.calibrationCounter < 5) {
                airValue.offset = air[0];
            }else {
                if(airValue.calibrationCounter < 25) {
                    airValue.calibrationValue += air[0];
					//                    printf("air[0] calib %f\n", air[0]);
                }
                if(airValue.calibrationCounter >= 25){
                    airValue.offset = airValue.calibrationValue / (airValue.calibrationCounter - 5);
                    airValue.calibrationFlag = false; // lock up after you
                    ofLog(OF_LOG_VERBOSE, "airValue.offset after calibration is %f\n", airValue.offset);
                    ofLog(OF_LOG_VERBOSE, "finished calibrating atmospheric pressure after %lld ms\n", ofGetElapsedTimeMillis());
                }
            }
            airValue.calibrationCounter++;
        }
        
        airValue.relative = air[0] - airValue.offset;
        
        if(airValue.calibratePressureRange) {
            if(airValue.relative < airValue.minimum){
                airValue.minimum = airValue.relative;
            }
            if(airValue.relative > airValue.maximum){
                airValue.maximum = airValue.relative;
            }
            if(airValue.maximum > abs(airValue.minimum)) {
                airValue.scale = ( 1.0 / airValue.maximum) * 0.5;
            } else if(airValue.maximum < abs(airValue.minimum) ){
				airValue.scale = ( 1.0 / abs(airValue.minimum)) * 0.5;
            }else{
                airValue.scale = 0.0;
            }
        }
        
        airValue.continuous = airValue.relative * airValue.scale + 0.5;
        airValue.continuous = CLAMP( airValue.continuous, 0.0, 1.0);
	}
    
    if(fullspeedOSC == false){
        haveInput[2] = false;
    }
}

void threadedHID::calcKeycode()
{
	int i;
	
	keycode = 0;
	for(i = 0; i < 25; i++) {
		if(keys[i].binary == 1) {
			keycode += pow((double)2, (double)i);
		}
	}
	
	if (keycode != keycodeOld) {
		keycodeOld = keycode;
		keycodeChanged = true;
        
        validMidiNote = false;
        midinote = -1;
        for(i = 0; i < 128; i++) {
            if(keycode == midiNote[i].keycode) {
                midinote = midiNote[i].note;
                validMidiNote = true;
                break;
            }
        }
		
	}else{
		//		keycodeChanged = false;
	}
}

void threadedHID::calcHeadingTilt()
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
	tilt = CLAMP( (atan( sqrt(bx + by) ) * RAD_TO_DEG * 8.0), 0.0, 1.0);
	//	tilt = atan( sqrt(bx + by) ) * RAD_TO_DEG * 10.0;
}

void threadedHID::sendOSC(int ID, bool resetFlags)
{
    int i, j;
    char tempBuf[20];
    
    // The standard mode
    
    if( (senderMode[ID] & 1) == 1) {
		// Keys ------------------------
		
        systemTimestamp = ofGetElapsedTimeMillis();
		
        if(sendFullFrame) {
            m[0].clear();
            m[0].setAddress( "/sabre/dataframe" ); // timestamp server
            m[0].addStringArg( "begin" );
            sender[ID].sendMessage( m[0] );
        }
		
        m[1].clear();
        m[1].setAddress( timestampAddressServer ); // timestamp server
        m[1].addIntArg( deltaTimeL );
        m[1].addIntArg( deltaTimeR );
        m[1].addIntArg( systemTimestamp );
        
        m[2].clear();
        m[2].setAddress( timestampAddressLeft ); // timestamp Left
        m[2].addIntArg( timestampLeft );
		
        m[3].clear();
        m[3].setAddress( timestampAddressRight ); // timestamp Right
        m[3].addIntArg( timestampRight );
        
        m[4].clear();
        m[4].setAddress( timestampAddressAir ); // timestamp
        m[4].addIntArg( timestampAir );
        
        m[5].clear();
        m[5].setAddress( airaddresses[0] ); // air pressure
        m[5].addFloatArg( airValue.continuous);
        
        
        m[6].clear();
        m[6].setAddress( airaddresses[1] ); // air temperature
        m[6].addFloatArg( air[1]);
        
        // send first batch
        
        for(i = 1; i < 7; i++) {
            sender[ID].sendMessage( m[i] );
        }
		
        if(keycodeChanged) { // keycode
            m[7].clear();
            m[7].setAddress( keycodeaddress );
            m[7].addIntArg( keycode );
            
            sender[ID].sendMessage( m[7] );
            if(resetFlags) {
                keycodeChanged = false;
            }
            if(validMidiNote) {
                m[8].clear();	// midinote derived from keycode
                m[8].setAddress( midinoteaddress );
                m[8].addIntArg( midinote );
                
                sender[ID].sendMessage( m[8] );
            }
        }
        
        for(i = 9, j = 2; i < 12; i++, j--) { // buttons
            if(buttonChanged[j]) {
                m[i].clear();
                m[i].setAddress( buttonaddresses[j] );
                m[i].addIntArg( button[j] );
                if(resetFlags) {
                    buttonChanged[j] = false;
                }
                sender[ID].sendMessage( m[i] );
            }
        }
        
        for(i = 12, j = 0; i < 37; i++, j++) { // binary key values
            if(keys[j].binaryChanged) {
                m[i].clear();
                m[i].setAddress( keys[j].oscaddress+"/down");
                m[i].addIntArg( keys[j].binary );
                if(resetFlags) {
                    keys[j].binaryChanged = false;
                }
                sender[ID].sendMessage( m[i] );
                
            }
        }
        
        for(i = 37, j = 0; i < 62; i++, j++) { // continuous key values
            if(keys[j].changed) {
                m[i].clear();
                m[i].setAddress( keys[j].oscaddress+"/continuous");
                m[i].addFloatArg( keys[j].continuous );
                if(!sendRawValues && resetFlags) {
                    keys[j].changed = false;
                }
                
                sender[ID].sendMessage( m[i] );
            }
        }
        
        if(sendRawValues) {
            for(i = 62, j = 0; i < 87; i++, j++) { // raw key values
                if(keys[j].changed) {
                    m[i].clear();
                    m[i].setAddress( keys[j].oscaddress+"/raw");
                    m[i].addIntArg( keys[j].raw );
                    if(resetFlags) {
                        keys[j].changed = false;
                    }
                    sender[ID].sendMessage( m[i] );
                }
            }
        }
		
		// IMU ------------------------
		
        m[87].clear();
        m[87].setAddress( imuaddresses[0] ); // IMU accelero scaled
        m[87].addFloatArg( IMU[0] );
        m[87].addFloatArg( IMU[1] );
        m[87].addFloatArg( IMU[2] );
        
        m[88].clear();
        m[88].setAddress( imuaddresses[1] ); // IMU gyro scaled
        m[88].addFloatArg( IMU[3] );
        m[88].addFloatArg( IMU[4] );
        m[88].addFloatArg( IMU[5] );
        
        m[89].clear();
        m[89].setAddress( imuaddresses[2] ); // IMU magneto scaled
        m[89].addFloatArg( IMU[6] );
        m[89].addFloatArg( IMU[7] );
        m[89].addFloatArg( IMU[8] );
        
        m[90].clear();
        m[90].setAddress( imuaddresses[6] ); // IMU accelero summed
        m[90].addFloatArg( summedIMU[0] );
        
        m[91].clear();
        m[91].setAddress( imuaddresses[7] ); // IMU gyro summed
        m[91].addFloatArg( summedIMU[1] );
        
        m[92].clear();
        m[92].setAddress( imuaddresses[8] ); // IMU magneto summed
        m[92].addFloatArg( summedIMU[2] );
        
        m[93].clear();
        m[93].setAddress( imuaddresses[10] ); // IMU heading from accelerometer
        m[93].addFloatArg( heading );
        
        m[94].clear();
        m[94].setAddress( imuaddresses[11] ); // IMU tilt from accelerometer
        m[94].addFloatArg( tilt );
        
        m[95].clear();
        m[95].setAddress( imuaddresses[9] ); // IMU temperature in degreee celsius
        m[95].addFloatArg( IMU[9] );
        
        for(i = 87; i < 96; i++) {
            sender[ID].sendMessage( m[i] );
        }
        
        if(sendRawValues) {
            m[96].clear();
            m[96].setAddress( imuaddresses[3] ); // IMU accelero raw
            m[96].addFloatArg( rawIMU[0] );
            m[96].addFloatArg( rawIMU[1] );
            m[96].addFloatArg( rawIMU[2] );
            
            m[97].clear();
            m[97].setAddress( imuaddresses[4] ); // IMU gyro raw
            m[97].addFloatArg( rawIMU[3] );
            m[97].addFloatArg( rawIMU[4] );
            m[97].addFloatArg( rawIMU[5] );
            
            m[98].clear();
            m[98].setAddress( imuaddresses[5] ); // IMU magneto raw
            m[98].addFloatArg( rawIMU[6] );
            m[98].addFloatArg( rawIMU[7] );
            m[98].addFloatArg( rawIMU[8] );
            
            for(i = 96; i < 99; i++) {
                sender[ID].sendMessage( m[i] );
            }
        }
        
        m[99].clear();
        m[99].setAddress( batteryAddressAir ); // air battery
        m[99].addIntArg( batteryLevelAir);
        
        m[100].clear();
        m[100].setAddress( linkQualityAddressAir ); // air link quality
        m[100].addIntArg( linkQualityAir);
		
        m[101].clear();
        m[101].setAddress( linkQualityAddressLeft ); // left link quality
        m[101].addIntArg( linkQualityLeft );
        
        m[102].clear();
        m[102].setAddress( linkQualityAddressRight ); // right link quality
        m[102].addIntArg( linkQualityRight );
        
        m[103].clear();
        m[103].setAddress( batteryAddressMain ); // battery level main
        m[103].addIntArg( batteryLevelRight );
        
        for(i = 99; i < 104; i++) {
            sender[ID].sendMessage( m[i] );
        }
        
        if(sendFullFrame) {
            m[104].clear();
            m[104].setAddress( "/sabre/dataframe" ); // timestamp server
            m[104].addStringArg( "end" );
            sender[ID].sendMessage( m[104] );
        }
    }
    
    // Gerhard Eckels direct SC-server Mode
    if( ((senderMode[ID] >> 4) & 1) == 1){
        
        m[0].clear();
        m[0].setAddress( "/c_setn" ); // SC server command
        m[0].addIntArg( 0 ); // SC server Bus Nr.
        m[0].addIntArg( 26 ); // SC server NumArgs
		
        if(sendRawValues == 0){
            m[0].addFloatArg( airValue.continuous);
            for(i = 0; i < 25; i++){
                m[0].addFloatArg( keys[i].continuous );
            }
        }else if(sendRawValues == 1){
            m[0].addFloatArg( airValue.relative);
            for(i = 0; i < 25; i++){
                m[0].addIntArg( keys[i].raw );
            }
        }
        sender[ID].sendMessage( m[0] );
    }
    /*
    if(dLhParsing) {
        dLhStopT = ofGetElapsedTimeMicros();
        printf("LH time: %01.03f ms\n\n", ((float)(dLhStopT - dLhStartT)/1000));
        sprintf(tempBuf, "%lld", (dLhStopT-dLhStartT));
        dBuffer.append(tempBuf);
        dBuffer.append(";\n");
        dLhStartT = 0;
        dLhStopT = 0;
        dLhParsing = false;
    }
    if(dRhParsing) {
        dRhStopT = ofGetElapsedTimeMicros();
        printf("RH time: %01.03f ms\n\n", ((float)(dRhStopT - dRhStartT)/1000));
        sprintf(tempBuf, "%lld", (dRhStopT-dRhStartT));
        dBuffer.append(tempBuf);
        dBuffer.append(";\n");
        dRhStartT = 0;
        dRhStopT = 0;
        dRhParsing = false;
    }
    if(dAmParsing) {
        dAmStopT = ofGetElapsedTimeMicros();
        printf("aM time: %01.03f ms\n\n", ((float)(dAmStopT - dAmStartT)/1000));
        sprintf(tempBuf, "%lld", (dAmStopT-dAmStartT));
        dBuffer.append(tempBuf);
        dBuffer.append(";\n");
        dAmStartT = 0;
        dAmStopT = 0;
        dAmParsing = false;
    }
    */
    return;
}

void threadedHID::calcResetID()
{
    resetID = -1;
    for(int i = 0; i < NUMOSCSENDERS; i++) {
        if(senderActive[i] == 1){
            resetID = i;
        }
    }
    printf("resetID is %d\n", resetID);
}

