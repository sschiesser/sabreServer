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

#include "sabreServer.h"

void sabreServer::setup()
{
	ofSetEscapeQuitsApp(false);
	ofEnableAlphaBlending();
	titleString = "sabreServer version 0.96b";
	
	rawHIDobject = new(threadedHID);

	TTF.loadFont("lucidagrande.ttf", 8, 1, 1, 0);
	TTFsmall.loadFont("lucidagrande.ttf", 8, 1, 0, 0);
	rawHIDobject->TTF.loadFont("lucidagrande.ttf", 8, 1, 1, 0);
	
	texScreen.allocate(440, 700, GL_RGB);
	
	windowChanged = 1;
	drawValues = 0;
    rawHIDobject->drawValues = 0;
	menuState = 0;

	rawHIDobject->rawHID.deviceInfo.vendorID = 0x1C57;
	rawHIDobject->rawHID.deviceInfo.productID = 0x5ABE;
	rawHIDobject->rawHID.deviceInfo.manufacturerName= "ICST";
	rawHIDobject->rawHID.deviceInfo.productName= "SABRe";
    rawHIDobject->rawHID.deviceSelected = true;

	rawHIDobject->rawHID.deviceInfo.usagePage = 0xFFAB;
	rawHIDobject->rawHID.deviceInfo.usage = 0x0100;

	receiveport = 41001;
	rawHIDobject->debounceTimeOut = 0;
    
    for(int i = 0; i < NUMOSCSENDERS; i++) {
        rawHIDobject->sendIP[i] = "127.0.0.1";
        rawHIDobject->sendport[i] = 40002+i;
        rawHIDobject->senderActive[i] = false;
        rawHIDobject->senderMode[i] = 1;
    }

	status = readPrefs();	
	if(status) {
		status3 = "Success reading settings in \"sabreServer.xml\"";		
	}else{
		status3 = "Failed reading opening file \"sabreServer.xml\" ";
	}
	readMidicodes();
	lastTime = ofGetElapsedTimef();
	
	receiver.setup( receiveport );
	
//	framerate = 20;
	ofSetFrameRate( framerate ); // cap the glut callback rate
//	ofSetVerticalSync( true );
//	ofBackground( 224, 224, 224);
	ofBackground( 255, 255, 255);
	redrawFlag = 1;
	// redrawInterval = redrawValues[display]; // in seconds
	firstflag = 1;
	
	runOnce = 1;
	runOnceDelay = 2.0f;
	lastRedraw = runOnceStart = ofGetElapsedTimef();
	
 	getHIDDeviceList();
	
    // we don't start the serialThread automatically
    rawHIDobject->status = false;

//    startHID();
	
	ofSetWindowPosition(0,44);

//    dumpPrefs();
}


void sabreServer::update()
{
	receiveOSC();
	if(rawHIDobject->unplugged) {
		ofSystemAlertDialog("SABRe Receiver unplugged\nstopping now...");
		rawHIDobject->stop(); // the rawHID thread
		rawHIDobject->status = false;
		rawHIDobject->unplugged = false;
	}
}


