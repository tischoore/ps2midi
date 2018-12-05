#pragma once

#include "global_definitions.h"
#include "ps2midi_handler.h"
#include "ps2midi_display.h"
#include "setup_serializer.h"




// Uncomment if you are debugging and don't want the hassle of a actual MIDI device.
//#define PRINT_TO_SERIAL
PS2MIDI_handler handler;
PS2MIDI_display* lcd;
SetupSerializer* serializer;
BaseSetup baseSetup;


//right now, the library does NOT support hot pluggable controllers, meaning 
//you must always either restart your Arduino after you conect the controller, 
//or call config_gamepad(pins) again after connecting the controller.
int error = 0; 
byte type = 0;
byte vibrate = 0;


// Indicating 
byte midichannel_change = 0;
byte transpose_change = 0;

/*
 { PSB_START,    false, 0x1E},
  { PSB_SELECT,   false, 0x1F}
  */


// See keymap: http://www.wavosaur.com/download/midi-note-hex.php
Key keys[] = {
{ PSB_PAD_DOWN    , false,   0x30 },
{ PSB_PAD_LEFT    , false,   0x31 },
{ PSB_PAD_UP      , false,   0x32 },
{ PSB_PAD_RIGHT   , false,   0x33 },
{ PSB_L1          , false,   0x34 },
{ PSB_L2          , false,   0x35 },
{ PSB_CROSS       , false,   0x36 },
{ PSB_SQUARE      , false,   0x37 },
{ PSB_TRIANGLE    , false,   0x38 },
{ PSB_CIRCLE      , false,   0x39 },
{ PSB_R1          , false,   0x3A },
{ PSB_R2          , false,   0x3B }

};
int keys_length = 12;

bool octaver_pressed = false;



void all_node_up()
{
  for(int k = 0; k < keys_length; k++)
  {
    if(keys[k].pressed)
    {
      // 0x8X indicates note off //0x9 would be on
      noteOn(0x80 + handler.midichannel, keys[k].pitch, 0x45);
      keys[k].pressed = false;
    }
  }  
}



void change_pitch(int p)
{
  all_node_up();
  
  for(int k = 0; k < keys_length; k++)
  {
    keys[k].pitch += p;
  }
  handler.transpose += p;
}




void midi_reset()
{
  // Control change is 0xBX where X is the midi channel
  // 0x7B is all keys up (notes off)
  // https://users.cs.cf.ac.uk/Dave.Marshall/Multimedia/node158.html
  for(int mc = 0xB0; mc <= 0xBF; mc++)
  { 
    Serial.write(mc);
    Serial.write(0x7B);
  }
}


void exit_program(char* line1, char* line2)
{
  lcd->info_print(line1,line2);
  while(true){delay(1000);}
}




void setup()
{
	// Power indicator LED (red)
	pinMode(POWER_DIODE_PIN, OUTPUT);
	digitalWrite(POWER_DIODE_PIN, HIGH);


	if (lcd == NULL)
		lcd = new PS2MIDI_display();
	lcd->lcd->clear();
	lcd->lcd->setCursor(0, 0);
	lcd->lcd->backlight();

#ifdef _DEBUG
	// Warn if using a debug build because we don't support playing music at that speed
	lcd->info_print("  DEBUG BUILD   ", "WARN. SLOW EXEC.");
	for (int i = 0; i < 30; i++)
	{
		digitalWrite(POWER_DIODE_PIN, !digitalRead(9));
		delay(200);
	}
#endif // _DEBUG



	// Trying to initialize Controller
	while (!handler.InitializeController())
	{
		lcd->info_print("Controller not", "found.");
		delay(3000);
		lcd->info_print("Controller not", "found. Retrying.");
		delay(1000);
	}

	String controllerName;
	type = handler.ps2x->readType();
	switch (type)
	{
	case 0:
		// How to get the controller type
		//handler.ps2x->getControllerTypeHex(), HEX
		lcd->info_print("Unknown", "controller");
		controllerName = "Unknown Ctrl";
		break;
	case 1:
		lcd->info_print("DualShock Found", "");
		controllerName = "DualShock";
		break;
	case 2:
		lcd->info_print("GuitarHero Found", "");
		controllerName = "Guitar";
		break;
	}
	
	delay(2000);

	// See if we should go into uplaod new Setup mode. Happens if we are either having no setup or if we are 
	// pressing both start and select
	serializer = new SetupSerializer();
	handler.ps2x->read_gamepad();
	if (!serializer->HasValidSetup() || (handler.ps2x->Button(PSB_START) && handler.ps2x->Button(PSB_SELECT)))
	{
		// Expect a new Setup
		DownloadNewSetup();
	}

	// Read the stored setup from EEPROM
	serializer->Open();
	if (!baseSetup.Initialize(serializer))
	{
		exit_program("Basic setup", "invalid. Exiting");
	}

	if (baseSetup.GetControllerType() != type)
	{
		digitalWrite(POWER_DIODE_PIN, LOW);
		while (true)
		{
			lcd->info_print("Controller not", "matching setup");
			delay(2000);
			lcd->info_print("Expected ctrl:", baseSetup.controller);
			delay(2000);
			lcd->info_print("Found:", controllerName);
			delay(2000);
		}
	}

	serializer->Close();

	lcd->info_print(baseSetup.owner, "Ver: " + baseSetup.setupVs);
	delay(5000);


	// Midi baud 
	Serial.begin(31250);



	midi_reset();
	lcd->info_print("Init Done", "");

	lcd->add_menu_itm(0, "TSP");
	lcd->set_value_hex(0, handler.transpose);
	lcd->add_menu_itm(1, "CH ");
	lcd->set_value_hex(1, handler.midichannel);
	lcd->add_menu_itm(2, "OCT");
	lcd->set_value_str(2, (handler.octaver ? "ON" : "OFF"));
	lcd->add_menu_itm(3, "EFF");
	lcd->set_value_str(3, handler.EffectModulation_hex_to_str());
	delay(1000);
}


