#include "Logging.h"

void Logging::Init(int level, Print* pprinter)
{
	_level = constrain(level, LOG_LEVEL_NOOUTPUT, LOG_LEVEL_VERBOSE);
	this->pprinter = pprinter;
}

void Logging::Error(const __FlashStringHelper *format, ...)
{
	if (LOG_LEVEL_ERRORS <= _level)
	{
		va_list args;
		va_start(args, format);
		print(format, args);
	}
}

void Logging::Info(const __FlashStringHelper *format, ...)
{
	if (LOG_LEVEL_INFOS <= _level)
	{
		va_list args;
		va_start(args, format);
		print(format, args);
	}
}

void Logging::Debug(const __FlashStringHelper *format, ...)
{
	if (LOG_LEVEL_DEBUG <= _level)
	{
		va_list args;
		va_start(args, format);
		print(format, args);
	}
}

void Logging::Verbose(const __FlashStringHelper *format, ...)
{
	if (LOG_LEVEL_VERBOSE <= _level)
	{
		va_list args;
		va_start(args, format);
		print(format, args);
	}
}


void Logging::print(const __FlashStringHelper *format, va_list args) {

	const char PROGMEM *p = (const char PROGMEM *)format;

	while (true)
	{
		char c = pgm_read_byte(p++);
		if (c == '\0') break;

		if (c == '%')
		{
			c = pgm_read_byte(p++);
			if (c == '\0') break;
			if (c == '%')
				pprinter->print(c);
			else
			if (c == 's')
			{
				register char *s = (char *)va_arg(args, int);
				pprinter->print(s);
			}
			else
			if (c == 'd')
				pprinter->print(va_arg(args, int), DEC);
			else
			if (c == 'x')
				pprinter->print(va_arg(args, int), HEX);
			else
			if (c == 'c')
				pprinter->print((char)va_arg(args, int));
			else
			if (c == 't')
			{
				if (va_arg(args, int) == 1)
					pprinter->print("True");
				else
					pprinter->print("False");
			}
		}
		else
			pprinter->print(c);
	}

	pprinter->print(CRNL);
}


Logging Log = Logging();










