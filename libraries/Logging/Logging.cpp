#include "Logging.h"

void Logging::Init(int level, long baud)
{
	_sd_card_present = false;
	_log_to_file = NULL;
	_level = constrain(level,LOG_LEVEL_NOOUTPUT,LOG_LEVEL_VERBOSE);
	_baud = baud;
	Serial.begin(_baud);
}

void Logging::Error(const __FlashStringHelper *format, ...)
{
	if (LOG_LEVEL_ERRORS <= _level) 
	{   
		open_file();
		va_list args;
		va_start(args, format);
		print(format, args);
		close_file();
	}
}

void Logging::Info(const __FlashStringHelper *format, ...)
{
	if (LOG_LEVEL_INFOS <= _level) 
	{   
		open_file();
		va_list args;
		va_start(args, format);
		print(format, args);
		close_file();
	}
}

void Logging::Debug(const __FlashStringHelper *format, ...)
{
	if (LOG_LEVEL_DEBUG <= _level) 
	{   
		_log_to_file = NULL;
		va_list args;
		va_start(args, format);
		print(format, args);
	}
}

void Logging::Verbose(const __FlashStringHelper *format, ...)
{
	if (LOG_LEVEL_VERBOSE <= _level) 
	{   
		_log_to_file = NULL;
		va_list args;
		va_start(args, format);
		print(format, args);
	}
}


void Logging::print(const __FlashStringHelper *format, va_list args) {

	print(&Serial, format, args);

	if (_sd_card_present && _log_to_file)
	{
		print_date_time(_log_to_file, now());
		print(_log_to_file, format, args);    
	}
}

void Logging::print(Stream* stream, const __FlashStringHelper *format, va_list args) {
	//
	// loop through format string
	const char PROGMEM *p = (const char PROGMEM *)format;

	while(true)
	{
		char c = pgm_read_byte(p++);
		if (c == '\0') break;

		if (c == '%') 
		{
			c = pgm_read_byte(p++);
			if (c == '\0') break;
			if (c == '%') 
				stream->print(c);
			else
				if( c == 's' ) 
				{
					register char *s = (char *)va_arg( args, int );
					stream->print(s);
				}
				else
					if( c == 'd' )
						stream->print(va_arg( args, int ),DEC);
					else
						if( c == 'x' )
							stream->print(va_arg( args, int ),HEX);
						/*else
						else
						if( c == 'b' ) 
						stream->print(va_arg( args, int ),BIN);
						else
						if( c == 'l' ) 
						stream->print(va_arg( args, long ),DEC);
						*/
						else
							if( c == 'c' ) 
								stream->print((char)va_arg( args, int ));
							else
								if( c == 't' ) 
								{
									if (va_arg( args, int ) == 1) 
										stream->print("True");
									else
										stream->print("False");				
								}
		}
		else
			stream->print(c);
	}
}


void Logging::print_date_time(Stream* stream, time_t tm)
{
	char buffer[20];
	sprintf(buffer, "%02d/%02d/%d %02d:%02d:%02d - ", day(tm), month(tm), year(tm), hour(tm), minute(tm), second(tm));
	stream->print(buffer);
}

void Logging::UseSDCard()
{
	_sd_card_present = true;
} 

void Logging::open_file()
{
	_log_to_file = NULL;
	if (_sd_card_present)
	{
		char buffer[20];
		get_log_file_name("LOG_", buffer);
		File file = SD.open(buffer, FILE_WRITE);
		if (file) 
			_log_to_file = &file;
		else
			Error(F("Cannot write to datafile"CR));
	}
}

void Logging::close_file()
{
	if (_log_to_file)
	{
		_log_to_file->close();
		_log_to_file = NULL;
	}
}

void Logging::get_log_file_name(char* prefix, char* buffer)
{
	time_t tm = now();
	sprintf(buffer, "%s%02d%02d%d", prefix, year(tm) - 2000, month(tm), day(tm));
}

void Logging::GetLogFileName(char* prefix, char* buffer)
{
	get_log_file_name(prefix, buffer);
}


Logging Log = Logging();