void sabreServer::draw()
{
	int i;
	int anchorx = 15;
	int anchory = 66;
	int stepsize = 18;
	int columnwidth = 200;
	int rightColumn = 270;
	int leftColumn = 10;
	int width;
	int height;
	double yy;
    int pos_x;

	if(windowChanged == 1) {
		if(drawValues == 0) {
			width = 550;
			height = 52;
			ofSetWindowShape(width, height);
			windowChanged = 0;
		} else if(drawValues == 1) {
			width = 746;
			height = 514; // 790
			timeOut = 5.0;
			ofSetWindowShape(width, height);
			windowChanged = 0;
		}
	}
	
	if(redrawFlag == 1) // drawn once after first update
	{
		// header frame background
		ofFill();        
		ofSetColor(0, 0, 0, 1);
        ofRect(0, 0, width, 49);

		
		ofSetColor(255, 127, 0, 10);
		ofRect(0, 0, width, 3 * height);
		
		if(rawHIDobject->status) {
			ofSetColor(63, 63, 63, 255);
		} else {
			ofSetColor(127, 127, 127, 255);
		}
		TTFsmall.drawString(status1, anchorx, 18);
		TTFsmall.drawString(status2[whichStatus], anchorx, 42);

		// separator lines
//		ofSetColor(240, 240, 240, 127);
//		ofLine(0, anchory-14, width, anchory-14);	
//		ofSetColor(127, 127, 127, 127);
//		ofLine(0, anchory-13, width, anchory-13);		
		
		ofSetColor(200, 200, 200, 255);
		
		// Menu
//		ofFill();
//		ofSetColor(232, 232, 232);
//		ofRect(leftColumn, 3, 188, 18);
//		ofNoFill();
//		ofSetColor(127, 127, 127);
//		ofRect(leftColumn, 3, 188, 18);
//		ofFill();
//		ofTriangle(leftColumn+177,7,leftColumn+185, 7, leftColumn+181, 13);
//		ofSetColor(0, 0, 0);
//		TTFsmall.drawString(rawHIDobject->serialport,leftColumn+5, 16);
		

		// start/stop button
		if(rawHIDobject->status) {
			ofFill();
			ofSetColor(255, 255, 255);
			ofRect(rightColumn, 3, 124, 18);
			ofNoFill();
			ofSetColor(127, 127, 127);
			ofRect(rightColumn, 3, 124, 18);
			ofSetColor(0, 0, 0);
			TTFsmall.drawString("Stop", rightColumn+48, 16);
		} else {
			ofFill();
			ofSetColor(232, 232, 232);
			ofRect(rightColumn, 3, 124, 18);
			ofNoFill();
			ofSetColor(127, 127, 127);
			ofRect(rightColumn, 3, 124, 18);
			ofSetColor(0, 0, 0);
			TTFsmall.drawString("Start", rightColumn+48, 16);
		}
		
		// show values button
        if( !drawValues ) {
            ofFill();
            ofSetColor(232, 232, 232);
            ofRect(rightColumn+126, 3, 124, 18);
            ofNoFill();
            ofSetColor(127, 127, 127);
            ofRect(rightColumn+126, 3, 124, 18);
            ofSetColor(0, 0, 0);		
            TTFsmall.drawString("Show Values", rightColumn+28+126, 16);
        }else{
            ofFill();
            ofSetColor(255, 255, 255);
            ofRect(rightColumn+126, 3, 124, 18);
            ofNoFill();
            ofSetColor(127, 127, 127);
            ofRect(rightColumn+126, 3, 124, 18);
            ofSetColor(0, 0, 0);
            TTF.drawString("Hide Values", rightColumn+32+126, 16);
		}
		// Calibrate Button
		ofFill();
		ofSetColor(232, 232, 232);
		ofRect(375, 480, 124, 20);
		ofNoFill();
		ofSetColor(127, 127, 127);
		ofRect(375, 480, 124, 20);
		ofSetColor(0, 0, 0);
		TTFsmall.drawString("Calibrate Keys", 375+20, 480+14);
        
        // Calibrate Button
		ofFill();
		ofSetColor(232, 232, 232);
		ofRect(502, 480, 124, 20);
		ofNoFill();
		ofSetColor(127, 127, 127);
		ofRect(502, 480, 124, 20);
		ofSetColor(0, 0, 0);
		TTFsmall.drawString("Calibrate Air", 500+24, 480+14);

		// value display left column
		
        ofFill();
		for(i = 0; i < 25; i++) { // stripes
			if((i % 2) == 0){
				ofSetColor(255, 127, 0, 10);
				ofRect(anchorx-2, anchory+((i-1) * stepsize)+7, 94, 16);
			}			
		}
        for(i = 0; i < 9; i++) { // stripes
			if((i % 3) == 0){
				ofSetColor(255, 127, 0, 10);
				ofRect(anchorx-2 + 360, anchory+((i-1) * stepsize)+7, 144, 16);
			}
		}
        ofFill(); // heading
		ofSetColor(255, 127, 0, 10);
        ofRect(anchorx-2 + 360, anchory+((8) * stepsize)+7, 144, 16);
        
        ofFill(); // button
		ofSetColor(255, 127, 0, 10);
        ofRect(anchorx-2 + 360, anchory+((11) * stepsize)+7, 144, 16);
        
        ofFill(); // pressure
		ofSetColor(255, 127, 0, 10);
        ofRect(anchorx-2 + 360, anchory+((13) * stepsize)+7, 144, 16);
        
        ofFill(); // keycode
		ofSetColor(255, 127, 0, 10);
        ofRect(anchorx-2 + 360, anchory+((15) * stepsize)+7, 144, 16);
        
		ofSetColor(0, 0, 0, 191);
        
		for(i = 0; i < 25; i++) { // key addresses
			TTFsmall.drawString(rawHIDobject->keys[i].oscaddress, anchorx, anchory+((i) * stepsize) );
		}
		for(i = 0; i < 9; i++) { // imu addresses
			std::string str = rawHIDobject->imuaddresses[i/3];
			std::string::size_type end = str.find_last_of('/');
			if(end != str.npos)
				str = str.substr(0, end);
			TTFsmall.drawString(str, anchorx + 360, anchory+((i) * stepsize) );
//			TTFsmall.drawString(str, anchorx + 360, anchory+((i+25) * stepsize) );
//			TTFsmall.drawString(rawHIDobject->imuaddresses[i/3], anchorx, anchory+5+((i+25) * stepsize) );
		}
        
        TTFsmall.drawString(rawHIDobject->imuaddresses[10], anchorx + 360, anchory+((9) * stepsize) );
        TTFsmall.drawString(rawHIDobject->imuaddresses[11], anchorx + 360, anchory+((10) * stepsize) );
        
        
		for(i = 0; i < 1; i++) { // first button address truncated
			char temp[64];
			strncpy(temp, rawHIDobject->buttonaddresses[0].c_str(), rawHIDobject->buttonaddresses[0].size()-2);
			temp[rawHIDobject->buttonaddresses[0].size()-2] = 0;
//			TTFsmall.drawString(temp, anchorx + 360, anchory+((i+35) * stepsize) );
			TTFsmall.drawString(temp, anchorx + 360, anchory+((i+12) * stepsize) );
		}
        
        // air addresses
		TTFsmall.drawString(rawHIDobject->airaddresses[0], anchorx + 360, anchory+(14 * stepsize) );
        
        // air addresses
		TTFsmall.drawString(rawHIDobject->keycodeaddress, anchorx + 360, anchory+(16 * stepsize) );
        TTFsmall.drawString(rawHIDobject->midinoteaddress, anchorx + 360, anchory+(17 * stepsize) );
      
        
		texScreen.loadScreenData(0,0, 440, 700);
		drawTex = 1;
	} else {
		if(drawTex) {
			texScreen.draw(0, 0, 440, 266);
			drawTex = 0;
		}
	}
#pragma mark draw values    
    
	anchorx = 12;
	anchory = 66;
	leftColumn = 110;
	int midColumn = 150;
	rightColumn = 220;
	int farRightColumn = 330;
    
    int imuColumnLeft = 410;
	
	stepsize = 18;
	columnwidth = 180;
	width = 430;
	height = 635;
    
    if (status == 1 && drawValues)
    {
        //			ofFill();
        //			ofSetColor(200, 200, 200, 255);
        //			ofRect(leftColumn-1, 79, width-leftColumn-5, height-anchory-15);
        ofSetColor(0, 127, 255, 255);
        
        for(i = 0; i < 25; i++) { // stripes
            if((i % 2) == 0){
                ofFill();
				ofSetColor(255, 127, 0, 10);
                ofRect(leftColumn-1, anchory+((i-1) * stepsize)+7, width-leftColumn-85, 16);
                ofSetColor(0, 0, 0, 255);
            }
        }
        
        for(i = 0; i < 9; i++) { // stripes
            if((i % 3) == 0){
                ofFill();
				ofSetColor(255, 127, 0, 10);
                ofRect(leftColumn-1 + imuColumnLeft, anchory+((i-1) * stepsize)+7, width-leftColumn-103, 16);
                ofSetColor(0, 0, 0, 255);
            }
        }
        
        ofFill(); // heading
		ofSetColor(255, 127, 0, 10);
        ofRect(leftColumn-1 + imuColumnLeft, anchory+((8) * stepsize)+7, width-leftColumn-103, 16);
        
        ofFill(); // button
		ofSetColor(255, 127, 0, 10);
        ofRect(leftColumn-1 + imuColumnLeft, anchory+((11) * stepsize)+7, width-leftColumn-103, 16);
        
        ofFill(); // pressure
		ofSetColor(255, 127, 0, 10);
        ofRect(leftColumn-1 + imuColumnLeft, anchory+((13) * stepsize)+7, width-leftColumn-103, 16);
        
        ofFill(); // keycode
		ofSetColor(255, 127, 0, 10);
        ofRect(leftColumn-1 + imuColumnLeft, anchory+((15) * stepsize)+7, width-leftColumn-103, 16);
        
        for(i = 0; i < 25; i++) { // keys
            ofSetColor(0, 0, 0, 255);
            yy = anchory+(i * stepsize);
            TTF.drawString(ofToString(rawHIDobject->keys[i].raw, 6), leftColumn, yy );
            //                printf("%lx ", keys[i].raw);
            TTF.drawString(ofToString(rawHIDobject->keys[i].continuous, 6), midColumn, yy);
            ofNoFill();
            ofSetColor(91, 91, 91, 255);
            ofRect(rightColumn, yy-9, 104, 12);
            ofFill();
            ofSetColor(0, 0, 0, 127);
            if(rawHIDobject->calibrate[i]) {
                ofSetColor(255, 127, 0, 191);
                ofRect( rightColumn + (104 * rawHIDobject->keys[i].minimum * rawHIDobject->scale10), yy-7, (104 * (rawHIDobject->keys[i].maximum - rawHIDobject->keys[i].minimum) * rawHIDobject->scale10), 9);
                ofSetColor(0, 0, 0, 255);
                ofRect( rightColumn + (104 * (rawHIDobject->keys[i].raw * rawHIDobject->scale10)), yy-9, 2, 12);
                
            } else {
                ofRect( rightColumn + (104 * rawHIDobject->keys[i].continuous), yy-9, 2, 12);
                ofSetColor(91, 91, 91, 255);
                ofLine(rightColumn + (104 * rawHIDobject->keys[i].threshDown), yy-9, rightColumn + (104 * rawHIDobject->keys[i].threshDown), yy+4);
                ofLine(rightColumn + (104 * rawHIDobject->keys[i].threshUp), yy-9, rightColumn + (104 * rawHIDobject->keys[i].threshUp), yy+4);
            }
            // draw binary boxes
            ofNoFill();
            ofSetColor(91, 91, 91, 255);
            ofRect(farRightColumn, yy-9, 12, 12);
            
            if(rawHIDobject->keys[i].binary) {
                ofFill();
                ofSetColor(0, 0, 0, 255);
                ofRect(farRightColumn+2, yy-6, 7, 7);
            }
            // individual toggles
            if(rawHIDobject->calibrateSwitch) {
                if(rawHIDobject->calibrateSingle) {
                    
                    if(rawHIDobject->calibrate[i]){
                        ofFill();
                        ofSetColor(255,127,0, 191);
                        ofRect(rightColumn +126, yy-9, 16, 12);
                    }
                    ofNoFill();
                    ofSetColor(0,0,0);
                    ofRect(rightColumn +126, yy-9, 16, 12);
                    TTF.drawString("c", rightColumn+130, yy+1);
                }
            }
        }
        
        for(i = 0; i < 9; i++) { // imu
            ofSetColor(0, 0, 0, 255);
            //				yy = anchory+((i+25) * stepsize);
            yy = anchory+((i) * stepsize);
            TTF.drawString( ofToString(rawHIDobject->raw[i], 6) , leftColumn + imuColumnLeft, yy );
            TTF.drawString(ofToString(rawHIDobject->IMU[i], 6), midColumn  + 10 + imuColumnLeft, yy);
            ofNoFill();
            ofSetColor(91, 91,91, 255);
            ofRect(rightColumn + imuColumnLeft, yy-9, 104, 12);
            ofFill();
            ofSetColor(0, 0, 0, 255);
            ofRect( rightColumn + imuColumnLeft + (104 * rawHIDobject->IMU[i]), yy-9, 2, 12);
            ofNoFill();
            ofSetColor(91, 91, 91, 255);
            ofLine(rightColumn+52 + imuColumnLeft, yy-9, rightColumn+52 + imuColumnLeft, yy+4);
        }
        
        // heading
        ofSetColor(0, 0, 0, 255);
        yy = anchory+((9) * stepsize);
        TTF.drawString( ofToString(rawHIDobject->heading, 2) , midColumn  + 10 + imuColumnLeft, yy );
        ofNoFill();
        ofSetColor(91, 91,91, 255);
        ofRect(rightColumn + imuColumnLeft, yy-9, 104, 12);
        ofFill();
        ofSetColor(0, 0, 0, 255);
        ofRect( rightColumn + imuColumnLeft + 52 + (rawHIDobject->heading/3.46153846153846), yy-9, 2, 12);
        ofNoFill();
        ofSetColor(91, 91, 91, 255);
        ofLine(rightColumn+52 + imuColumnLeft, yy-9, rightColumn+52 + imuColumnLeft, yy+4);

        // tilt
        ofSetColor(0, 0, 0, 255);
        yy = anchory+((10) * stepsize);
        TTF.drawString( ofToString(rawHIDobject->tilt, 2) , midColumn  + 10 + imuColumnLeft, yy );
        ofNoFill();
        ofSetColor(91, 91,91, 255);
        ofRect(rightColumn + imuColumnLeft, yy-9, 104, 12);
        ofFill();
        ofSetColor(0, 0, 0, 255);
//        ofRect( rightColumn + imuColumnLeft + 52 + (52 * rawHIDobject->tilt), yy-9, 2, 12);
        ofRect( rightColumn + imuColumnLeft + (104 * rawHIDobject->tilt), yy-9, 2, 12);
        ofNoFill();
        ofSetColor(91, 91, 91, 255);
//        ofLine(rightColumn+52 + imuColumnLeft, yy-9, rightColumn+52 + imuColumnLeft, yy+4);
        
        // air
        ofSetColor(0, 0, 0, 255);
        //			yy = anchory+(34 * stepsize);
        yy = anchory+(14 * stepsize);
        TTF.drawString(ofToString(rawHIDobject->air[0], 2), leftColumn + imuColumnLeft, yy );
        TTF.drawString(ofToString(rawHIDobject->airValue.continuous, 2), midColumn  + 10 + imuColumnLeft, yy);
        
        TTF.drawString(ofToString(rawHIDobject->keycode, 2), midColumn  + 10 + imuColumnLeft, anchory+(16 * stepsize) );
        if(rawHIDobject->validMidiNote) {
            TTF.drawString(ofToString(rawHIDobject->midinote, 2), midColumn  + 10 + imuColumnLeft, anchory+(17 * stepsize));
        }
        ofNoFill();
        ofSetColor(91, 91, 91, 255);
        ofRect(rightColumn + imuColumnLeft, yy-9, 104, 12);
        ofFill();
        ofSetColor(0, 0, 0, 127);
        ofRect( rightColumn + imuColumnLeft + (104 * (CLAMP( ((rawHIDobject->airLong[0] - 500.0) * 0.001), 0, 1))), yy-9, 2, 12);
        
        if(rawHIDobject->airValue.calibratePressureRange) {
            ofSetColor(255, 224, 0, 191);
            ofRect( rightColumn + imuColumnLeft, yy-7, (103), 9);
            // TODO figure scaling for the rangebars
            ofSetColor(0, 0, 0, 255);
            ofRect( rightColumn + imuColumnLeft + (104 * (CLAMP( ((rawHIDobject->airValue.continuous - 500.0) * 0.001), 0, 1))), yy-9, 2, 12);
        } else {
            if(rawHIDobject->airValue.calibrationFlag){
                ofFill();
                ofSetColor(255, 0, 0, 255);
                ofRect(rightColumn + imuColumnLeft, yy-9, 104, 12);
            }else{
                ofNoFill();
                ofSetColor(91, 91, 91, 255);
                ofRect(rightColumn + imuColumnLeft, yy-9, 104, 12);
            }
            ofFill();
            ofSetColor(0, 0, 0, 127);
            ofRect( rightColumn + imuColumnLeft + CLAMP((104 * rawHIDobject->airValue.continuous), 0, 104), yy-9, 2, 12);
        }
        
        // buttons
        ofSetColor(0, 0, 0, 255);
        //			yy = anchory+((35) * stepsize);
        yy = anchory+((12) * stepsize);
        TTF.drawString( ofToString(rawHIDobject->button[0], 1), midColumn + 10 + imuColumnLeft, yy );
        TTF.drawString( ofToString(rawHIDobject->button[1], 1), midColumn + 10 + 12 + imuColumnLeft, yy );
        TTF.drawString( ofToString(rawHIDobject->button[2], 1), midColumn + 10 + 24 + imuColumnLeft, yy );
        
        ofNoFill();
        ofSetColor(91, 91, 91, 255);
        ofRect(rightColumn + imuColumnLeft, yy-9, 12, 12);
        ofRect(rightColumn + imuColumnLeft + 14 , yy-9, 12, 12);
        ofRect(rightColumn + imuColumnLeft + 28 , yy-9, 12, 12);
        
        ofFill();
        ofSetColor(0, 0, 0, 255);
        if(rawHIDobject->button[0]) {
            ofRect(rightColumn + imuColumnLeft + 2, yy-6, 7, 7);
        }
        if(rawHIDobject->button[1]) {
            ofRect(rightColumn + imuColumnLeft + 16, yy-6, 7, 7);
        }
        if(rawHIDobject->button[2]) {
            ofRect(rightColumn + imuColumnLeft + 30, yy-6, 7, 7);
        }
        
        // battery
        //            ofSetColor(0, 0, 0, 255);
        ////			yy = anchory+((36) * stepsize);
        //            yy = 40;
        //			TTF.drawString( "main: "+ofToString((int)(batteryLevelRight*12.5))+"%", anchorx+82 + 360, yy );
        //			TTF.drawString( "mouthpiece: "+ofToString((int)(batteryLevelAir*12.5))+"%", leftColumn+12 + 360, yy );
        
        if(rawHIDobject->calibrateSwitch) {
            ofFill();
            ofSetColor(255, 127, 0);
            ofRect(375, 480, 124, 20);
            ofNoFill();
            ofSetColor(127, 127, 127);
            ofRect(375, 480, 124, 20);
            ofSetColor(0, 0, 0);
            TTF.drawString("Calibrating Keys", 375+12, 480+14);
            
            if(rawHIDobject->calibrateSingle == 0) {
                ofFill();
                ofSetColor(255, 127, 0);
                ofRect(375, 458, 124, 20);
                ofNoFill();
                ofSetColor(127, 127, 127);
                ofRect(375, 458, 124, 20);
                ofSetColor(0, 0, 0);
                TTF.drawString("Calibrate All...", 375+24, 458+14);
            }else{
                ofNoFill();
                ofSetColor(127, 127, 127);
                ofRect(375, 458, 124, 20);
                ofSetColor(0, 0, 0);
                TTF.drawString("Calibrate All Keys", 375+10, 458+14);
            }
            
            ofNoFill();
            ofSetColor(127, 127, 127);
            ofRect(375, 436, 124, 20);
            ofSetColor(0, 0, 0);
            TTF.drawString("Reset Key Calibr.", 375+12, 436+14);
        }
        
        if(rawHIDobject->airValue.calibratePressureRange) {
            ofFill();
            ofSetColor(255, 224, 0);
            ofRect(502, 480, 124, 20);
            ofNoFill();
            ofSetColor(127, 127, 127);
            ofRect(502, 480, 124, 20);
            ofSetColor(0, 0, 0);
            TTF.drawString("Calibrating Air", 502+12, 480+14);
        }

    } else {
        //			str = "can't lock!\neither an error\nor the thread has stopped";
        ofLog(OF_LOG_ERROR, "SabreServer: couldn't start HID Thread !! can't lock!\neither an error\nor the thread has stopped");
    }





#pragma mark draw levels
    
    ofSetColor(63, 63, 63, 255);
    TTFsmall.drawString( "battery: main       air", 280, 34 );
    
    // battery display
    for(i = 0; i < 15; i++){
        pos_x = 360;
        if(rawHIDobject->batteryLevelRight*6.667 >= (i * 6.667) ) {
            ofSetColor(127, 127, 127);
            ofRect(pos_x+i*2, 25, 2, 10);
        }
    }
    //    ofRect
    for(i = 0; i < 15; i++){
        pos_x = 425;
        if(rawHIDobject->batteryLevelAir*6.667 >= (i * 6.667) ) {
            ofSetColor(127, 127, 127);
            ofRect(pos_x+i*2, 25, 2, 10);
        }
    }
    ofSetColor(63, 63, 63, 255);
    ofNoFill();
    ofRect(360, 25, 31, 10);
    ofRect(425, 25, 31, 10);
    ofRect(391, 27, 2, 6);
    ofRect(456, 27, 2, 6);
    
    ofFill();
    
    
    ofSetColor(63, 63, 63, 255);
    TTFsmall.drawString( "wireless: left       right      air", 280, 48 );
    
    for(i = 0; i < 8; i++){
        pos_x = 360;
        if( (CLAMP(rawHIDobject->linkQualityLeft, 0, 205) - 0 ) >= (i * 18) ) {
            ofSetColor(127, 127, 127, 255);
        }else{
            ofSetColor(212, 212, 212, 255);
            
        }
        ofRect(pos_x+i*4, 36+(10-i), 2, 2+i);
    }
    for(i = 0; i < 8; i++){
        pos_x = 430;
        if( (CLAMP(rawHIDobject->linkQualityRight, 0, 205) - 0 ) >= (i * 18) ) {
            ofSetColor(127, 127, 127, 255);
        }else{
            ofSetColor(212, 212, 212, 255);
            
        }
        ofRect(pos_x+i*4, 36+(10-i), 2, 2+i);
    }
    for(i = 0; i < 8; i++){
        pos_x = 485;
        if( (CLAMP(rawHIDobject->linkQualityAir, 0, 205) - 0 ) >= (i * 18) ) {
            ofSetColor(127, 127, 127, 255);
        }else{
            ofSetColor(212, 212, 212, 255);
            
        }
        ofRect(pos_x+i*4, 36+(10-i), 2, 2+i);
    }
    
    //    rawHIDobject->batteryLevelAir*6.25
    //    rawHIDobject->batteryLevelRight*6.25
    //
    //    rawHIDobject->linkQualityLeft*0.390625
    //    rawHIDobject->linkQualityRight*0.390625
    //    rawHIDobject->linkQualityAir*0.390625
    
//	if(drawValues) {
//		rawHIDobject->draw();
//	}
	/*
	if(menuState) {
		numMenuItems = (int)rawHIDobject->deviceList.size() / 2;

		ofSetColor(232, 232, 232);
		ofFill();
		ofRect(leftColumn, 21, 188, numMenuItems*18);

		vector<ofSerialDeviceInfo>::iterator it;
		int i;
		for ( it = rawHIDobject->deviceList.begin(), i = 0 ; it < rawHIDobject->deviceList.end(); it++) {
			if(!strncmp(it->getDeviceName().c_str(), "tty", 3)){
				ofNoFill();
				ofSetColor(232, 232, 232);
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
	*/
    
}

