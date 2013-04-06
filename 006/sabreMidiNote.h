/*
 *  sabreMidiNote.h
 *  sabreServerProto2
 *
 *  Created by jasch on 9/12/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
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

