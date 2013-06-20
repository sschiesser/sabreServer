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
    OSCInterval = 4; // default value
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
//    printf("OSC thread started\n");
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
            if(serialObject->fullspeedOSC == 0){
                sendOSC();
            }
            ofSleepMillis(OSCInterval);
			unlock();
		}
	}
}

void threadedOSC::sendOSC()
{
    
// timestamps & keys
//	if(serialObject->haveInput[0] || serialObject->haveInput[1]) { // both hands triggers sending

        // Keys
		m[61].clear();
		m[61].setAddress( serialObject->timestampAddressRight ); // timestamp Right
        m[61].addIntArg( serialObject->timestampRight );
		sender.sendMessage( m[61] );
        
        m[61].clear();
		m[61].setAddress( serialObject->timestampAddressLeft ); // timestamp Left
        m[61].addIntArg( serialObject->timestampLeft );
		sender.sendMessage( m[61] );
        
		serialObject->systime = ofGetElapsedTimeMillis();
		serialObject->systemTimestamp = serialObject->systime - serialObject->oldSystime;
		serialObject->oldSystime = serialObject->systime;
        
        m[62].clear();
		m[62].setAddress( serialObject->timestampAddressServer ); // timestamp
        m[62].addIntArg( serialObject->systemTimestamp );
		sender.sendMessage( m[62] );
        
		
		for(int i = 0; i < 25; i++) { // continuous key values
			if(serialObject->keys[i].changed) {
				m[i+16].clear();
				m[i+16].setAddress( serialObject->keys[i].oscaddress+"/continuous");
				m[i+16].addFloatArg( serialObject->keys[i].continuous );
				sender.sendMessage( m[i+16] );
			}
		}
		for(int i = 0; i < 25; i++) { // binary key values
			if(serialObject->keys[i].binaryChanged) {
				m[i+16].clear();
				m[i+16].setAddress( serialObject->keys[i].oscaddress+"/down");
				m[i+16].addIntArg( serialObject->keys[i].binary );
				sender.sendMessage( m[i+16] );
			}
		}	
		for(int i = 0; i < 25; i++) { // raw key values
			if(serialObject->keys[i].changed) {
				m[i+16].clear();
				m[i+16].setAddress( serialObject->keys[i].oscaddress+"/raw");
				m[i+16].addIntArg( serialObject->keys[i].raw );
				sender.sendMessage( m[i+16] );
			}
		}	
		if(serialObject->keycodeChanged) { // keycode
			m[42].clear();
			m[42].setAddress( serialObject->keycodeaddress );
			m[42].addIntArg( serialObject->keycode );
			sender.sendMessage( m[42] );
			serialObject->keycodeChanged = false;
			// printf("sending keycode %d\n", keycode);
		}
		if(serialObject->validMidiNote) {	
			m[43].clear();	// midinote derived from keycode
			m[43].setAddress( serialObject->midinoteaddress );
			m[43].addIntArg( serialObject->midinote );
			sender.sendMessage( m[43] );
			serialObject->validMidiNote = false;
		}
		for(int i = 0; i < 3; i++) { // buttons
			if(serialObject->buttonChanged[i]) {
				m[i+44].clear();
				m[i+44].setAddress( serialObject->buttonaddresses[2-i] );
				m[i+44].addIntArg( serialObject->button[i] );
				sender.sendMessage( m[i+44] );	
			}
		}

        // IMU
		m[0].clear();
		m[0].setAddress( serialObject->imuaddresses[0] ); // IMU accelero scaled
		m[0].addFloatArg( serialObject->IMU[0] );
		m[0].addFloatArg( serialObject->IMU[1] );
		m[0].addFloatArg( serialObject->IMU[2] );
		sender.sendMessage( m[0] );	
		
		m[1].clear();
		m[1].setAddress( serialObject->imuaddresses[1] ); // IMU gyro scaled
		m[1].addFloatArg( serialObject->IMU[3] );
		m[1].addFloatArg( serialObject->IMU[4] );
		m[1].addFloatArg( serialObject->IMU[5] );
		sender.sendMessage( m[1] );	
		
		m[2].clear();
		m[2].setAddress( serialObject->imuaddresses[2] ); // IMU magneto scaled
		m[2].addFloatArg( serialObject->IMU[6] );
		m[2].addFloatArg( serialObject->IMU[7] );
		m[2].addFloatArg( serialObject->IMU[8] );
		sender.sendMessage( m[2] );	
		
		m[3].clear();
		m[3].setAddress( serialObject->imuaddresses[3] ); // IMU accelero raw
		m[3].addFloatArg( serialObject->rawIMU[0] );
		m[3].addFloatArg( serialObject->rawIMU[1] );
		m[3].addFloatArg( serialObject->rawIMU[2] );
		sender.sendMessage( m[3] );	
		
		m[4].clear();
		m[4].setAddress( serialObject->imuaddresses[4] ); // IMU gyro raw
		m[4].addFloatArg( serialObject->rawIMU[3] );
		m[4].addFloatArg( serialObject->rawIMU[4] );
		m[4].addFloatArg( serialObject->rawIMU[5] );
		sender.sendMessage( m[4] );	
		
		m[5].clear();
		m[5].setAddress( serialObject->imuaddresses[5] ); // IMU magneto raw
		m[5].addFloatArg( serialObject->rawIMU[6] );
		m[5].addFloatArg( serialObject->rawIMU[7] );
		m[5].addFloatArg( serialObject->rawIMU[8] );
		sender.sendMessage( m[5] );	
		
		m[6].clear();
		m[6].setAddress( serialObject->imuaddresses[6] ); // IMU accelero summed
		m[6].addFloatArg( serialObject->summedIMU[0] );
		sender.sendMessage( m[6] );	
		
		m[7].clear();
		m[7].setAddress( serialObject->imuaddresses[7] ); // IMU gyro summed
		m[7].addFloatArg( serialObject->summedIMU[1] );
		sender.sendMessage( m[7] );	
		
		m[8].clear();
		m[8].setAddress( serialObject->imuaddresses[8] ); // IMU magneto summed
		m[8].addFloatArg( serialObject->summedIMU[2] );
		sender.sendMessage( m[8] );
		
		m[9].clear();
		m[9].setAddress( serialObject->imuaddresses[10] ); // IMU heading from accelerometer
		m[9].addFloatArg( serialObject->heading );
		sender.sendMessage( m[9] );	
		
		m[10].clear();
		m[10].setAddress( serialObject->imuaddresses[11] ); // IMU tilt from accelerometer
		m[10].addFloatArg( serialObject->tilt );
		sender.sendMessage( m[10] );
		
		m[11].clear();
		m[11].setAddress( serialObject->imuaddresses[9] ); // IMU temperature in degreee celsius 
		m[11].addFloatArg( serialObject->IMU[9] );
		sender.sendMessage( m[11] );
        
        m[12].clear();
		m[12].setAddress( serialObject->batteryAddressMain ); // battery level main
		m[12].addIntArg( serialObject->batteryLevelRight );
		sender.sendMessage( m[12] );
        
        m[13].clear();
		m[13].setAddress( serialObject->linkQualityAddressLeft ); // left link quality
		m[13].addIntArg( serialObject->linkQualityLeft );
		sender.sendMessage( m[13] );
        
        m[14].clear();
		m[14].setAddress( serialObject->linkQualityAddressRight ); // right link quality
		m[14].addIntArg( serialObject->linkQualityRight );
		sender.sendMessage( m[14] );
        
        // reset flags
		if(serialObject->haveInput[0]) serialObject->haveInput[0] = false;
        if(serialObject->haveInput[1]) serialObject->haveInput[1] = false;

//    }
//	if(serialObject->haveInput[2]) { // AirMems packet
    
        m[63].clear();
		m[63].setAddress( serialObject->timestampAddressAir ); // timestamp
        m[63].addIntArg( serialObject->timestampAir );
		sender.sendMessage( m[63] );
		
		m[48].clear();
		m[48].setAddress( serialObject->airaddresses[0] ); // air pressure
		m[48].addFloatArg( serialObject->air[0]);
		sender.sendMessage( m[48] );
		
		m[49].clear();
		m[49].setAddress( serialObject->airaddresses[1] ); // air temperature
		m[49].addFloatArg( serialObject->air[1]);
		sender.sendMessage( m[49] );
        
		m[50].clear();
		m[50].setAddress( serialObject->batteryAddressAir ); // air battery
		m[50].addIntArg( serialObject->batteryLevelAir);
		sender.sendMessage( m[50] );
        
        m[51].clear();
		m[51].setAddress( serialObject->linkQualityAddressAir ); // air link quality
		m[51].addIntArg( serialObject->linkQualityAir);
		sender.sendMessage( m[51] );
        
        // reset flag
		serialObject->haveInput[2] = false;
//	}
}
