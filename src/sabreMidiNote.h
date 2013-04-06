/*
 *  SabreMidiNotes.h
 * 
 *  SABRe-server
 *  © 2012 ICST / ZHdK  
 *
 *  @author Jan Schacher
 *  @date 20121030
 *
 */

#pragma once

class sabreMidiNote
{	
public:
	sabreMidiNote(void) 
	{ 
		keycode = 0;
		note = -1;
	}
	
	virtual ~sabreMidiNote(){};
	
	long	keycode;
	int		note;
};

