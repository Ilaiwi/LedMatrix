// Buffer.h

#ifndef _BUFFER_h
#define _BUFFER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


#endif

union Buffer {
	struct
	{
		unsigned bits : 40;
	};
	struct
	{
		unsigned byte_0 : 8;
		unsigned byte_1 : 8;
		unsigned byte_2 : 8;
		unsigned byte_3 : 8;
		unsigned byte_4 : 8;
	};
	struct
	{

		unsigned blue : 10;
		unsigned red : 10;
		unsigned green : 10;
		unsigned level : 10;
	};
};

