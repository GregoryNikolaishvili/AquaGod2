#ifndef LOGGING_H
#define LOGGING_H
#include <inttypes.h>
#include <stdarg.h>
#include <Time.h>
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
//#include "pins_arduino.h"
extern "C" {
#include <avr/io.h>
}


#define LOG_LEVEL_NOOUTPUT 0 
#define LOG_LEVEL_ERRORS 1
#define LOG_LEVEL_INFOS 2
#define LOG_LEVEL_DEBUG 3
#define LOG_LEVEL_VERBOSE 4

// default loglevel if nothing is set from user
#define LOGLEVEL LOG_LEVEL_DEBUG 


#define CRNL "\r\n"
#define LOGGING_VERSION 1

/*!
* Logging is a helper class to output informations over
* RS232. If you know log4j or log4net, this logging class
* is more or less similar ;-) <br>
* Different loglevels can be used to extend or reduce output
* All methods are able to handle any number of output parameters.
* All methods print out a formated string (like printf).<br>
* To reduce output and program size, reduce loglevel.
* <br>
* Output format string can contain below wildcards. Every wildcard
* must be start with percent sign (\%)
*
* <b>Depending on loglevel, source code is excluded from compile !</b><br>
* <br>
* <b>Wildcards</b><br>
* <ul>
* <li><b>\%s</b>	replace with an string (char*)</li>
* <li><b>\%c</b>	replace with an character</li>
* <li><b>\%d</b>	replace with an integer value</li>
* <li><b>\%l</b>	replace with an long value</li>
* <li><b>\%x</b>	replace and convert integer value into hex</li>
* <li><b>\%X</b>	like %x but combine with <b>0x</b>123AB</li>
* <li><b>\%b</b>	replace and convert integer value into binary</li>
* <li><b>\%B</b>	like %x but combine with <b>0b</b>10100011</li>
* <li><b>\%t</b>	replace and convert boolean value into <b>"t"</b> or <b>"f"</b></li>
* <li><b>\%T</b>	like %t but convert into <b>"true"</b> or <b>"false"</b></li>
* </ul><br>
* <b>Loglevels</b><br>
* <table border="0">
* <tr><td>0</td><td>LOG_LEVEL_NOOUTPUT</td><td>no output </td></tr>
* <tr><td>1</td><td>LOG_LEVEL_ERRORS</td><td>only errors </td></tr>
* <tr><td>2</td><td>LOG_LEVEL_INFOS</td><td>errors and info </td></tr>
* <tr><td>3</td><td>LOG_LEVEL_DEBUG</td><td>errors, info and debug </td></tr>
* <tr><td>4</td><td>LOG_LEVEL_VERBOSE</td><td>all </td></tr>
* </table>
* <br>
* <h1>History</h1><br>
* <table border="0">
* <tr><td>01 FEB 2012</td><td>initial release</td></tr>
* <tr><td>06 MAR 2012</td><td>implement a preinstanciate object (like in Wire, ...)</td></tr>
* <tr><td></td><td>methode init get now loglevel and baud parameter</td></tr>
*/
class Logging {
private:
	int _level;
	Print* pprinter;
public:
	//Logging(){};

	void Init(int level, Print* pprinter);

	void Error(const __FlashStringHelper *ifsh, ...);
	void Info(const __FlashStringHelper *ifsh, ...);
	void Debug(const __FlashStringHelper *ifsh, ...);
	void Verbose(const __FlashStringHelper *ifsh, ...);

	//void Error(char* msg, ...);
	//void Info(char* msg, ...);
	//void Debug(char* msg, ...);
	//void Verbose(char* msg, ...);   


	inline int GetLevel() { return _level; }
	inline void SetLogLevel(int level) { _level = level; }
	inline boolean IsVerbose() { return _level == LOG_LEVEL_VERBOSE; }
private:
	void print(const __FlashStringHelper *format, va_list args);
	void print(Stream* stream, const __FlashStringHelper *format, va_list args);
};

extern Logging Log;
#endif
