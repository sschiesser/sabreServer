/*
 *  ofxRawHID.cpp
 *  openFrameworks addon for PJRC rawHID devices
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
 *  @@date 27/7/2014
 *
 */


#include "ofxRawHID.h"

#include <string.h>

ofxRawHID::ofxRawHID()
{
    packetSize = -1;
    deviceIndex = 0;
    deviceOpen = false;
    timeout = 1; // millisecs :: default 1 ms
    len = 64; // buffersize
    
    //set up storage for device list transfer
    // arrays
    rawDeviceIndex = new int[ MAX_NUM_DEVICES ];
    rawVendorID = new long[ MAX_NUM_DEVICES ];
    rawProductID = new long[ MAX_NUM_DEVICES ];
    // setup storage for array of char *
    rawManufacturerName = new char*[ MAX_NUM_DEVICES ];
    rawProductName = new char*[ MAX_NUM_DEVICES ];
    for (int i = 0; i < MAX_NUM_DEVICES; i++ ) {
        rawManufacturerName[i] = new char[1024];
        rawProductName[i] = new char[1024];
    }
}

ofxRawHID::~ofxRawHID()
{
    clearDeviceList();
    closeDevice();
    
    // clear storage for device list transfer
    delete[] rawDeviceIndex;
    delete[] rawVendorID;
    delete[] rawProductID;
    for (int i = 0; i < 256; ++i) {
        delete[] rawManufacturerName[i];
        delete[] rawProductName[i];
    }
    delete[] rawManufacturerName;
    delete[] rawProductName;
}

int ofxRawHID::openDevice()
{
    if(deviceSelected) {
        long vendorID = deviceInfo.getVendorID();
        long productID = deviceInfo.getProductID();
		long usagePage = deviceInfo.getUsagePage();
		long usage = deviceInfo.getUsage();
        
        int r = rawhid_open(1, vendorID, productID, usagePage, usage);
        if (r <= 0) {
            ofLog(OF_LOG_ERROR) << "no rawhid device found";
            deviceOpen = false;
            return -1;
        } else {
            ofLog(OF_LOG_VERBOSE) << "found rawhid device: " <<  ofToString(vendorID) << ":" << ofToString(productID);
            deviceOpen = true;
			return 0;
        }
    }else{
        ofLog(OF_LOG_WARNING) << "no rawhid device selected";
        return -1;
    }
}

bool ofxRawHID::closeDevice()
{
    if(deviceOpen) {
        rawhid_close(deviceIndex);
        deviceOpen = false;
    }
}

bool ofxRawHID::isOpen()
{
    return rawhid_isOpen(deviceIndex);
}

int ofxRawHID::receivePacket()
{
//    num = rawhid_recv(0, buf, 64, 220);
    int result = rawhid_recv(deviceIndex, buf, len, timeout);
    
    if (result < 0) {
        ofLog(OF_LOG_ERROR) << "error reading rawHID, device went offline";
        closeDevice();
        return -1;
    }
    return result;
}

int ofxRawHID::sendPacket()
{
    int result = rawhid_send(deviceIndex, buf, len, timeout);
    return result;
}

int ofxRawHID::listDevices()
{
    bool doubleFlag = false;
    int numberOfDevices = rawhid_listdevices(rawDeviceIndex, rawVendorID, rawProductID, rawManufacturerName, rawProductName);
//    cout << "numberOfDevices " << ofToString(numberOfDevices) << endl;
    
    if(numberOfDevices > 0) {
        // copy into local reviceList with the filtering applied
        ofxHIDDeviceInfo localDeviceInfo;
        clearDeviceList();
        
        if(numberOfDevices >= MAX_NUM_DEVICES){
            ofLog(OF_LOG_ERROR, "Too many HID devices, maximum is %d", MAX_NUM_DEVICES);
            return -1;
        }
     
        
        for(int i = 0; i < numberOfDevices ; i++) {
            
//            int compare = strncmp(rawProductName[i], "Apple", 5);
//            printf("string compare for \"apple\" at %s %d\n", rawProductName[i], compare);
//            if( rawVendorID[i] >= 0x1000 ) {
            
            if ( strncmp(rawProductName[i], "Apple", 5) != 0) { // filter out Apple HID devices

                localDeviceInfo.index = rawDeviceIndex[i];
                localDeviceInfo.manufacturerName = std::string( &rawManufacturerName[i][0] ); // double dereference from array of char *
                localDeviceInfo.productName = std::string( &rawProductName[i][0] );
                localDeviceInfo.vendorID = rawVendorID[i];
                localDeviceInfo.productID = rawProductID[i];
                
                HID_devices.push_back(localDeviceInfo);
                
//                printf( "device %d %04lX %04lX  %s %s \n", rawDeviceIndex[i], rawVendorID[i], rawProductID[i],  &rawManufacturerName[i][0],  &rawProductName[i][0] );
            }
        }
        // sort and remove duplicates from HID_devices vector
        std::sort(HID_devices.begin(), HID_devices.end());
        auto last = std::unique(HID_devices.begin(), HID_devices.end());
        HID_devices.erase(last, HID_devices.end());
        return HID_devices.size();

    }else{
        return -1;
    }
}

bool ofxRawHID::clearDeviceList()
{
    HID_devices.clear();
}