void sabreServer::exit()
{
	free(rawHIDobject);
	
	XML.pushTag("sabre", 0);
	XML.removeTag("display", 0);
	XML.setValue("display", drawValues, 0);
	XML.popTag();
	XML.saveFile("sabreServer.xml");
	return;
	// printf("exit\n");
}

void sabreServer::startHID()
{
    ofLog(OF_LOG_NOTICE, "starting HID thread");
	if(rawHIDobject->rawHID.deviceSelected) {
        
        if(rawHIDobject->rawHID.isOpen() ) {
            rawHIDobject->rawHID.closeDevice();
			
        }
        
		rawHIDobject->status = rawHIDobject->rawHID.openDevice();

        if(rawHIDobject->status == 0) {
            status1 = "RawHID Device open: " + rawHIDobject->rawHID.deviceInfo.manufacturerName + " " + rawHIDobject->rawHID.deviceInfo.productName;
			
            rawHIDobject->start(); // the HID thread
			rawHIDobject->unplugged = false;
			ofLog(OF_LOG_NOTICE,"opened rawHID thread");
        } else {
            status1 = "Unable to open ";
            rawHIDobject->stop(); // the thread
//            ofSystemAlertDialog("SABRe Server \n"+status1 + rawHIDobject->rawHID.deviceInfo.manufacturerName + " " + rawHIDobject->rawHID.deviceInfo.productName);
			ofLog(OF_LOG_ERROR,"failed to open rawHID thread");

        }
        redrawFlag = 1;
    }
}

