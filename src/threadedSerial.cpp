/*
 *  threadedSerial.cpp
 * 
 *  SABRe-server
 *  © 2012 ICST / ZHdK  
 *
 *  @author Jan Schacher
 *  @date 20121030
 *
 */

#include "threadedSerial.h"

threadedSerial::threadedSerial()
{
	int i;
	
    streamSize[0] = PATTERNLENGTH_1;
    streamSize[1] = PATTERNLENGTH_2;
    streamSize[2] = PATTERNLENGTH_3;
    
	scale10 = 1.0 / 1024.0;
	scale11 = 1.0 / 2048.0;
	scale12 = 1.0 / 4096.0;
	scale13 = 1.0 / 8192.0;
	scale16 = 1.0 / 65536.0;
	//	scale32 = 1.0 / 2147483648;
	
	accelResolution = 4;
	accelOffset = 1024;
	accelScale = scale11;
	
	gyroResolution = 16;
	gyroOffset = 32768;
	gyroScale = scale16;
	
	magnetoResolution = 8;
	magnetoOffset = 2047;
	magnetoScale = scale12;
	
	tempOffset = 32768;
	tempScale = scale16;
	
	headingLowpassFactor = 0.2;
	headingOld_x = headingOld_y = 0.0;
	
	TTF.loadFont("lucidagrande.ttf", 8, 1, 1, 0);
	calibrateAll = 0;
	for(i = 0; i < MAXNUM; i++) {
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
    
    raw[6] = 1;
    IMU[6] = 1.0f;
    raw[7] = 1;
    IMU[7] = 1.0f;
    raw[8] = 1;
    IMU[8] = 1.0f;
    
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
			haveInput[0] = 0;
			haveInput[1] = 0;
			haveInput[2] = 0;
			
			readSerial(); // this is the threaded serial polling call
            if(fullspeedOSC) {
                sendOSC();
            }
//            ofSleepMillis(1);
            usleep(500);	//mac sleeps in microseconds = 1/2 millisecond interval for serialThread
			unlock();
		}
	}
}

void threadedSerial::readSerial()
{
	long timesRead = 0;
	long nRead  = 0;	
	unsigned char bytesReturned[1];
	
    // read the entire content of the serial buffer into the serailStream and input arrays
	while( (nRead = serial.readBytes(bytesReturned, 1)) > 0){
//		nTimesRead++;	
//		nBytesRead = nRead;
		serialparse(bytesReturned);
	}
}

void threadedSerial::serialparse(unsigned char *c)
{
	int i, j;
	long sum;
	
    // do the circular buffer thing three time independently
    for (j = 0; j < 3; j++) {
        
        // push bytes forward in the three circular buffers "serialStream[j]"
        for (i = 0; i < streamSize[j]-1; i++) {
            serialStream[j][i] = serialStream[j][i+1];
        }
        serialStream[j][streamSize[j]-1] = c[0]; // append new byte to each serialStream[j] buffer
        
    }

	// pattern matching
	if (serialStream[0][0] == 65) { // packet start marker
        if(serialStream[0][1] == 240) {	// left hand packet
            if(serialStream[0][22] == 90) {
//                printf("\ninput 0 ");

                for(i = 0; i < 20; i++) { // collect n-2 bytes into buffer
                    input[0][i] = serialStream[0][i+2];
//                    printf("%x ", input[0][i]);
                }
                haveInput[0] = true;
                parseLeft();
                calcKeycode();
            }
        }
    }
    
    if (serialStream[1][0] == 65) { // packet start marker
        if(serialStream[1][1] == 241) { // right hand packet
            if(serialStream[1][38] == 90) {
//                printf("\ninput 1 ");
                for(i = 0; i < 36; i++) { // collect n-2 bytes into buffer
                    input[1][i] = serialStream[1][i+2];
//                    printf("%x ", input[1][i]);
                }
                haveInput[1] = true;
                parseRight();
                calcKeycode();
                parseIMU();
            }
        }
    }
    
    if (serialStream[2][0] == 65) { // packet start marker
        if(serialStream[2][1] == 242) { // AirMems packet
            if(serialStream[2][14] == 90) {
//                printf("\ninput 2 ");
                for(i = 0; i < 12; i++) { // collect n-2 bytes into buffer
                    input[2][i] = serialStream[2][i+2];
//                    printf("%x ", input[2][i]);
                }
                haveInput[2] = true;
                parseAir();
           }
        }
    }
}

