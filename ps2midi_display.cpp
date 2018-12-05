#include "ps2midi_display.h"
#include<math.h>

PS2MIDI_display::PS2MIDI_display()
{
  
  // set the LCD address to 0x27 for a 16 chars 2 line display
  // A FEW use address 0x3F
  // Set the pins on the I2C chip used for LCD connections:
  //                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
  lcd = new LiquidCrystal_I2C(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
  lcd->begin(16,2);
  top_line = new char[18];
  bottom_line = new char[18];
  clear_menu();
}

PS2MIDI_display::~PS2MIDI_display()
{
  if(lcd != NULL)
    delete lcd;
}



void PS2MIDI_display::info_print(const char* line1, const char* line2 = "")
{
  lcd->clear();
  lcd->setCursor(0,0);
  lcd->write(line1);
  lcd->setCursor(0,1);
  lcd->write(line2);    
}

void PS2MIDI_display::info_print(String line1, String line2 = "")
{
	String topLineCropped = line1.substring(0, 15);
	String btmLineCropped = line2.substring(0, 15);

	char topLine[16];
	char bottomLine[16];
	memset(topLine, ' ', 16);
	memset(bottomLine, ' ', 16);
	sprintf(topLine, "%s", topLineCropped.c_str());
	sprintf(bottomLine, "%s", btmLineCropped.c_str());
	info_print(topLine, bottomLine);
}

void PS2MIDI_display::clear_lcd()
{
	lcd->clear();
}


void PS2MIDI_display::add_menu_itm(unsigned int index, char* menu_item_name)
{
	if (index > 3)
		return;	

	sprintf(&top_line[index*4], "%s ", menu_item_name);
	menu_changed = true;
}




void PS2MIDI_display::set_value_int(unsigned int index, int val)
{
	if (index > 3)
		return;

	sprintf(&bottom_line[index * 4], "%02d  ", val);
	menu_changed = true;
}

void PS2MIDI_display::set_value_hex(unsigned int index, int val)
{
	set_value_int(index, val);
}

void PS2MIDI_display::set_value_str(unsigned int index, char* val)
{
	if (index > 3)
		return;

	sprintf(&bottom_line[index * 4], "%-4s", val);
	menu_changed = true;
}


void PS2MIDI_display::display_menu()
{
	if (menu_changed)
	{
		lcd->clear();
		lcd->setCursor(0, 0);
		lcd->write(top_line);
		lcd->setCursor(0, 1);
		lcd->write(bottom_line);
		menu_changed = false;
	}
}

void PS2MIDI_display::clear_menu()
{
	for (int i = 0; i < 18; i++)
	{
		top_line[i] = ' ';
		bottom_line[i] = ' ';
		menu_changed = true;
	}
}