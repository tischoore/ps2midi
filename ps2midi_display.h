#include <LiquidCrystal.h>
#include <Wire.h>  // Comes with Arduino IDE
#include <FastIO.h>
#include <I2CIO.h>
#include <LCD.h>
#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal_SR.h>
#include <LiquidCrystal_SR2W.h>
#include <LiquidCrystal_SR3W.h>

#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>


#ifndef PS2MIDI_DISPLAY
#define PS2MIDI_DISPLAY


class PS2MIDI_display
{
  public:
   LiquidCrystal_I2C* lcd;
  
   PS2MIDI_display();
   ~PS2MIDI_display();

   // Print text to display
   void info_print(const char* line1, const char* line2 = "");
   void info_print(String line1, String line2 = "");
   void clear_lcd();
  

   void display_menu();
   void clear_menu();
   void add_menu_itm(unsigned int index, char* menu_item_name);
   void set_value_int(unsigned int index, int val);
   void set_value_hex(unsigned int index, int val);
   void set_value_str(unsigned int index, char* val);
private:
	bool menu_changed = false;

	char* top_line;
	char* bottom_line;
};




#endif //PS2MIDI_DISPLAY