void sabreServer::stopHID()
{
    if(rawHIDobject->status) {
		rawHIDobject->rawHID.closeDevice();
		rawHIDobject->stop(); // the serial thread
		rawHIDobject->status = false;
	}
	redrawFlag = 1;
}

void sabreServer::getHIDDeviceList()
{
  	rawHIDobject->rawHID.listDevices();
    ofLog(OF_LOG_NOTICE, "Raw HID devices found: ");
	vector<ofxHIDDeviceInfo>::iterator it;
	for ( it = rawHIDobject->rawHID.HID_devices.begin() ; it < rawHIDobject->rawHID.HID_devices.end(); it++) {
        printf("[notice ]   0X%04lX 0x%04lX \"%s\" \"%s\"\n",
			   it->vendorID,
			   it->productID,
			   it->manufacturerName.c_str(),
			   it->productName.c_str()
			   );
	}
}

void sabreServer::startOSC()
{
	if(rawHIDobject->status) {
        rawHIDobject->stop(); // stops the thread, deletes the object which also closes the socket
	}
    
    for(int i = 0; i < NUMOSCSENDERS; i++) {
        if(rawHIDobject->senderActive[i]) {
            // open an outgoing connection to sendIP:PORT
            rawHIDobject->status = rawHIDobject->sender[i].setup( rawHIDobject->sendIP[i].c_str(), rawHIDobject->sendport[i]);
            if(rawHIDobject->status) {
                rawHIDobject->start(); // the OSC thread
                
                status2[i] = "OSC-stream "+ofToString(i+1)+": IP "+rawHIDobject->sendIP[i]+" Port "+ofToString(rawHIDobject->sendport[i])+" on";
            } else {
                ofSystemAlertDialog("Sabre Server Unable to open Network "+rawHIDobject->sendIP[0]+" on port "+ofToString(rawHIDobject->sendport[i]));
                status2[i] = "Unable to open Network "+rawHIDobject->sendIP[i]+" on port "+ofToString(rawHIDobject->sendport[i]);
            }
        }else{
            status2[i] = "OSC-stream "+ofToString(i+1)+" OFF - "+rawHIDobject->sendIP[i]+" / "+ofToString(rawHIDobject->sendport[i]);
        }
    }
    
	redrawFlag = 1;
}

