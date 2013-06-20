/*
 *  sabreAir.h
 * 
 *  SABRe-server
 *  © 2013 ICST / ZHdK  
 *
 *  @author Jan Schacher
 *  @date 20130620
 *
 */

#pragma once

class sabreAir
{
	
public:
	sabreAir(void) {

		minimum = 32768.0;
		maximum = -32768.0;
		scale = 1.0;
        offset = 0.0;
        relative = 0.0;
        calibrationFlag = 0;
        calibrationCounter = 0;
        calibrationValue = 0.0;
        calibratePressureRange = 0;
	}

	virtual ~sabreAir(){};

	double	continuous;		// container for continuous key-values
	double	minimum;
	double	maximum;
	double	scale;			// 1.0 / (max - min)
	double  offset;
    double  relative;
    
    bool        calibrationFlag;
    int         calibrationCounter;
    double      calibrationValue;
    bool        calibratePressureRange;
};

