#include <EEPROM.h>

#include "ps2midi_handler.h"

PS2MIDI_handler::~PS2MIDI_handler()
{
	if (keys != NULL && keys_length > 0)
	{
		delete[] keys;
		keys_length = -1;
	}
}


boolean PS2MIDI_handler::InitializeController()
{
	/*
	// Load Midichannel from EEPROM
	midichannel = EEPROM.read(0);
	if (midichannel < 0x0 || 0xF < midichannel)
		midichannel = 0x0; // Reset the value if it is outside the legal range
						   // Load Midichannel from EEPROM
	transpose = EEPROM.read(1);
	if (0x30 > transpose || transpose < -0x30)
		transpose = 0x0;

	int octaver_from_store = EEPROM.read(2);
	if (octaver_from_store != 0x0 && octaver_from_store != 0x1)
		octaver_from_store = 0x0;
	octaver = octaver_from_store != 0x0;
		
	effect_modulation = EEPROM.read(3);
	if (effect_modulation > 3)
		effect_modulation = 0;
	*/
	midichannel = 0;
	transpose = 0;
	octaver = 0;
	effect_modulation = 0;


  if(ps2x != NULL)
    delete ps2x;
  ps2x = new PS2X();
  
  delay(300);
  // Initalize connection to PS2 controller
  int error = ps2x->config_gamepad(13,11,10,12, false, false);   //setup pins and settings:  GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error

#ifdef PRINT_TO_SERIAL  
    if(error == 0)
    {
      Serial.println("Found Controller, configured successful");
      Serial.println("Try out all the buttons, X will vibrate the controller, faster as you press harder;");
      Serial.println("holding L1 or R1 will print out the analog stick values.");
      Serial.println("Go to www.billporter.info for updates and to report bugs.");
    }
    else if(error == 1)
     Serial.println("No controller found, check wiring, see readme.txt to enable debug. visit www.billporter.info for troubleshooting tips");
     
    else if(error == 2)
     Serial.println("Controller found but not accepting commands. see readme.txt to enable debug. Visit www.billporter.info for troubleshooting tips");
     
    else if(error == 3)
     Serial.println("Controller refusing to enter Pressures mode, may not support it. ");
 #endif

	if(error > 0)
	{
		return false;
	}
     

	// setup key map according to type of controller connected
	switch (ps2x->readType())
	{
	case 0:
		// Unknown controller
		break;
	case 1:
		// DualShock Controller Found
		keys_length = 12;
		keys = new Key[keys_length];

		keys[0].ps2_key = PSB_PAD_DOWN;    
		keys[0].pressed = false;   
		keys[0].pitch = 0x30;

		keys[1].ps2_key = PSB_PAD_LEFT;    
		keys[1].pressed = false;   
		keys[1].pitch = 0x31;

		keys[2].ps2_key = PSB_PAD_UP;      
		keys[2].pressed = false;   
		keys[2].pitch = 0x32;

		keys[3].ps2_key = PSB_PAD_RIGHT;   
		keys[3].pressed = false;   
		keys[3].pitch = 0x33;

		keys[4].ps2_key = PSB_L1;          
		keys[4].pressed = false;   
		keys[4].pitch = 0x34;

		keys[5].ps2_key = PSB_L2;          
		keys[5].pressed = false;   
		keys[5].pitch = 0x35;

		keys[6].ps2_key = PSB_CROSS;       
		keys[6].pressed = false;   
		keys[6].pitch = 0x36;

		keys[7].ps2_key = PSB_SQUARE;      
		keys[7].pressed = false;   
		keys[7].pitch = 0x37;
		
		keys[8].ps2_key = PSB_TRIANGLE;    
		keys[8].pressed = false;   
		keys[8].pitch = 0x38;

		keys[9].ps2_key = PSB_CIRCLE;      
		keys[9].pressed = false;   
		keys[9].pitch = 0x39;

		keys[10].ps2_key = PSB_R1;          
		keys[10].pressed = false;   
		keys[10].pitch = 0x3A;

		keys[11].ps2_key = PSB_R2;          
		keys[11].pressed = false;   
		keys[11].pitch = 0x3B;
		break;
	case 2:
		// GuitarHero Controller Found
		keys_length = 5;
		keys = new Key[keys_length];
		
		keys[0].ps2_key = ORANGE_FRET;
		keys[0].pressed = false;
		keys[0].pitch = 0x35;
	
		keys[1].ps2_key = BLUE_FRET;
		keys[1].pressed = false;
		keys[1].pitch = 0x34;

		keys[2].ps2_key = YELLOW_FRET;
		keys[2].pressed = false;
		keys[2].pitch = 0x33;

		keys[3].ps2_key = RED_FRET;
		keys[3].pressed = false;
		keys[3].pitch = 0x32;

		keys[4].ps2_key = GREEN_FRET;
		keys[4].pressed = false;
		keys[4].pitch = 0x31;
	
		break;
	}

	return true;
  
}