void sabreServer::stopOSC()
{
	if(rawHIDobject->status) {
        rawHIDobject->stop(); // stops the thread, deletes the object which also closes the socket
		rawHIDobject->status = false;
	}
	redrawFlag = 1;
}

void sabreServer::receiveOSC()
{
	int i;
	string temp;
	
	while( receiver.hasWaitingMessages()) {
		ofxOscMessage m;
		receiver.getNextMessage( &m );
		temp = m.getAddress();
		
		if ( !strcmp( temp.c_str(), "/sabre/framerate" )) {
//			framerate = m.getArgAsInt32( 0 );
//			status2 = "sending OSC to "+rawHIDobject->sendIP+" on port "+ofToString(rawHIDobject->sendport);
		}else if ( strcmp( temp.c_str(), "/sabre/display" ) == 0 ) {
			display = m.getArgAsInt32( 0 );
			windowChanged = 1;
		}else if ( strcmp( temp.c_str(), "/sabre/reset" ) == 0 ) {
		}else if ( strcmp( temp.c_str(), "/sabre/writePrefs" ) == 0 ) {
			sabreServer::writePrefs();
		}else if ( strcmp( temp.c_str(), "/sabre/readPrefs" ) == 0 ) {
			sabreServer::readPrefs();
		}else if ( strcmp( temp.c_str(), "/sabre/network/receiver/port" ) == 0 ) {
			receiveport = m.getArgAsInt32( 0 );
			receiver.setup( receiveport );
		}else if ( strcmp( temp.c_str(), "/sabre/network/sender/port" ) == 0 ) {
			rawHIDobject->sendport[0] = m.getArgAsInt32( 0 );
            // TODO switch between fulspeed or OSc thread
//			rawHIDobject->sender.setup( rawHIDobject->sendIP, rawHIDobject->sendport );
//			status2 = "sending OSC to "+rawHIDobject->sendIP+" on port "+ofToString(rawHIDobject->sendport);
		}else if ( strcmp( temp.c_str(), "/sabre/network/sender/IP" ) == 0 ) {
			rawHIDobject->sendIP[0] = m.getArgAsString(0);
            // TODO switch between fulspeed or OSc thread
//			rawHIDobject->sender.setup( m.getArgAsString(0), rawHIDobject->sendport );
//			status2 = "sending OSC to "+rawHIDobject->sendIP+" on port "+ofToString(rawHIDobject->sendport);
		}else if ( strcmp( temp.c_str(), "/sabre/exit" ) == 0 ) {
			sabreServer().exit();
		}else if ( strcmp( temp.c_str(), "/sabre/calibrateSwitch" ) == 0 ) {
			if(rawHIDobject->calibrateSwitch == 1) { // before we switch it off
                
				for(i = 0; i < MAXNUM; i++) {
					rawHIDobject->calibrate[i] = 0;
				}
				writeScaling(); // we write the values into the prefs
                
			}
			rawHIDobject->calibrateSwitch = !rawHIDobject->calibrateSwitch;
			for(i = 0; i < MAXNUM; i++) {
				rawHIDobject->calibrate[i] = !rawHIDobject->calibrate[i];
			}
//			printf("calibrate is %d\n", rawHIDobject->calibrateSwitch);
            
		}else if ( strcmp( temp.c_str(), "/sabre/calibrate" ) == 0 ) {
			int which = m.getArgAsInt32( 0 );
			rawHIDobject->calibrate[which] = m.getArgAsInt32( 1 );
			if(rawHIDobject->calibrate[which] == 1) { // reset calibration values
				rawHIDobject->keys[which].minimum = 1023;
				rawHIDobject->keys[which].maximum = 0;
			}
			
			if(rawHIDobject->calibrate[which] == 0) { // before we switch it off
				writeScaling(); // we write the values into the prefs
			}
//			printf("calibrate[%d] is %d\n", which, rawHIDobject->calibrate[which]);
		}else if ( strcmp( temp.c_str(), "/sabre/calibrate/air" ) == 0 ) {
			int which = m.getArgAsInt32( 0 );
			rawHIDobject->airValue.calibratePressureRange = m.getArgAsInt32( 1 );
			if(rawHIDobject->airValue.calibratePressureRange == 1) { // reset calibration values
				rawHIDobject->keys[which].minimum = 32768;
				rawHIDobject->keys[which].maximum = -32768;
			}
			
			if(rawHIDobject->airValue.calibratePressureRange == 0) { // before we switch it off
				writeScaling(); // we write the values into the prefs
			}
//			printf("calibrateAir is %d\n", rawHIDobject->airValue.calibratePressureRange);
		}
	}
}

