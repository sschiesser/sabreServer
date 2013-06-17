/*
 *  threadedOSC.cpp
 * 
 *  SABRe-server
 *  © 2012 ICST / ZHdK  
 *
 *  @author Jan Schacher
 *  @date 20121030
 *
 */

#include "threadedOSC.h"

threadedOSC::threadedOSC()
{
	int i;
	// set the default address
	for(i = 0; i < 25; i++) {
		address[0] = "/sabre/key/"+ofToString(i)+"/continuous";
	}
    batteryAddressMain = "/sabre/battery/main";
    batteryAddressAir = "/sabre/battery/air";
    
    timestampAddressLeft = "/sabre/timestamp/left";
    timestampAddressRight = "/sabre/timestamp/right";
    timestampAddressAir = "/sabre/timestamp/air";
	
	TTF.loadFont("lucidagrande.ttf", 8, 1, 1, 0);
	}

threadedOSC::~threadedOSC()
{
	
	if( isThreadRunning() ) { 
		stopThread();
	}
}

void threadedOSC::start()
{
	startThread(true, false);   // blocking, verbose
}

void threadedOSC::stop()
{
	stopThread();
}

//--------------------------
void threadedOSC::threadedFunction() 
{
	while( isThreadRunning() != 0 ){
		if( lock() ){
            sendOSC();
			ofSleepMillis(1);
			unlock();			
		}
	}
}

