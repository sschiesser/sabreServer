/*
 *  sabreKeys.h
 * 
 *  SABRe-server
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
 *  @date 20121030
 *
 */

#pragma once

#include <string>

class sabreKeys
{
	
public:
	sabreKeys(void) { 
		continuous = 0.0;
		raw = 0;
		rawOld = 0;
		minimum = 10000;
		maximum = -10000;
		scale = 1.0;
        offset = 0.0;
		binary = false;
		binaryOld = false;
		binaryChanged = false;
        inverted = false;
		changed = false;
        threshDown = 0.5;
        threshUp = 0.5;
        lastTriggerTime = 0;
		oscaddress = "/nada";
	}

	virtual ~sabreKeys(){};

	double	continuous;		// container for continuous key-values
	long	raw;
	long	rawOld;
	long	minimum;
	long	maximum;
	double	scale;			// 1.0 / (max - min)
	double  offset;
    bool	binary;			// container for binary key-values
	bool	binaryOld;		// container for prev binary key-values
	bool    binaryChanged;
	bool	inverted;
	bool	changed;
	double	threshDown;
	double	threshUp;
	int		lastTriggerTime;
	string	oscaddress;
    
};