bool sabreServer::readPrefs()
{	
	int numTags;
	int numPtTags;
	int totalToRead;
	int i;
	int ID;
	int bitwidth;
	
	bool result = XML.loadFile("sabreServer.xml");
	if(result) {
		
		string VID = XML.getValue("sabre:rawHID:VID", "0x1C57");
		string PID = XML.getValue("sabre:rawHID:PID", "0x5ABE");
        rawHIDobject->rawHID.deviceInfo.vendorID = strtol(VID.c_str(), NULL, 16);
        rawHIDobject->rawHID.deviceInfo.productID = strtol(PID.c_str(), NULL, 16);
//		string usePg = XML.getValue("sabre:rawHID:usagepage", "0xFFAB");
//		string usg = XML.getValue("sabre:rawHID:usage", "0x0100");
//        rawHIDobject->rawHID.deviceInfo.usagePage = strtol(usePg.c_str(), NULL, 16);
//        rawHIDobject->rawHID.deviceInfo.usage = strtol(usg.c_str(), NULL, 16);
		rawHIDobject->rawHID.deviceInfo.manufacturerName = XML.getValue("sabre:rawHID:manufacturername", "ICST");
        rawHIDobject->rawHID.deviceInfo.productName = XML.getValue("sabre:rawHID:productname", "SABRe");
        
		rawHIDobject->sendRawValues = XML.getValue("sabre:OSCsender:sendRawValues", 0);
        rawHIDobject->OSCsendingInterval = XML.getValue("sabre:OSCsender:interval", 3);
        rawHIDobject->OSCsendingInterval = MAX(rawHIDobject->OSCsendingInterval, 3); // clip to 3
        
        rawHIDobject->numOSCloops = rawHIDobject->OSCsendingInterval * 2;
		
		framerate = XML.getValue("sabre:framerate", 20);

		
		numTags = XML.getNumTags("sabre:network:sender");
		if(numTags > 0) {
			XML.pushTag("sabre", numTags-1);
            XML.pushTag("network", numTags-1);

			numPtTags = XML.getNumTags("sender");
			if(numPtTags > 0) {
				totalToRead = MIN(numPtTags, NUMOSCSENDERS);
				for(i = 0; i < totalToRead; i++) {
					ID = XML.getAttribute("sender", "id", 0, i);
					ID--;
                    rawHIDobject->sendIP[ID] = XML.getValue("sender:IP", "127.0.0.1", i);
                    rawHIDobject->sendport[ID] = XML.getValue("sender:port", 40002+ID, i);
                    rawHIDobject->senderActive[ID] = XML.getValue("sender:active", 0, i);
                    rawHIDobject->senderMode[ID] = XML.getValue("sender:mode", 1, i);
				}
			}
            XML.popTag();
            XML.popTag();
        }
        rawHIDobject->calcResetID();

		receiveport = XML.getValue("sabre:network:receiver:port", 40001);

		drawValues = XML.getValue("sabre:display", 0);
        rawHIDobject->drawValues = drawValues = CLAMP(drawValues, 0, 1);
		
		rawHIDobject->threshDown = XML.getValue("sabre:thresholds:down", 0.2);
		rawHIDobject->threshUp = XML.getValue("sabre:thresholds:up", 0.8);

		for(i = 0; i < MAXNUM; i++) {
			rawHIDobject->keys[i].threshDown = rawHIDobject->threshDown;
			rawHIDobject->keys[i].threshUp = rawHIDobject->threshUp;
		}
		
		rawHIDobject->debounceTimeOut = XML.getValue("sabre:debounce-timeout", 0);
		rawHIDobject->accelResolution = XML.getValue("sabre:accel-resolution", 4);
		
		
		// 	4g/8g/16g 10/11/12/13bit resolution
		//  2g (10bit) : 512, 4g (11bit) : 1024, 8g (12bit) :  2048, 16g (13bit) : 4096
		if(rawHIDobject->accelResolution == 2){
			bitwidth = 10;
		} else if(rawHIDobject->accelResolution == 4){
			bitwidth = 11;
		} else if(rawHIDobject->accelResolution == 8){
			bitwidth = 12;
		} else if(rawHIDobject->accelResolution == 16){
			bitwidth = 13;			
		}
		
        // v3.5 comm structure
//		double rangeVal = pow((double)2.0, (double)bitwidth);
//		rawHIDobject->accelOffset = rangeVal * 0.5;
//		rawHIDobject->accelScale = 1.0 / rangeVal;
				
		numTags = XML.getNumTags("sabre:key");
		if(numTags > 0)
		{
			XML.pushTag("sabre", numTags-1);
			
			rawHIDobject->numKeyAddr = numPtTags = XML.getNumTags("key"); // key tags
			if(numPtTags > 0) {
				totalToRead = MIN(numPtTags, 64);
				for(i = 0; i < totalToRead; i++) {
					ID = XML.getAttribute("key", "id", 0, i);
					ID--;
					str1 = XML.getValue("key:oscaddress", "/sabre/key/0", i);
					if(str1.length() > 0) {
						rawHIDobject->keys[ID].oscaddress = str1;
					}
					rawHIDobject->keys[ID].inverted = XML.getValue("key:invert", 0, i);
					rawHIDobject->keys[ID].minimum = XML.getValue("key:minimum", 0, i);
					rawHIDobject->keys[ID].maximum = XML.getValue("key:maximum", 1023, i);
//					keys[ID].range = keys[ID].maximum - keys[ID].minimum;
					if(rawHIDobject->keys[ID].maximum != rawHIDobject->keys[ID].minimum) {
						rawHIDobject->keys[ID].scale = 1.0 / (rawHIDobject->keys[ID].maximum - rawHIDobject->keys[ID].minimum);
					}else{
						rawHIDobject->keys[ID].scale = 0.0;
					}
					rawHIDobject->keys[ID].threshUp = XML.getValue("key:threshold:up", rawHIDobject->threshUp, i);
					rawHIDobject->keys[ID].threshDown = XML.getValue("key:threshold:down", rawHIDobject->threshDown, i);
				}
			}
			 
			rawHIDobject->numImuAddr = numPtTags = XML.getNumTags("imu"); // imu tags
			if(numPtTags > 0) {
				totalToRead = MIN(numPtTags, 12);
				for(i = 0; i < totalToRead; i++) {
					ID = XML.getAttribute("imu", "id", 0, i);

					str1 = XML.getValue("imu:oscaddress", "/sabre/motion/0", i);
					if(str1.length() > 0) {
						rawHIDobject->imuaddresses[ID] = str1;
					}
				}
			}
            
            rawHIDobject->raw[0] = 0;
            rawHIDobject->IMU[0] = 0.0f;
            rawHIDobject->raw[1] = 0;
            rawHIDobject->IMU[1] = 0.0f;
            rawHIDobject->raw[2] = 0;
            rawHIDobject->IMU[2] = 0.0f;
            rawHIDobject->raw[3] = 0;
            rawHIDobject->IMU[3] = 0.0f;
            rawHIDobject->raw[4] = 0;
            rawHIDobject->IMU[4] = 0.0f;
            rawHIDobject->raw[5] = 0;
            rawHIDobject->IMU[5] = 0.0f;
            rawHIDobject->raw[6] = 0;
            rawHIDobject->IMU[6] = 0.0f;
            rawHIDobject->raw[7] = 0;
            rawHIDobject->IMU[7] = 0.0f;
            rawHIDobject->raw[8] = 0;
            rawHIDobject->IMU[8] = 0.0f;

			
			rawHIDobject->numButtonAddr = numPtTags = XML.getNumTags("button");
			if(numPtTags > 0) {
				totalToRead = MIN(numPtTags, 3);
				for(i = 0; i < totalToRead; i++) {
					ID = XML.getAttribute("button", "id", 0, i);

					str1 = XML.getValue("button:oscaddress", "/sabre/button/0", i);
					if(str1.length() > 0) {
						rawHIDobject->buttonaddresses[ID] = str1;
					}
				}
			}
			
			rawHIDobject->numAirAddr = numPtTags = XML.getNumTags("air");
			if(numPtTags > 0) {
				totalToRead = MIN(numPtTags, 2);
				for(i = 0; i < totalToRead; i++) {
					ID = XML.getAttribute("air", "id", 0, i);

					str1 = XML.getValue("air:oscaddress", "/sabre/air/0", i);
					if(str1.length() > 0) {
						rawHIDobject->airaddresses[ID] = str1;
					}
				}
                rawHIDobject->airValue.minimum = XML.getValue("air:minimum", 0, -20.0);
                rawHIDobject->airValue.maximum = XML.getValue("air:maximum", 120, 80.0);
                if(rawHIDobject->airValue.maximum != rawHIDobject->airValue.minimum) {
                    if(rawHIDobject->airValue.maximum > abs(rawHIDobject->airValue.minimum)) {
                        rawHIDobject->airValue.scale = ( 1.0 / rawHIDobject->airValue.maximum) * 0.5;
                    } else if(rawHIDobject->airValue.maximum < abs(rawHIDobject->airValue.minimum) ){
                        rawHIDobject->airValue.scale = ( 1.0 / abs(rawHIDobject->airValue.minimum)) * 0.5;
                    }else{
                        rawHIDobject->airValue.scale = 0.0;
                    }

                    
                }else{
                    rawHIDobject->airValue.scale = 0.0;
                }
			}						
            
			XML.popTag(); // pop root /sabre tag
		}
        
        str1 = XML.getValue("system:oscaddress", "/sabre/systime", i);
        if(str1.length() > 0) {
            rawHIDobject->timestampAddressServer = str1+"/server";
            rawHIDobject->timestampAddressLeft = str1+"/left";
            rawHIDobject->timestampAddressRight = str1+"/right";
            rawHIDobject->timestampAddressAir = str1+"/air";  
        }
        
        str1 = XML.getValue("keycode:oscaddress", "/sabre/keycode", i);
        if(str1.length() > 0) {
            rawHIDobject->keycodeaddress = str1;
        }
        
        str1 = XML.getValue("midinote:oscaddress", "/sabre/note", i);
        if(str1.length() > 0) {
            rawHIDobject->midinoteaddress = str1;
        }	
        

        ofLog(OF_LOG_NOTICE, "success reading Settings File");

	}
	return result;
}

