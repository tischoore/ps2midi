// 
// 
// 

#include <EEPROM.h>
#include "setup_serializer.h"




int SetupSerializer::WriteString(String inString)
{
	EEPROM.update('0', SETUP_VERIFICATION_ADDRESS); // Invalidating any setup in EEPROM before attempting new write


	// Assuming 1024 bytes of EEPROM
	if (inString.length() > EEPROM_SIZE - SETUP_START_ADDRESS) // Setup will always be placed after the verification byte
		return -1;
	if (inString.length() == 0)
		return -1;
		
	int bytesWritten = 0;
	int currentReadAddress = SETUP_START_ADDRESS; 
	for (int readAddress = 0; readAddress < inString.length(); readAddress++)
	{
		char currentReadByte = (char) inString[readAddress];
		if (currentReadByte == '\n' || currentReadByte == '\t')
			continue; // Skipping newlines and tab
		EEPROM.update(currentReadAddress, currentReadByte);
		currentReadAddress++;
		bytesWritten++;
	}
	
	if (bytesWritten > 0)
	{
		EEPROM.update(currentReadAddress, '\0');
		EEPROM.update('1', SETUP_VERIFICATION_ADDRESS);
	}
	return bytesWritten;
}

bool SetupSerializer::Open()
{
	if (!HasValidSetup())
		return false;
	if (inMemSetup != NULL)
		delete inMemSetup;
	inMemSetup = new String("");

	int readAddress = SETUP_START_ADDRESS;
	unsigned int writeAddress = 0;

	char b = (char)EEPROM.read(readAddress);
	while ( b != '\0' && readAddress < EEPROM_SIZE && writeAddress < EEPROM_SIZE)
	{
		(*inMemSetup) += b;
		readAddress++;
		b = (char)EEPROM.read(readAddress);
		writeAddress++;
	}
	

	/*Serial.println("*******************");
	Serial.println(*inMemSetup);
	Serial.print("Length: ");
	Serial.println(inMemSetup->length());
	Serial.println("*******************");
	*/
	return true;
}

void SetupSerializer::Close()
{
	if (inMemSetup != NULL)
		delete inMemSetup;
}



bool SetupSerializer::ValueOfField(String field, int * val)
{
	return ValueOfField(field, 0, val);
}

bool SetupSerializer::ValueOfField(String field, unsigned int startIndex, int * val)
{
	String tempVal;
	if (!ValueOfField(field, startIndex, &tempVal))
		return false;
	(*val) = (int) tempVal.toInt();
	return true;
}


bool SetupSerializer::ValueOfField(String field, String * val)
{
	return ValueOfField(field, 0, val);
}

bool SetupSerializer::ValueOfField(String field, unsigned int startIndex,  String * val)
{
	if (!HasValidSetup())
		return false;
	if (inMemSetup == NULL)
		return false;

	int index = inMemSetup->indexOf(field);
	if (index < 0)
		return false;

	int valueStartIndex = inMemSetup->indexOf(":", index) + 1;
	while (inMemSetup->charAt(valueStartIndex) == ' ')
		valueStartIndex++;
	int entryEndIndex = inMemSetup->indexOf(";", index);
	(*val) = inMemSetup->substring(valueStartIndex, entryEndIndex);
	return true;
}

String SetupSerializer::GetInMemSetup()
{
	return *inMemSetup;
}

bool SetupSerializer::HasValidSetup()
{
	if ((char)EEPROM.read(SETUP_VERIFICATION_ADDRESS) == '0')
		return false;
	return true;
}






bool BaseSetup::Initialize(SetupSerializer * input)
{
	if (input->GetInMemSetup() == NULL || !input->HasValidSetup())
		return false;

	bool required_value_found = true;
	required_value_found = required_value_found && input->ValueOfField("Owner", &owner);
	required_value_found = required_value_found && input->ValueOfField("SetupVs", &setupVs);
	required_value_found = required_value_found && input->ValueOfField("Controller", &controller);

	return required_value_found;
}

byte BaseSetup::GetControllerType()
{
	if (controller == "DualShock")
		return 1;
	if (controller == "Guitar")
		return 2;
	return 0; // Unknown
}
