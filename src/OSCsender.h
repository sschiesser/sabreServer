/*
*  OSCsender.h
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

#ifndef sabreServer_OSCsender_h
#define sabreServer_OSCsender_h

#define MAXNUM 64

#include "threadedSerial.h"

#include "ofMain.h"
#include "ofxOsc.h"

class OSCsender
{
public:

    OSCsender(){
        sendIP = "127.0.0.1";
        sendport = -1;
        active = 0;
        ID = -1;
    };
    
	~OSCsender();
	
    void sendOSC(bool reset);
    
    ofxOscSender sender;
    ofxOscMessage m[MAXNUM];
    
    string  sendIP;
	int     sendport;
    bool    active;
    
//    threadedSerial * data;
    
    int ID;
};

#endif