void sabreServer::dumpPrefs()
{
	int i;
	
	printf("serialport %s\n", rawHIDobject->serialport.c_str());
    
	printf("sender 1 IP %s\n", rawHIDobject->sendIP[0].c_str());
	printf("sender 1 port %d\n", rawHIDobject->sendport[0]);
	printf("sender 1 active %d\n", rawHIDobject->senderActive[0]);
	printf("sender 1 mode %d\n", rawHIDobject->senderMode[0]);

    printf("sender 2 IP %s\n", rawHIDobject->sendIP[1].c_str());
	printf("sender 2 port %d\n", rawHIDobject->sendport[1]);
	printf("sender 2 active %d\n", rawHIDobject->senderActive[1]);
	printf("sender 2 mode %d\n", rawHIDobject->senderMode[1]);
    
    printf("sender 3 IP %s\n", rawHIDobject->sendIP[2].c_str());
	printf("sender 3 port %d\n", rawHIDobject->sendport[2]);
	printf("sender 3 active %d\n", rawHIDobject->senderActive[2]);
	printf("sender 3 mode %d\n", rawHIDobject->senderMode[2]);
    
    printf("sender 4 IP %s\n", rawHIDobject->sendIP[3].c_str());
	printf("sender 4 port %d\n", rawHIDobject->sendport[3]);
	printf("sender 4 active %d\n", rawHIDobject->senderActive[3]);
	printf("sender 4 mode %d\n", rawHIDobject->senderMode[3]);
    
	printf("receive port %d\n", receiveport);
	printf("baudrate %d\n", rawHIDobject->baudrate);
	printf("framerate %d\n", framerate);
	printf("display %d\n", display);
	printf("threshDown %f\n", rawHIDobject->threshDown);
	printf("threshUp %f\n", rawHIDobject->threshUp);
	printf("accelResolution %d\n", rawHIDobject->accelResolution);
	printf("accelOffset %ld\n", rawHIDobject->accelOffset);
	printf("accelScale %2.12f\n", rawHIDobject->accelScale);
//	printf("OSCfullspeed %d\n", rawHIDobject->fullspeedOSC);
	printf("OSCinterval %d\n", rawHIDobject->OSCsendingInterval);
    
	for(i = 0; i < rawHIDobject->numKeyAddr; i++) {
		printf("key %d\n", i);
		printf("    oscaddress %s\n", rawHIDobject->keys[i].oscaddress.c_str());
		printf("    inverted %d\n", rawHIDobject->keys[i].inverted);
		printf("    minimum %ld\n", rawHIDobject->keys[i].minimum);
		printf("    maximum %ld\n", rawHIDobject->keys[i].maximum);
		printf("    threshUp %f\n", rawHIDobject->keys[i].threshUp);
		printf("    threshDown %f\n", rawHIDobject->keys[i].threshDown);
	}
	printf("keycode\n    oscaddress %s\n", rawHIDobject->keycodeaddress.c_str());

    for(i = 0; i < rawHIDobject->numAirAddr; i++) {
		printf("airValues %d\n", i);
		printf("    minimum %f\n", rawHIDobject->airValue.minimum);
		printf("    maximum %f\n", rawHIDobject->airValue.maximum);
    }
    
	printf("imu\n");		   
	for(i = 0; i < rawHIDobject->numImuAddr; i++){
		printf("    oscaddress %d %s\n",i, rawHIDobject->imuaddresses[i].c_str() );
	}

	printf("button\n");		   
	for(i = 0; i < rawHIDobject->numButtonAddr; i++){
		printf("    oscaddress %d %s\n", i, rawHIDobject->buttonaddresses[i].c_str());
	}
	printf("air\n");		   

	for(i = 0; i < rawHIDobject->numAirAddr; i++){
		printf("    oscaddress %d %s\n", i, rawHIDobject->airaddresses[i].c_str());

	}
	printf("timestamp\n    oscaddress server %s\n", rawHIDobject->timestampAddressServer.c_str());
    printf("    oscaddress left %s\n", rawHIDobject->timestampAddressLeft.c_str());
	printf("    oscaddress right %s\n", rawHIDobject->timestampAddressRight.c_str());
	printf("    oscaddress air %s\n", rawHIDobject->timestampAddressAir.c_str());

    printf("link quality\n    oscaddress left %s\n", rawHIDobject->linkQualityAddressLeft.c_str());
    printf("    oscaddress right %s\n", rawHIDobject->linkQualityAddressRight.c_str());
	printf("    oscaddress air %s\n", rawHIDobject->linkQualityAddressAir.c_str());

    printf("battery\n    oscaddress main %s\n", rawHIDobject->batteryAddressMain.c_str());
    printf("    oscaddress air %s\n", rawHIDobject->batteryAddressAir.c_str());

    
}


void sabreServer::writePrefs()
{
    // only store GUI changeable control parameters:
	XML.setValue("sabre:display",					display);
	XML.setValue("sabre:serialport",				rawHIDobject->serialport);
	XML.saveFile("sabreServer.xml");
	return;
}

void sabreServer::readMidicodes()
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

//			printf("midiCodes loaded with %d note\n", numPtTags);
			for(int i = 0; i < 128; i++) {
				rawHIDobject->midiNote[i].note = 0;
				rawHIDobject->midiNote[i].keycode = -1;
			}
			for(int i = 0; i < numPtTags; i++) {
				rawHIDobject->midiNote[i].note = XMLmidi.getValue("note:midi", 0, i);
				rawHIDobject->midiNote[i].keycode = XMLmidi.getValue("note:code", 0, i);
//				printf("midinote ID %d note %d code %ld\n", i, rawHIDobject->midiNote[i].note, rawHIDobject->midiNote[i].keycode); 
			}
			XML.popTag();
		}
        ofLog(OF_LOG_NOTICE, "success reading MIDI-codes");
	}
}

void sabreServer::writeScaling()
{	
	XML.pushTag("sabre", 0);

	for(int i = 0; i < rawHIDobject->numKeyAddr; i++) {
		XML.pushTag("key", i);
		
		XML.removeTag("minimum", 0);
		XML.removeTag("maximum", 0);
		XML.setValue("minimum", (int)rawHIDobject->keys[i].minimum, i);
		XML.setValue("maximum", (int)rawHIDobject->keys[i].maximum, i);
		// printf("key %d min %d max %d\n", i, (int)rawHIDobject->keys[i].minimum, (int)rawHIDobject->keys[i].maximum);
		XML.popTag();
	}
    XML.removeTag("air:minimum", 0);
    XML.removeTag("air:maximum", 0);
    XML.setValue("air:minimum", rawHIDobject->airValue.minimum, 0);
    XML.setValue("air:maximum", rawHIDobject->airValue.maximum, 0);
    
	XML.popTag();
	XML.saveFile("sabreServer.xml");
	return;
}

void sabreServer::resetCalibrate()
{
	for(int i = 0; i < rawHIDobject->numKeyAddr; i++) 
	{
		rawHIDobject->keys[i].minimum = 1023;
		rawHIDobject->keys[i].maximum = 0;
	}
}

void sabreServer::resetAirCalibrate()
{
    rawHIDobject->airValue.minimum = 32768;
    rawHIDobject->airValue.maximum = -32768;
}

