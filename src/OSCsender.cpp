/*
*  OSCsender.cpp
*  sabreServer
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
*  @date 20131220
*
*/

#include "OSCsender.h"


OSCsender::~OSCsender()
{
    //    sender.shutdown(); // it's ! private !!! i hope it gets properly destroyed
}

void OSCsender::sendOSC(bool reset)
{
    
    // Keys
    m[61].clear();
    m[61].setAddress( data->timestampAddressLeft ); // timestamp Left
    m[61].addIntArg( data->timestampLeft );
    sender.sendMessage( m[61] );
    
    m[62].clear();
    m[62].setAddress( data->timestampAddressRight ); // timestamp Right
    m[62].addIntArg( data->timestampRight );
    sender.sendMessage( m[62] );
    
    data->systemTimestamp = ofGetElapsedTimeMillis();
    
    m[63].clear();
    m[63].setAddress( data->timestampAddressServer ); // timestamp
    m[63].addIntArg( data->deltaTimeL );
    m[63].addIntArg( data->deltaTimeR );
    m[63].addIntArg( data->systemTimestamp );
    sender.sendMessage( m[63] );
    
    
    if(data->sendRawValues) {
        for(int i = 0; i < 25; i++) { // raw key values
            if(data->keys[i].changed) {
                m[i+16].clear();
                m[i+16].setAddress( data->keys[i].oscaddress+"/raw");
                m[i+16].addIntArg( data->keys[i].raw );
                sender.sendMessage( m[i+16] );
            }
        }
    }
    
    for(int i = 0; i < 25; i++) { // continuous key values
        if(data->keys[i].changed) {
            m[i+16].clear();
            m[i+16].setAddress( data->keys[i].oscaddress+"/continuous");
            m[i+16].addFloatArg( data->keys[i].continuous );
            sender.sendMessage( m[i+16] );
            if(reset) {
                data->keys[i].changed = false;
            }
        }
    }
    for(int i = 0; i < 25; i++) { // binary key values
        if(data->keys[i].binaryChanged) {
            m[i+16].clear();
            m[i+16].setAddress( data->keys[i].oscaddress+"/down");
            m[i+16].addIntArg( data->keys[i].binary );
            sender.sendMessage( m[i+16] );
            if(reset) {
                data->keys[i].binaryChanged = false;
            }
        }
    }
    
    if(data->keycodeChanged) { // keycode
        m[42].clear();
        m[42].setAddress( data->keycodeaddress );
        m[42].addIntArg( data->keycode );
        sender.sendMessage( m[42] );
        if(reset) {
            data->keycodeChanged = false;
        }        
        if(data->validMidiNote) {
            m[43].clear();	// midinote derived from keycode
            m[43].setAddress( data->midinoteaddress );
            m[43].addIntArg( data->midinote );
            sender.sendMessage( m[43] );
        }
    }
    
    for(int i = 0; i < 3; i++) { // buttons
        if(data->buttonChanged[i]) {
            m[i+44].clear();
            m[i+44].setAddress( data->buttonaddresses[2-i] );
            m[i+44].addIntArg( data->button[i] );
            sender.sendMessage( m[i+44] );
            if(reset) {
                data->buttonChanged[i] = false;
            }
        }
    }
    
    // IMU
    m[0].clear();
    m[0].setAddress( data->imuaddresses[0] ); // IMU accelero scaled
    m[0].addFloatArg( data->IMU[0] );
    m[0].addFloatArg( data->IMU[1] );
    m[0].addFloatArg( data->IMU[2] );
    sender.sendMessage( m[0] );
    
    m[1].clear();
    m[1].setAddress( data->imuaddresses[1] ); // IMU gyro scaled
    m[1].addFloatArg( data->IMU[3] );
    m[1].addFloatArg( data->IMU[4] );
    m[1].addFloatArg( data->IMU[5] );
    sender.sendMessage( m[1] );
    
    m[2].clear();
    m[2].setAddress( data->imuaddresses[2] ); // IMU magneto scaled
    m[2].addFloatArg( data->IMU[6] );
    m[2].addFloatArg( data->IMU[7] );
    m[2].addFloatArg( data->IMU[8] );
    sender.sendMessage( m[2] );
    
    if(data->sendRawValues) {
        m[3].clear();
        m[3].setAddress( data->imuaddresses[3] ); // IMU accelero raw
        m[3].addFloatArg( data->rawIMU[0] );
        m[3].addFloatArg( data->rawIMU[1] );
        m[3].addFloatArg( data->rawIMU[2] );
        sender.sendMessage( m[3] );
        
        m[4].clear();
        m[4].setAddress( data->imuaddresses[4] ); // IMU gyro raw
        m[4].addFloatArg( data->rawIMU[3] );
        m[4].addFloatArg( data->rawIMU[4] );
        m[4].addFloatArg( data->rawIMU[5] );
        sender.sendMessage( m[4] );
        
        m[5].clear();
        m[5].setAddress( data->imuaddresses[5] ); // IMU magneto raw
        m[5].addFloatArg( data->rawIMU[6] );
        m[5].addFloatArg( data->rawIMU[7] );
        m[5].addFloatArg( data->rawIMU[8] );
        sender.sendMessage( m[5] );
    }
    
    m[6].clear();
    m[6].setAddress( data->imuaddresses[6] ); // IMU accelero summed
    m[6].addFloatArg( data->summedIMU[0] );
    sender.sendMessage( m[6] );
    
    m[7].clear();
    m[7].setAddress( data->imuaddresses[7] ); // IMU gyro summed
    m[7].addFloatArg( data->summedIMU[1] );
    sender.sendMessage( m[7] );
    
    m[8].clear();
    m[8].setAddress( data->imuaddresses[8] ); // IMU magneto summed
    m[8].addFloatArg( data->summedIMU[2] );
    sender.sendMessage( m[8] );
    
    m[9].clear();
    m[9].setAddress( data->imuaddresses[10] ); // IMU heading from accelerometer
    m[9].addFloatArg( data->heading );
    sender.sendMessage( m[9] );
    
    m[10].clear();
    m[10].setAddress( data->imuaddresses[11] ); // IMU tilt from accelerometer
    m[10].addFloatArg( data->tilt );
    sender.sendMessage( m[10] );
    
    m[11].clear();
    m[11].setAddress( data->imuaddresses[9] ); // IMU temperature in degreee celsius
    m[11].addFloatArg( data->IMU[9] );
    sender.sendMessage( m[11] );
    
    m[12].clear();
    m[12].setAddress( data->batteryAddressMain ); // battery level main
    m[12].addIntArg( data->batteryLevelRight );
    sender.sendMessage( m[12] );
    
    m[13].clear();
    m[13].setAddress( data->linkQualityAddressLeft ); // left link quality
    m[13].addIntArg( data->linkQualityLeft );
    sender.sendMessage( m[13] );
    
    m[14].clear();
    m[14].setAddress( data->linkQualityAddressRight ); // right link quality
    m[14].addIntArg( data->linkQualityRight );
    sender.sendMessage( m[14] );
    
    // airMEMS
    m[48].clear();
    m[48].setAddress( data->timestampAddressAir ); // timestamp
    m[48].addIntArg( data->timestampAir );
    sender.sendMessage( m[48] );
    
    m[49].clear();
    m[49].setAddress( data->airaddresses[0] ); // air pressure
    m[49].addFloatArg( data->airValue.continuous);
    sender.sendMessage( m[49] );
    
    m[50].clear();
    m[50].setAddress( data->airaddresses[1] ); // air temperature
    m[50].addFloatArg( data->air[1]);
    sender.sendMessage( m[50] );
    
    m[51].clear();
    m[51].setAddress( data->batteryAddressAir ); // air battery
    m[51].addIntArg( data->batteryLevelAir);
    sender.sendMessage( m[51] );
    
    m[52].clear();
    m[52].setAddress( data->linkQualityAddressAir ); // air link quality
    m[52].addIntArg( data->linkQualityAir);
    sender.sendMessage( m[52] );
    
    // reset flags
    if(reset) {
        if(data->haveInput[0]) {
            data->haveInput[0] = false;
        }
        if(data->haveInput[1]) {
            data->haveInput[1] = false;
        }
        if(data->haveInput[2]) {
            data->haveInput[2] = false;
        }
    }
}