bool PS2MIDI_handler::switch_to_menu_mode_handler()
{
	// Turn on menu mode?
	if (ps2x->Button(PSB_SELECT) && ps2x->Button(PSB_START))
	{
		all_notes_up();

		unsigned long current_delta = millis() - engage_menu_timer;

		// Finding the sweet spot to turn on the menus
		if (current_delta  > 2500)
			engage_menu_timer = millis();
		else if (current_delta > 2000)
		{
			menu_mode = true;
			return true;
		}
	}
	digitalWrite(POWER_DIODE_PIN, HIGH);
	return false;
}



bool PS2MIDI_handler::HandleGuitarHeroController()
{
	ps2x->read_gamepad();          //read controller with no vibration	


	if (menu_mode || switch_to_menu_mode_handler())
		return HandleGuitarHeroMenu();


	// Read only the closest button. Only one tone is ever played at a time

	// Star power?
	//if (ps2x->ButtonPressed(STAR_POWER))
		//Serial.println("Star Power Command");
		//noteOn(0x90, 0x1E, 0x45);
	
	
	if (ps2x->ButtonPressed(UP_STRUM) || ps2x->ButtonPressed(DOWN_STRUM))
	{
		bool node_up_rest = false;
		for (int i = 0; i < keys_length; i++)
		{
			if (node_up_rest)
			{
				if (keys[i].pressed)
				{
					noteOff(keys[i].pitch);
					keys[i].pressed = false;
				}
				continue;
			}

			if (ps2x->Button(keys[i].ps2_key))
			{
				if (!keys[i].pressed)
				{
					// This key is newly pressed (since last strum). Play node
					keys[i].pressed = true;
					if (octaver && ps2x->Button(UP_STRUM))
					{
						noteOn(keys[i].pitch - 0x0C, 0x45);
					}
					else
						noteOn(keys[i].pitch, 0x45);
				}
				node_up_rest = true; // Only play the higest node and ignore all other pressed buttons
			}
		}
	}
	else if (ps2x->Button(UP_STRUM) || ps2x->Button(DOWN_STRUM))
	{
		// Terminate keys if no longer pressed
		for (int i = 0; i < keys_length; i++)
		{
			if (!ps2x->Button(keys[i].ps2_key) && keys[i].pressed)
			{
				noteOff(keys[i].pitch);
				keys[i].pressed = false;
			}
		}
	}
	else
	{
		all_notes_up();
	}

	// Wammy bar control

	if (effect_modulation > 0)
	{
		Serial.write(0xb0 + midichannel);
		switch (effect_modulation)
		{
		case 1: //MOD
			Serial.write(0x01);
			Serial.write(0x7f - ps2x->Analog(WHAMMY_BAR));
			break;
		case 2: //EFF
			Serial.write(0x0b);
			Serial.write(ps2x->Analog(WHAMMY_BAR));
			break;
		}
	}

	return false;
}