void sabreServer::resetSingleCalibrate(int i)
{
    rawHIDobject->keys[i].minimum = 1023;
    rawHIDobject->keys[i].maximum = 0;

}

void sabreServer::keyReleased(int key)
{
	switch(key){
		case 'f': // f-key: switch winow size
			drawValues = !drawValues;
            rawHIDobject->drawValues = drawValues;
			windowChanged = 1;
			redrawFlag = 1;
//			if(!drawValues) {
//				rawHIDobject->calibrate = 0;
//				writeScaling();
//			} else {
//				resetCalibrate();
//			}
			break;
		case 'c': // c-key:
			/*
			rawHIDobject->calibrate != rawHIDobject->calibrate;
			if(rawHIDobject->calibrate == 1) { // before we switch it off
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
		case 'd':
			dumpPrefs();
			break;
		case 'p':
			// sabreServer::readPrefs();
			break;
		case 'F':
			drawValues = !drawValues;
            rawHIDobject->drawValues = drawValues;
			windowChanged = 1;
			redrawFlag = 1;
//			if(!drawValues) {
//				rawHIDobject->calibrate = 0;			
//				writeScaling();
//			} else {
//				resetCalibrate();
//			}
			break;			
		case 'w':
			// sabreServer::writePrefs();
			break;
        case OF_KEY_LEFT:
            break;
        case OF_KEY_UP:
            whichStatus++;
            if(whichStatus >3){
                whichStatus = 0;
            }
            redrawFlag = 1;
            break;
        case OF_KEY_RIGHT:
            break;
        case OF_KEY_DOWN:
            whichStatus--;
            if(whichStatus < 0){
                whichStatus = 3;
            }
            redrawFlag = 1;
            break;
	}
}

void sabreServer::mouseMoved( int x, int y)
{
}

void sabreServer::mouseDragged( int x, int y, int button)
{
}

void sabreServer::mouseReleased()
{
}

void sabreServer::mousePressed(int x, int y, int button)
{
	int i;
//	printf("mousepressed at %d %d\n", x, y);
	ofRect(295, 36, 295+124, 36+20);
	
	// click in start/stope values
	if(x > 270 && x < 394 && y > 4 && y < 25) {
		// printf("start/stop clicked with status %d\n", rawHIDobject->status);
		if(rawHIDobject->status) {
			stopHID();
//			drawValues = 0;
//			rawHIDobject->status = false;
		} else {
            stopHID();
            ofSleepMillis(5);
			startHID();
            
            startOSC();

			drawValues = 1;
            rawHIDobject->drawValues = 1;
//			rawHIDobject->status = true;
		}
		
		windowChanged = 1;
		redrawFlag = 1;
	}
	// click in show/hide values
	if(x > 397 && x < 520 && y > 3 && y < 24) {
		if(drawValues != 0) {
			drawValues = 0;
            rawHIDobject->drawValues = 0;
			rawHIDobject->calibrateSwitch = 0;
		} else {
			drawValues = 1;
            rawHIDobject->drawValues = 1;
		}
		windowChanged = 1;
		redrawFlag = 1;
	}
	
	/*
	// click in menu-original-textbox
	if(x > 10 && x < 200
       && y > 3 && y < 21) {
		if(menuState != 0) {
			menuState = 0;
		} else {
			menuState = 1;
			getHIDDeviceList();
			stopHID();
			drawValues = 1;
            rawHIDobject->drawValues = 1;
		}
		windowChanged = 1;
		redrawFlag = 1;
		// printf("menustate %d\n", menuState);
	}
	
	// click in menu?
	if(menuState) {
		if(x > 15 && x < 203 && y > 21 && y < 21+numMenuItems*18) {
			int clickedItem = (y - 21) / 18;
			
			// printf("clicked in menu at %d %d clickedItem %d\n", x, y, clickedItem);
			menuState = 0;

			vector<ofSerialDeviceInfo>::iterator it;
			int i = 0;
			for ( it = rawHIDobject->deviceList.begin(); it < rawHIDobject->deviceList.end(); it++) {
				if(!strncmp(it->getDeviceName().c_str(), "tty", 3)){
					if(i == clickedItem) {
						rawHIDobject->serialport = it->getDevicePath();
                        stopHID();
                        ofSleepMillis(5);
						startHID();
                        
                        startOSC();

//						XML.pushTag("sabre", 0);
//						XML.removeTag("serialport", 0);
//						XML.setValue("serialport", rawHIDobject->serialport, 0);
//						XML.popTag();
//						XML.saveFile("sabreServer.xml");
						
						break;
					}
					i++;
				}
			}
			redrawFlag = 1;
		}
	}
	
	*/
	// click in main calibrate button
	// ofRect(295, 690, 124, 20);
	if(x > 375 && x < 500 && y > 480 && y < 500) {
		if(rawHIDobject->calibrateSwitch != 0) { // switch off
			rawHIDobject->calibrateSwitch = 0;
            rawHIDobject->calibrateSingle = 0;
            for(i = 0; i < MAXNUM; i++) {
				rawHIDobject->calibrate[i] = 0;
			}
			writeScaling();
		
        } else { //switch on
			rawHIDobject->calibrateSwitch = 1;
            rawHIDobject->calibrateSingle = 1;
//            for(i = 0; i < MAXNUM; i++) {
//				rawHIDobject->calibrate[i] = 1;
//			}
//            
//			resetCalibrate();
		}
//		printf("rawHIDobject->calibrateSwitch is %d\n", rawHIDobject->calibrate);
	}
    if(rawHIDobject->calibrateSwitch) {
        // click in calibrateAll == calibrateSingle flag :: conditional on main calibrate button
        if(x > 375 && x < 500 && y > 458 && y < 478) {
            if(rawHIDobject->calibrateSingle != 0){ // switch off
                rawHIDobject->calibrateSingle = 0;
                // reset all to zero
                for(i = 0; i < MAXNUM; i++) {
                    rawHIDobject->calibrate[i] = 1;
                }
                resetCalibrate();
            } else { // switch on
                rawHIDobject->calibrateSingle = 1;
                for(i = 0; i < MAXNUM; i++) {
                    rawHIDobject->calibrate[i] = 0;
                }
                writeScaling();

            }
//            printf("rawHIDobject->calibrateSingle is %d\n", rawHIDobject->calibrateSingle);
        }
        
        if(x > 346 && x < 362){
            int yy = y - 57;
            i = yy / 18;
//            printf("clicked inside calibrate toggle Nr. %d at pos %d %d\n",i, x, y);
            if(rawHIDobject->calibrate[i] == 0){
                rawHIDobject->calibrate[i] = 1;
                resetSingleCalibrate(i);
            }else{
                rawHIDobject->calibrate[i] = 0;
                writeScaling();
            }
        }

        if(x > 375 && x < 500 && y > 436 && y < 456) {
//            printf("clicked in reset Calibration");
            for(int i = 0; i < rawHIDobject->numKeyAddr; i++)
            {
                rawHIDobject->keys[i].minimum = 0;
                rawHIDobject->keys[i].maximum = 1023;
            }
        }
        
    }
    
    // click in calibrate air
    if(x > 502 && x < 627 && y > 480 && y < 500) {
		if(rawHIDobject->airValue.calibratePressureRange == true){
            rawHIDobject->airValue.calibratePressureRange = false;
//            printf("finished calibrating air with values minimum %f maximum %f\n",rawHIDobject->airValue.minimum, rawHIDobject->airValue.maximum);
            
            writeScaling();
        } else {
            rawHIDobject->airValue.calibratePressureRange = true;
            resetAirCalibrate();
        }
    }
}

