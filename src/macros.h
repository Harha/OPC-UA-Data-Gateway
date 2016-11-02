#ifndef MACROS_H
#define MACROS_H

// std includes
#include <iostream>
#include <string>
#include <open62541.h>

// Logging (Normal)
template<typename ... Args>
void LOG(const char * msg, UA_DateTime datetime = UA_DateTime_now(), Args... args)
{
	// Convert datetime to struct
	UA_DateTimeStruct datetime_struct = UA_DateTime_toStruct(datetime);

	// Print the line to log
	std::printf((std::string("LOG %02u/%02u/%04u %02u:%02u:%02u:%03u: ") + std::string(msg)).c_str(),
		datetime_struct.day, datetime_struct.month, datetime_struct.year,
		datetime_struct.hour, datetime_struct.min, datetime_struct.sec, datetime_struct.milliSec,
		args...
	);
}

// Logging (Warning)
template<typename ... Args>
void WRN(const char * msg, UA_DateTime datetime = UA_DateTime_now(), Args... args)
{
	// Convert datetime to struct
	UA_DateTimeStruct datetime_struct = UA_DateTime_toStruct(datetime);

	// Print the line to log
	std::printf((std::string("WRN %02u/%02u/%04u %02u:%02u:%02u:%03u: ") + std::string(msg)).c_str(),
		datetime_struct.day, datetime_struct.month, datetime_struct.year,
		datetime_struct.hour, datetime_struct.min, datetime_struct.sec, datetime_struct.milliSec,
		args...
	);
}

// Logging (Error)
template<typename ... Args>
void ERR(const char * msg, UA_DateTime datetime = UA_DateTime_now(), Args... args)
{
	// Convert datetime to struct
	UA_DateTimeStruct datetime_struct = UA_DateTime_toStruct(datetime);

	// Print the line to log
	std::printf((std::string("ERR %02u/%02u/%04u %02u:%02u:%02u:%03u: ") + std::string(msg)).c_str(),
		datetime_struct.day, datetime_struct.month, datetime_struct.year,
		datetime_struct.hour, datetime_struct.min, datetime_struct.sec, datetime_struct.milliSec,
		args...
	);
}

// Macros for heap object deletion
#define DELETES(a) if( (a) != NULL ) delete (a); (a) = NULL;
#define DELETEA(a) if ( (a) != NULL ) delete[] (a); (a) = NULL;

#endif // MACROS_H