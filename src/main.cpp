/*
 *  main.cpp
 * 
 *  SABRe-server
 *  © 2012 ICST / ZHdK  
 *
 *  @author Jan Schacher
 *  @date 20121030
 *
 */

#include "ofMain.h"
#include "sabreServer.h"
#include "ofAppGlutWindow.h"

//========================================================================
int main( ){
	
    ofAppGlutWindow window;
	ofSetupOpenGL(&window, 420, 60, OF_WINDOW);			// <-------- setup the GL context
	
	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp( new sabreServer());
	
}