// This function assumes that no nodes are playing!
bool PS2MIDI_handler::HandleGuitarHeroMenu()
{
	if ((millis() / 500) % 2 == 0)
		digitalWrite(POWER_DIODE_PIN, HIGH);
	else
		digitalWrite(POWER_DIODE_PIN, LOW);


	if (ps2x->ButtonPressed(PSB_SELECT) && ps2x->ButtonPressed(PSB_START))
	{
		// Store "new" state
		/*EEPROM.write(0, midichannel);
		EEPROM.write(1, transpose);
		EEPROM.write(2, (octaver ? 0x1 : 0x0));
		EEPROM.write(3, effect_modulation);
*/
		// Disengage menu_mode
		menu_mode = false;
		engage_menu_timer = 0;
		digitalWrite(POWER_DIODE_PIN, HIGH);
		return true;
	}

	bool update_lcd = false;

	if (ps2x->Button(GREEN_FRET))
	{
		if (ps2x->ButtonPressed(DOWN_STRUM))
		{
			transpose -= 0x1;
			update_lcd = true;
		}
		else if (ps2x->ButtonPressed(UP_STRUM))
		{
			transpose += 0x1;
			update_lcd = true;
		}
	}


	if (ps2x->Button(RED_FRET))
	{
		if (ps2x->ButtonPressed(DOWN_STRUM))
		{
			midichannel -= 0x1;
			update_lcd = true;
		}
		else if (ps2x->ButtonPressed(UP_STRUM))
		{
			midichannel += 0x1;
			update_lcd = true;
		}

		// Cap value to 1-16 (midi channels)
		if (midichannel < 0x0)
			midichannel = 0x0;
		if (midichannel > 0xf)
			midichannel = 0xf;
	}


	if (ps2x->Button(YELLOW_FRET))
	{
		if (ps2x->ButtonPressed(DOWN_STRUM))
		{
			octaver = false;
			update_lcd = true;
		}
		else if (ps2x->ButtonPressed(UP_STRUM))
		{
			octaver = true;
			update_lcd = true;
		}
	}

	if (ps2x->Button(BLUE_FRET))
	{
		if (ps2x->ButtonPressed(DOWN_STRUM))
		{
			effect_modulation -= 0x1;
			update_lcd = true;
		}
		else if (ps2x->ButtonPressed(UP_STRUM))
		{
			effect_modulation += 0x1;
			update_lcd = true;
		}

		// Wrap around
		if (effect_modulation < 0x0)
			effect_modulation = 0x2;
		if (effect_modulation > 0x2)
			effect_modulation = 0x0;
	}


	return update_lcd;
}





bool PS2MIDI_handler::HandleDualShockController()
{
	ps2x->read_gamepad(false, vibrate);          //read controller and set large motor to spin at 'vibrate' speed

	if (menu_mode || switch_to_menu_mode_handler())
		return HandleDualShockMenuController();

	for (int i = 0; i < keys_length; i++)
	{
		if (!keys[i].pressed && ps2x->ButtonPressed(keys[i].ps2_key))
		{
			keys[i].pressed = true;
			noteOn(keys[i].pitch, 0x45);
			if(octaver)
				noteOn(keys[i].pitch - 0x0C, 0x45);
		}
		else if (keys[i].pressed && !ps2x->Button(keys[i].ps2_key))
		{
			keys[i].pressed = false;
			noteOff(keys[i].pitch);
		}
	}

	if (effect_modulation != 0x0)
	{
		
		// MOD
		int left_stick_x = ps2x->Analog(PSS_LX);
		if (left_stick_x < 110)
		{
			Serial.write(0xB0 + midichannel);
			Serial.write(0x01);
			Serial.write(126 - (int)(effect_multiplier * left_stick_x));
			mod_neutral_set = false; // dirty the listener
		}
		else if (left_stick_x > 144)
		{
			Serial.write(0xB0 + midichannel);
			Serial.write(0x01);
			Serial.write( (int)(effect_multiplier * (left_stick_x - 145)));
			mod_neutral_set = false; // dirty the listener
		}
		else if (!mod_neutral_set)
		{
			// Set to neutral
			Serial.write(0xB0 + midichannel);
			Serial.write(0x01);
			Serial.write(0x00);
			mod_neutral_set = true; // Only set the pitch to neutral once. Don't flood the serial with more noise
		}




		// EFF
		int left_stick_y = ps2x->Analog(PSS_LY);
		if (left_stick_y < 110)
		{
			Serial.write(0xB0 + midichannel);
			Serial.write(0x0B);
			Serial.write((int)(effect_multiplier * left_stick_y));
			eff_neutral_set = false; // dirty the listener

			
		}
		else if (left_stick_y > 144)
		{
			Serial.write(0xB0 + midichannel);
			Serial.write(0x0B);
			Serial.write(126 - (int)(effect_multiplier * (left_stick_y - 145)));
			eff_neutral_set = false; // dirty the listener
		}
		else if (!eff_neutral_set)
		{
			// Set to neutral
			Serial.write(0xB0 + midichannel);
			Serial.write(0x0B);
			Serial.write(0x7F);
			eff_neutral_set = true; // Only set the pitch to neutral once. Don't flood the serial with more noise
		}


		// BEND
		// Pitch bend uses LSB/MSB of a 16 bit unsighed int. from 0x0000 to 0x3FFF (16,383 decimal). 0x2000 bieing neutral.
		// We have a deadzone around each joystick of 16 degrees. 
		int right_stick = ps2x->Analog(PSS_RX);
		if (right_stick < 110 )
		{
			Serial.write(0xE0 + midichannel);
			uint16_t adjustment = (uint16_t)(pitch_multiplier * right_stick);
			Serial.write(adjustment & 0xFF);
			Serial.write(adjustment >> 8);
			pitch_neutral_set = false; // dirty the listener
		}
		else if(right_stick > 144)
		{
			Serial.write(0xE0 + midichannel);
			uint16_t adjustment = 8192 + ((uint16_t)(pitch_multiplier * (right_stick - 145)));
			Serial.write(adjustment & 0xFF);
			Serial.write(adjustment >> 8);
			pitch_neutral_set = false; // dirty the listener
		}
		else if(!pitch_neutral_set)
		{
			// Set to neutral
			Serial.write(0xE0 + midichannel);
			Serial.write(0x00);
			Serial.write(0x20);
			pitch_neutral_set = true; // Only set the pitch to neutral once. Don't flood the serial with more noise
		}

	}

	return false;
}



