// setup_serializer.h

#ifndef _SETUP_SERIALIZER_h
#define _SETUP_SERIALIZER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


#define SETUP_VERIFICATION_ADDRESS 0
#define SETUP_START_ADDRESS 50
#define EEPROM_SIZE 1024 


class SetupSerializer
{
public:
	int WriteString(String in);

	bool Open();
	void Close();
	bool ValueOfField(String field, int* val);
	bool ValueOfField(String field, unsigned int startIndex, int* val);
	bool ValueOfField(String field, String* val);
	bool ValueOfField(String field, unsigned int startIndex, String * val);
	String GetInMemSetup();
	bool HasValidSetup();


private:
	String* inMemSetup;
};




class BaseSetup
{
public:
	String owner;
	String setupVs;
	String controller;
	String* Song_command_sequence;
	unsigned int* song_change_buttons;

	bool Initialize(SetupSerializer* input);
	byte GetControllerType(); // See the definition in PS2XLib
};


#endif

