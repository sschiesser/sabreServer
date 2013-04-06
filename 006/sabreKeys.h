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
		inverted = 0;
		binary = 0;
		binaryOld = 0;
		binaryChanged = 0;
		inverted = 0;
		changed = 0;
		oscaddress = "/nada";
		lastTriggerTime = 0;
	}

	virtual ~sabreKeys(){};

	double	continuous;		// container for continuous key-values
	long	raw;
	long	rawOld;
	long	minimum;
	long	maximum;
	double	scale;			// 1.0 / (max - min)
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