//  plays a MIDI note.  Doesn't check to see that
//  cmd is greater than 127, or that data values are  less than 127:
void noteOn(int cmd, int pitch, int velocity) {
  Serial.write(cmd);
  Serial.write(pitch);
  Serial.write(velocity);
}




void loop()
{
	lcd->display_menu();




   /* You must Read Gamepad to get new values
   Read GamePad and set vibration values
   handler.ps2x->read_gamepad(small motor on/off, larger motor strenght from 0-255)
   if you don't enable the rumble, use handler.ps2x->read_gamepad(); with no values
   
   you should call this at least once a second
   */
   
 if(error == 1) //skip loop if no controller found
  return; 
  
 if (type == 2)
 { //Guitar Hero Controller
	 if (handler.HandleGuitarHeroController())
	 {
		 lcd->set_value_hex(0, handler.transpose);
		 lcd->set_value_hex(1, handler.midichannel);
		 lcd->set_value_str(2, (handler.octaver ? "ON" : "OFF"));
		 lcd->set_value_str(3, handler.EffectModulation_hex_to_str());
	 }

 }
 else 
 { //DualShock Controller
  
	 if (handler.HandleDualShockController())
	 {
		 lcd->set_value_hex(0, handler.transpose);
		 lcd->set_value_hex(1, handler.midichannel);
		 lcd->set_value_str(2, (handler.octaver ? "ON" : "OFF"));
		 lcd->set_value_str(3, handler.EffectModulation_hex_to_str());
	 }
 }
 delay(10);   
}


// Mode where we expect a new setup to be uploaded from the Setup C# program
void DownloadNewSetup()
{
	Serial.end();
	Serial.begin(115200);
	Serial.flush();

	lcd->info_print("Waiting for serial");
	while (Serial.available() <= 0)
	{
		delay(5);
	}
	lcd->clear_lcd();
	if (Serial.available() > 0)
	{
		digitalWrite(POWER_DIODE_PIN, LOW);
		String eeprom_written_successfully = "False";
		String allTextRecieved = "";

		// Wait for start of setup buffer
		bool startOfTransmission = false;
		while (!startOfTransmission)
		{
			if (Serial.available() > 0)
			{
				String tmpInput = Serial.readString();
				int startTransmissionIndex = tmpInput.indexOf(":#[");
				if (startTransmissionIndex >= 0)
				{
					startOfTransmission = true;
					allTextRecieved = tmpInput.substring(startTransmissionIndex + 3);
				}
			}
		}


		bool endOfTransmissionRecieved = false;
		while (!endOfTransmissionRecieved)
		{
			if (Serial.available() > 0)
				allTextRecieved = allTextRecieved + Serial.readString();
			int endTransmissionIndex = allTextRecieved.indexOf("]#:");
			if (endTransmissionIndex >= 0)
			{
				endOfTransmissionRecieved = true;
				allTextRecieved = allTextRecieved.substring(0, endTransmissionIndex);
			}
		}

		int numberOfBytesWrittenToEEPROM = 0;
		if (endOfTransmissionRecieved && allTextRecieved.length() > 0)
		{
			numberOfBytesWrittenToEEPROM = serializer->WriteString(allTextRecieved);
			if (numberOfBytesWrittenToEEPROM == allTextRecieved.length())
			{
				eeprom_written_successfully = "True";
			}
		}
		digitalWrite(POWER_DIODE_PIN, HIGH);

		char topLine[16];
		char bottomLine[16];
		memset(topLine, ' ', 16);
		memset(bottomLine, ' ', 16);
		sprintf(topLine, "EEPROM: %s", eeprom_written_successfully.c_str());
		sprintf(bottomLine, "Re:%000d Wr:%000d", allTextRecieved.length(), numberOfBytesWrittenToEEPROM);
		lcd->info_print(topLine, bottomLine);
		delay(10000);
	}


	exit_program("Upload Done.", "Restart now");
}