bool PS2MIDI_handler::HandleDualShockMenuController()
{
	if ((millis() / 500) % 2 == 0)
		digitalWrite(POWER_DIODE_PIN, HIGH);
	else
		digitalWrite(POWER_DIODE_PIN, LOW);


	if (ps2x->ButtonPressed(PSB_SELECT) && ps2x->ButtonPressed(PSB_START))
	{
		// Store "new" state
		/*
		EEPROM.write(0, midichannel);
		EEPROM.write(1, transpose);
		EEPROM.write(2, (octaver ? 0x1 : 0x0));
		EEPROM.write(3, effect_modulation);
*/
		// Disengage menu_mode
		menu_mode = false;
		engage_menu_timer = 0;
		digitalWrite(POWER_DIODE_PIN, HIGH);
		return true;
	}

	bool display_update = false;

	// Mod Eff
	if (ps2x->ButtonPressed(PSB_TRIANGLE))
	{
		effect_modulation = 0x3;
		display_update = true;
	}
	else if (ps2x->ButtonPressed(PSB_CROSS))
	{
		effect_modulation = 0x0;
		display_update = true;
	}

	// Octaver
	if (ps2x->ButtonPressed(PSB_PAD_UP))
	{
		octaver = true;
		display_update = true;
	}
	else if (ps2x->ButtonPressed(PSB_PAD_DOWN))
	{
		octaver = false;
		display_update = true;
	}

	// Transpose
	if (ps2x->ButtonPressed(PSB_L1))
	{
		transpose += 0x1;
		display_update = true;
	}
	else if (ps2x->ButtonPressed(PSB_L2))
	{
		transpose -= 0x1;
		display_update = true;
	}

	// Midi channel
	if (ps2x->ButtonPressed(PSB_R1))
	{
		midichannel += 0x1;
		display_update = true;
	}
	else if (ps2x->ButtonPressed(PSB_R2))
	{
		midichannel -= 0x1;
		display_update = true;
	}
	// Cap value to 1-16 (midi channels)
	if (midichannel < 0x0)
		midichannel = 0x0;
	if (midichannel > 0xf)
		midichannel = 0xf; 


	delay(100);
	return display_update;
}

















void PS2MIDI_handler::noteOn(int pitch, int velocity) 
{
	Serial.write(0x90 + midichannel);
	Serial.write(pitch + transpose);
	Serial.write(velocity);
}

void PS2MIDI_handler::noteOff(int pitch)
{
	Serial.write(0x80 + midichannel);
	Serial.write(pitch + transpose);
	Serial.write(0x45);

	if (octaver)
	{
		Serial.write(0x80 + midichannel);
		Serial.write((pitch + transpose) - 0x0C);
		Serial.write(0x45);
	}
}


void PS2MIDI_handler::all_notes_up()
{
	for (int i = 0; i < keys_length; i++)
	{
		if (keys[i].pressed)
		{
			noteOff(keys[i].pitch);
			keys[i].pressed = false;
		}
	}
}


char* PS2MIDI_handler::EffectModulation_hex_to_str()
{
	switch (effect_modulation)
	{
	case 0x0:
		return "OFF";
	case 0x1:
		return "MOD";
	case 0x2: 
		return "EXP";
	case 0x3:
		return "ON";
	}
}