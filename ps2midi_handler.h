/* PS2MIDI main handler function.
 *  
 *  By André Glasius Tischer 2018
 */
#include "PS2X_lib.h"  //for v1.6
#include "global_definitions.h"

#ifndef PS2MIDI_MAIN_HANDLER
#define PS2MIDI_MAIN_HANDLER


typedef struct
{
	unsigned int ps2_key;
	bool pressed;
	int pitch;
} Key;




class PS2MIDI_handler
{
  public:
    PS2X* ps2x;
  
	~PS2MIDI_handler();
  
    boolean InitializeController();
    
	bool switch_to_menu_mode_handler();

	// Returns if LCD display should update any values
	bool HandleGuitarHeroController();
	bool HandleGuitarHeroMenu();


	bool HandleDualShockController();
	bool HandleDualShockMenuController();


	void noteOn(int pitch, int velocity);
	void noteOff(int pitch);
	void all_notes_up();


	int midichannel = 0x0;
	int transpose = 0x0;
	bool octaver = false;
	int effect_modulation = 0x0;
	bool vibrate = false;

	char* EffectModulation_hex_to_str();

private:
	float pitch_multiplier = 74.4727f;
	float effect_multiplier = 1.14545f;
	bool pitch_neutral_set = false;
	bool mod_neutral_set = false;
	bool eff_neutral_set = false;

	// The initialized keys
	Key* keys;
	int keys_length = -1;

	unsigned long engage_menu_timer = 0;
	bool menu_mode = false; // When true the instrument does not play but handles the menus
};





#endif //PS2MIDI_MAIN_HANDLER
