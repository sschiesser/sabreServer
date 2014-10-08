/*
 *  ofxRawHID.h
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

#ifndef __ofxRawHID__
#define __ofxRawHID__

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


#if defined(OS_LINUX) || defined(OS_MACOSX)
#include <sys/ioctl.h>
#include <termios.h>

#elif defined(OS_WINDOWS)
#include <conio.h>
#endif

#include "ofMain.h"

extern "C" {
    #include "raw_hid.h"
}

class ofxHIDDeviceInfo {
public:
    ofxHIDDeviceInfo(string manufacturerNameIn, string productNameIn, long vendorIDIn, long productIDIn, int indexIn) {
        manufacturerName	= manufacturerNameIn;
        productName			= productNameIn;
        vendorID			= vendorIDIn;
        productID			= productIDIn;
        index               = indexIn;
    }
	
    ofxHIDDeviceInfo() {
        manufacturerName	= "undefinedManufacturerName";
        productName			= "undefinedProductName";
        vendorID			= -1;
        productID			= -1;
		usagePage			= -1;
		usage				= -1;
        index               = -1;
    }
    
    friend bool operator<( const ofxHIDDeviceInfo& c1, const ofxHIDDeviceInfo& c2 ) {
        return c1.productID < c2.productID;
    }

    friend bool operator==(const ofxHIDDeviceInfo& c1, const ofxHIDDeviceInfo& c2) {
        return c1.productID == c2.productID;
    }
    
    string getManufacturerName() {
        return manufacturerName;
    }
    string getProductName() {
        return productName;
    }
    long getVendorID() {
        return vendorID;
    }
    long getProductID() {
        return productID;
    }
    int getIndex() {
        return index;
    }
	long getUsagePage() {
        return usagePage;
    }
    long getUsage() {
        return usage;
    }
    void setManufacturerName(string manName) {
        manufacturerName = manName;
    }
    void setProductName(string prodName) {
        productName = prodName;
    }
    void setVendorID(long vendID) {
        vendorID = vendID;
    }
    void setProductID(long prodID) {
        productID = prodID;
    }
    void setIndex(int indexInput) {
        index = indexInput;
    }
	void setUsagePage(long usePg) {
        usagePage = usePg;
    }
    void setUsage(long usg) {
        usage = usg;
    }

    int     index;
    string  manufacturerName;       //eg: 'ICST'
    string  productName;			//eg: 'SABRe'
    long    vendorID;				//eg: 0x1C57
    long    productID;				//eg: 0x5ABE
	long    usagePage;				//eg: oxFFAB
    long    usage;					//eg: 0x0100
    
};


class ofxRawHID
{
	
public:

    ofxRawHID();
	~ofxRawHID();

    int openDevice();
    bool closeDevice();
    bool isOpen();
    int receivePacket();
    int sendPacket();
    int listDevices();
    bool clearDeviceList();
    
    vector <ofxHIDDeviceInfo> HID_devices;
    ofxHIDDeviceInfo deviceInfo;
    int deviceIndex;
    bool deviceSelected;
    bool deviceOpen;
    
    int packetSize;
    unsigned char  buf[64];
    int len;
    int timeout;
    
private:
    int * rawDeviceIndex;
    long * rawVendorID;
    long * rawProductID;
    char ** rawManufacturerName;
    char ** rawProductName;
    
};

#endif /* defined(__ofxRawHID__) */