void threadedSerial::parseLeft()
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

//        printf("\nraw left ");
//        for (int i = 0; i < 13; i++) { // 13 keys
//            printf("%lx ", keys[i].raw);
//        }

        
        int now = ofGetElapsedTimeMillis();
		for (int i = 0; i < 13; i++) { // 13 keys
			j = i;
            
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
        
		button[0] = (input[0][16] & 0x8) >> 3;
		button[1] = (input[0][16] & 0x10) >> 4;
		button[2] = (input[0][16] & 0x20) >> 5;
		
		for(int i = 0; i < 3; i++) {
			if(button[i] != buttonOld[i]) {
				buttonChanged[i] = 1;
				buttonOld[i] = button[i];
			} else {
				buttonChanged[i] = 0;
			}
		}
        
        timestampLeft = input[0][17] + (input[0][18] << 8);
        linkQualityLeft = input[0][19];

	} // end haveInput check
}

void threadedSerial::parseRight()
{
	int temp;
	int i, j;
	
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
		
		for (int i = 0; i < 12; i++) { // 12 keys
			j = i+13; // the keys 13 to 25
						
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
        // IMU parsing in separate function() using same input buffer
        batteryLevelRight = input[1][32] & 0xF;
        timestampRight = input[1][33] + (input[1][34] << 8);
        linkQualityRight  = input[1][35];

	}
}

void threadedSerial::parseIMU()
{
	int i;
	if(haveInput[1]) {
		
		
		raw[0] = input[1][15] + ((input[1][18] & 0xE0) << 3); // accel
		raw[1] = input[1][16] + ((input[1][18] & 0x1C) << 6);
		raw[2] = input[1][17] + ((input[1][19] & 0xE0) << 3);
		
		raw[3] = input[1][20] + (input[1][24] << 8); // gyro
		raw[4] = input[1][21] + (input[1][25] << 8);
		raw[5] = input[1][22] + (input[1][26] << 8);

		raw[9] = input[1][23] + (input[1][27] << 8); // temp
		
		raw[6] = input[1][28] + ((input[1][31] & 0xF0) << 4); // compass / magneto
		raw[7] = input[1][29] + ((input[1][31] & 0xF ) << 8);
		raw[8] = input[1][30] + ((input[1][32] & 0xF0) << 4);
		
		
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
		summedIMU[0] = ( fabs(IMU[0] - 0.5) + fabs(IMU[1] - 0.5) + fabs(IMU[2] - 0.5) ) * 0.6666666666666666666666666;
		summedIMU[1] = ( fabs(IMU[3] - 0.5) + fabs(IMU[4] - 0.5) + fabs(IMU[5] - 0.5) ) * 0.6666666666666666666666666;
		summedIMU[2] = ( fabs(IMU[6] - 0.5) + fabs(IMU[7] - 0.5) + fabs(IMU[8] - 0.5) ) * 0.6666666666666666666666666;
		
		calcHeadingTilt();
	}
}