void threadedOSC::sendOSC()
{	
	// timestamps & keys
	if(haveInput[1] || haveInput[0]) { // right hand triggers sending
		
		
		m[25].clear();
		m[25].setAddress( timestampAddressRight ); // timestamp
        m[25].addIntArg( serialObject->timestampRight );
		sender.sendMessage( m[25] );
		
		for(int i = 0; i < 25; i++) { // continuous key values
			if(serialObject->keys[i].changed) {
				m[i].clear();
				m[i].setAddress( serialObject->keys[i].oscaddress+"/continuous");
				m[i].addFloatArg( serialObject->keys[i].continuous );
				sender.sendMessage( m[i] );
			}
		}
		for(int i = 0; i < 25; i++) { // binary key values
			if(serialObject->keys[i].binaryChanged) {
				m[i].clear();
				m[i].setAddress( serialObject->keys[i].oscaddress+"/down");
				m[i].addIntArg( serialObject->keys[i].binary );
				sender.sendMessage( m[i] );
			}
		}	
		for(int i = 0; i < 25; i++) { // raw key values
			if(serialObject->keys[i].changed) {
				m[i].clear();
				m[i].setAddress( serialObject->keys[i].oscaddress+"/raw");
				m[i].addIntArg( serialObject->keys[i].raw );
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
			if(serialObject->buttonChanged[i]) {
				m[i].clear();
				m[i].setAddress( buttonaddresses[2-i] );
				m[i].addIntArg( serialObject->button[i] );
				sender.sendMessage( m[i] );	
			}
		}
		if(haveInput[0]) haveInput[0] = false;
        if(haveInput[1]) haveInput[1] = false;
    }
	//IMU
	if(haveInput[2]) {
        
        m[25].clear();
		m[25].setAddress( timestampAddressLeft ); // timestamp
        m[25].addIntArg( serialObject->timestampLeft );
		sender.sendMessage( m[25] );
        
		serialObject->systime = ofGetElapsedTimeMillis();
		serialObject->systemTimestamp = serialObject->systime - serialObject->oldSystime;
		serialObject->oldSystime = serialObject->systime;
        
        m[13].clear();
		m[13].setAddress( timestampAddressServer ); // timestamp
        m[13].addIntArg( serialObject->systemTimestamp );
		sender.sendMessage( m[13] );
		
		m[0].clear();
		m[0].setAddress( imuaddresses[0] ); // IMU accelero scaled
		m[0].addFloatArg( serialObject->IMU[0] );
		m[0].addFloatArg( serialObject->IMU[1] );
		m[0].addFloatArg( serialObject->IMU[2] );
		sender.sendMessage( m[0] );	
		
		m[1].clear();
		m[1].setAddress( imuaddresses[1] ); // IMU gyro scaled
		m[1].addFloatArg( serialObject->IMU[3] );
		m[1].addFloatArg( serialObject->IMU[4] );
		m[1].addFloatArg( serialObject->IMU[5] );
		sender.sendMessage( m[1] );	
		
		m[2].clear();
		m[2].setAddress( imuaddresses[2] ); // IMU magneto scaled
		m[2].addFloatArg( serialObject->IMU[6] );
		m[2].addFloatArg( serialObject->IMU[7] );
		m[2].addFloatArg( serialObject->IMU[8] );
		sender.sendMessage( m[2] );	
		
		m[3].clear();
		m[3].setAddress( imuaddresses[3] ); // IMU accelero raw
		m[3].addFloatArg( serialObject->rawIMU[0] );
		m[3].addFloatArg( serialObject->rawIMU[1] );
		m[3].addFloatArg( serialObject->rawIMU[2] );
		sender.sendMessage( m[3] );	
		
		m[4].clear();
		m[4].setAddress( imuaddresses[4] ); // IMU gyro raw
		m[4].addFloatArg( serialObject->rawIMU[3] );
		m[4].addFloatArg( serialObject->rawIMU[4] );
		m[4].addFloatArg( serialObject->rawIMU[5] );
		sender.sendMessage( m[4] );	
		
		m[5].clear();
		m[5].setAddress( imuaddresses[5] ); // IMU magneto raw
		m[5].addFloatArg( serialObject->rawIMU[6] );
		m[5].addFloatArg( serialObject->rawIMU[7] );
		m[5].addFloatArg( serialObject->rawIMU[8] );
		sender.sendMessage( m[5] );	
		
		m[6].clear();
		m[6].setAddress( imuaddresses[6] ); // IMU accelero summed
		m[6].addFloatArg( serialObject->summedIMU[0] );
		sender.sendMessage( m[6] );	
		
		m[7].clear();
		m[7].setAddress( imuaddresses[7] ); // IMU gyro summed
		m[7].addFloatArg( serialObject->summedIMU[1] );
		sender.sendMessage( m[7] );	
		
		m[8].clear();
		m[8].setAddress( imuaddresses[8] ); // IMU magneto summed
		m[8].addFloatArg( serialObject->summedIMU[2] );
		sender.sendMessage( m[8] );
		
		m[9].clear();
		m[9].setAddress( imuaddresses[10] ); // IMU heading from accelerometer
		m[9].addFloatArg( serialObject->heading );
		sender.sendMessage( m[9] );	
		
		m[10].clear();
		m[10].setAddress( imuaddresses[11] ); // IMU tilt from accelerometer
		m[10].addFloatArg( serialObject->tilt );
		sender.sendMessage( m[10] );
		
		m[11].clear();
		m[11].setAddress( imuaddresses[9] ); // IMU temperature in degreee celsius 
		m[11].addFloatArg( serialObject->IMU[9] );
		sender.sendMessage( m[11] );
        
        m[14].clear();
		m[14].setAddress( batteryAddressMain ); // air temperature
		m[14].addIntArg( serialObject->batteryLevelRight );
		sender.sendMessage( m[14] );       
		
		haveInput[2] = false;
	}
	if(haveInput[3]) {
        
        m[25].clear();
		m[25].setAddress( timestampAddressAir ); // timestamp
        m[25].addIntArg( serialObject->timestampAir );
		sender.sendMessage( m[25] );
		
		m[11].clear();
		m[11].setAddress( airaddresses[0] ); // air pressure
		m[11].addFloatArg( serialObject->air[0]);
		sender.sendMessage( m[11] );	
		
		m[12].clear();
		m[12].setAddress( airaddresses[1] ); // air temperature
		m[12].addFloatArg( serialObject->air[1]);
		sender.sendMessage( m[12] );
        
		m[13].clear();
		m[13].setAddress( batteryAddressAir ); // air temperature
		m[13].addIntArg( serialObject->batteryLevelAir);
		sender.sendMessage( m[13] );        
		
		haveInput[3] = false;
	}
}