void threadedSerial::parseAir()
{
	if(haveInput[2]) {
		airLong[0] = input[2][0] + (input[2][1] << 8) + (input[2][2] << 16) + (input[2][3] << 24) ; // pressure
		airLong[1] = input[2][4] + (input[2][5] << 8) + (input[2][6] << 16) + (input[2][7] << 24) ; // temperature
		
		air[0] = ((double)(airLong[0] / 100.0));    // + 1.0) * 0.5;
		air[1] = ((double)(airLong[1] / 100.0));	// + 1.0) * 0.5; 
        
        batteryLevelAir = input[2][8] & 0xF;
        timestampAir = input[2][9] + (input[2][10] << 8);
        linkQualityAir = input[2][11];
        
        //collect atmospheric pressure value index 50-300 at each startup to have atmo-pressure offset 
        if(airValue.calibrationFlag) {
            if(airValue.calibrationCounter < 50) {
                airValue.offset = air[0];
            }else {
                if(airValue.calibrationCounter < 300) {
                    airValue.calibrationValue += air[0];
                    airValue.offset = airValue.calibrationValue / (airValue.calibrationCounter - 50);
                }
                if(airValue.calibrationCounter >= 300){
                    airValue.calibrationFlag = 0; // lock up after you
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
                 airValue.scale = ( 1.0 / airValue.minimum) * 0.5;
            }else{
                airValue.scale = 0.0;
            }
        }
        
        airValue.continuous = airValue.relative * airValue.scale + 0.5;
        airValue.continuous = CLAMP( airValue.continuous, 0.0, 1.0);
	}
}

void threadedSerial::calcKeycode()
{	
	int i;
	
	//		if(haveInput[0] || haveInput[1]) {
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
    if(haveInput[0] || haveInput[1]) { // both hands triggers sending
    
        // Keys
        m[61].clear();
        m[61].setAddress( timestampAddressRight ); // timestamp Right
        m[61].addIntArg( timestampRight );
        sender.sendMessage( m[61] );
        
        m[61].clear();
        m[61].setAddress( timestampAddressLeft ); // timestamp Left
        m[61].addIntArg( timestampLeft );
        sender.sendMessage( m[61] );
        
        systime = ofGetElapsedTimeMillis();
        systemTimestamp = systime - oldSystime;
        oldSystime = systime;
        
        m[62].clear();
        m[62].setAddress( timestampAddressServer ); // timestamp
        m[62].addIntArg( systemTimestamp );
        sender.sendMessage( m[62] );
        
        
        for(int i = 0; i < 25; i++) { // continuous key values
            if(keys[i].changed) {
                m[i+16].clear();
                m[i+16].setAddress( keys[i].oscaddress+"/continuous");
                m[i+16].addFloatArg( keys[i].continuous );
                sender.sendMessage( m[i+16] );
            }
        }
        for(int i = 0; i < 25; i++) { // binary key values
            if(keys[i].binaryChanged) {
                m[i+16].clear();
                m[i+16].setAddress( keys[i].oscaddress+"/down");
                m[i+16].addIntArg( keys[i].binary );
                sender.sendMessage( m[i+16] );
            }
        }
        for(int i = 0; i < 25; i++) { // raw key values
            if(keys[i].changed) {
                m[i+16].clear();
                m[i+16].setAddress( keys[i].oscaddress+"/raw");
                m[i+16].addIntArg( keys[i].raw );
                sender.sendMessage( m[i+16] );
            }
        }
        if(keycodeChanged) { // keycode
            m[42].clear();
            m[42].setAddress( keycodeaddress );
            m[42].addIntArg( keycode );
            sender.sendMessage( m[42] );
            keycodeChanged = false;
            // printf("sending keycode %d\n", keycode);
        }
        if(validMidiNote) {
            m[43].clear();	// midinote derived from keycode
            m[43].setAddress( midinoteaddress );
            m[43].addIntArg( midinote );
            sender.sendMessage( m[43] );
            validMidiNote = false;
        }
        for(int i = 0; i < 3; i++) { // buttons
            if(buttonChanged[i]) {
                m[i+44].clear();
                m[i+44].setAddress( buttonaddresses[2-i] );
                m[i+44].addIntArg( button[i] );
                sender.sendMessage( m[i+44] );
            }
        }
        
        // IMU
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
        
        m[12].clear();
        m[12].setAddress( batteryAddressMain ); // battery level main
        m[12].addIntArg( batteryLevelRight );
        sender.sendMessage( m[12] );
        
        m[13].clear();
        m[13].setAddress( linkQualityAddressLeft ); // left link quality
        m[13].addIntArg( linkQualityLeft );
        sender.sendMessage( m[13] );
        
        m[14].clear();
        m[14].setAddress( linkQualityAddressRight ); // right link quality
        m[14].addIntArg( linkQualityRight );
        sender.sendMessage( m[14] );
        
        // reset flags
        if(haveInput[0]) haveInput[0] = false;
        if(haveInput[1]) haveInput[1] = false;
    
    }
    if(haveInput[2]) { // AirMems packet
    
        m[63].clear();
        m[63].setAddress( timestampAddressAir ); // timestamp
        m[63].addIntArg( timestampAir );
        sender.sendMessage( m[63] );
        
        m[48].clear();
        m[48].setAddress( airaddresses[0] ); // air pressure
        m[48].addFloatArg( airValue.continuous);
        sender.sendMessage( m[48] );
        
        m[49].clear();
        m[49].setAddress( airaddresses[1] ); // air temperature
        m[49].addFloatArg( air[1]);
        sender.sendMessage( m[49] );
        
        m[50].clear();
        m[50].setAddress( batteryAddressAir ); // air battery
        m[50].addIntArg( batteryLevelAir);
        sender.sendMessage( m[50] );
        
        m[51].clear();
        m[51].setAddress( linkQualityAddressAir ); // air link quality
        m[51].addIntArg( linkQualityAir);
        sender.sendMessage( m[51] );
        
        // reset flag
        haveInput[2] = false;
    }
}

void threadedSerial::draw()
{
	int i;
	int anchorx = 12;
	int anchory = 66;
	int leftColumn = 110;
	int midColumn = 150;
	int rightColumn = 220;
	int farRightColumn = 330;
    
    int imuColumnLeft = 410;
	
	int stepsize = 18;
	int columnwidth = 180;
	int width = 430;
	int height = 635;
	double yy;
	
	if( lock() )
	{
		if (status == 1 && drawValues)
		{
//			ofFill();
//			ofSetColor(200, 200, 200, 255);
//			ofRect(leftColumn-1, 79, width-leftColumn-5, height-anchory-15);
			
			for(i = 0; i < 25; i++) { // stripes
				if((i % 2) == 0){
					ofFill();
					ofSetColor(255, 255, 255, 255);
					ofRect(leftColumn-1, anchory+((i-1) * stepsize)+7, width-leftColumn-85, 16);
					ofSetColor(0, 0, 0, 255);
				}			
			}
            
            for(i = 0; i < 12; i++) { // stripes
				if((i % 3) == 0){
					ofFill();
					ofSetColor(255, 255, 255, 255);
					ofRect(leftColumn-1 + imuColumnLeft, anchory+((i-1) * stepsize)+7, width-leftColumn-103, 16);
					ofSetColor(0, 0, 0, 255);
				}
			}
            
//            printf("\ndraw raw ");
			for(i = 0; i < 25; i++) { // keys
				ofSetColor(0, 0, 0, 255);
				yy = anchory+(i * stepsize);
				TTF.drawString(ofToString(keys[i].raw, 6), leftColumn, yy );
//                printf("%lx ", keys[i].raw);
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
//				yy = anchory+((i+25) * stepsize);
				yy = anchory+((i) * stepsize);
				TTF.drawString( ofToString(raw[i], 6) , leftColumn + imuColumnLeft, yy );
				TTF.drawString(ofToString(IMU[i], 6), midColumn  + 10 + imuColumnLeft, yy);
				ofNoFill();
				ofSetColor(91, 91,91, 255);
				ofRect(rightColumn + imuColumnLeft, yy-9, 104, 12);
				ofFill();
				ofSetColor(0, 0, 0, 255);
				ofRect( rightColumn + imuColumnLeft + (104 * IMU[i]), yy-9, 2, 12);
				ofNoFill();
				ofSetColor(91, 91, 91, 255);
				ofLine(rightColumn+52 + imuColumnLeft, yy-9, rightColumn+52 + imuColumnLeft, yy+4);
			}	
			// air 
			ofSetColor(0, 0, 0, 255);
//			yy = anchory+(34 * stepsize);
			yy = anchory+(10 * stepsize);
			TTF.drawString(ofToString(air[0], 2), leftColumn + imuColumnLeft, yy );
			TTF.drawString(ofToString(airValue.continuous, 2), midColumn  + 10 + imuColumnLeft, yy);
			
			ofNoFill();
			ofSetColor(91, 91, 91, 255);
			ofRect(rightColumn + imuColumnLeft, yy-9, 104, 12);
			ofFill();
			ofSetColor(0, 0, 0, 127);
			ofRect( rightColumn + imuColumnLeft + (104 * (CLAMP( ((airLong[0] - 500.0) * 0.001), 0, 1))), yy-9, 2, 12);
            
            if(airValue.calibratePressureRange) {
                ofSetColor(255, 224, 0, 191);
                ofRect( rightColumn + imuColumnLeft, yy-7, (103), 9);
                // TODO figure scaling for the rangebars
                ofSetColor(0, 0, 0, 255);
                ofRect( rightColumn + imuColumnLeft + (104 * (CLAMP( ((airValue.continuous - 500.0) * 0.001), 0, 1))), yy-9, 2, 12);
            } else {
                ofNoFill();
                ofSetColor(91, 91, 91, 255);
                ofRect(rightColumn + imuColumnLeft, yy-9, 104, 12);
                ofFill();
                ofSetColor(0, 0, 0, 127);
                ofRect( rightColumn + imuColumnLeft + (104 * (CLAMP( ((airValue.continuous - 500.0) * 0.001), 0, 1))), yy-9, 2, 12);
            }
            
            
            
			
			// buttons
			ofSetColor(0, 0, 0, 255);
//			yy = anchory+((35) * stepsize);
			yy = anchory+((9) * stepsize);
			TTF.drawString( ofToString(button[2], 1), leftColumn + imuColumnLeft, yy );
			TTF.drawString( ofToString(button[1], 1), leftColumn+12 + imuColumnLeft, yy );
			TTF.drawString( ofToString(button[0], 1), leftColumn+24 + imuColumnLeft, yy );
            
			ofNoFill();
			ofSetColor(91, 91, 91, 255);
			ofRect(midColumn + 10 + imuColumnLeft, yy-9, 12, 12);
			ofRect(midColumn + 10 + 14 + imuColumnLeft, yy-9, 12, 12);
			ofRect(midColumn + 10 + 28 + imuColumnLeft, yy-9, 12, 12);
			
			ofFill();
			ofSetColor(0, 0, 0, 255);
			if(button[2]) {
				ofRect(midColumn + 12 + imuColumnLeft, yy-6, 7, 7);
			}				
			if(button[1]) {
				ofRect(midColumn + 12 + 14 + imuColumnLeft, yy-6, 7, 7);
			}
			if(button[0]) {
				ofRect(midColumn + 12 + 28 + imuColumnLeft, yy-6, 7, 7);
			}
            
			// battery
//            ofSetColor(0, 0, 0, 255);
////			yy = anchory+((36) * stepsize);
//            yy = 40;
//			TTF.drawString( "main: "+ofToString((int)(batteryLevelRight*12.5))+"%", anchorx+82 + 360, yy );
//			TTF.drawString( "mouthpiece: "+ofToString((int)(batteryLevelAir*12.5))+"%", leftColumn+12 + 360, yy );
            
			if(calibrateAll) {
				ofFill();
				ofSetColor(255, 127, 0);
				ofRect(375, 480, 124, 20);
				ofNoFill();
				ofSetColor(127, 127, 127);
				ofRect(375, 480, 124, 20);
				ofSetColor(0, 0, 0);
				TTF.drawString("Calibrating Keys...", 375+12, 480+14);
			}
            if(airValue.calibratePressureRange) {
				ofFill();
				ofSetColor(255, 224, 0);
				ofRect(502, 480, 124, 20);
				ofNoFill();
				ofSetColor(127, 127, 127);
				ofRect(502, 480, 124, 20);
				ofSetColor(0, 0, 0);
				TTF.drawString("Calibrating Air...", 502+12, 480+14);
			}
		}
		unlock();
	}else{
		//			str = "can't lock!\neither an error\nor the thread has stopped";
        ofLog(OF_LOG_ERROR, "SabreServer: couldn't start serial Thread !! can't lock!\neither an error\nor the thread has stopped");
	}
}
