#include <Arduino.h>
#include <MIDI.h>
#include <U8g2lib.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <EEPROM.h>

// ###############
// Pin assignments
// ###############

#define POTI_1 26 
#define POTI_2 14
#define POTI_3 27 
#define POTI_4 13

#define OLED_SCL 22
#define OLED_SDA 21

#define BUTTON_1 12 
#define BUTTON_2 25
#define BUTTON_3 33
#define BUTTON_4 32
#define BUTTON_5 19
#define BUTTON_6 18
#define BUTTON_7 15
#define BUTTON_8 23
#define BUTTON_9 4 
#define BUTTON_10 16

#define BUTTON_1_NAME "1"
#define BUTTON_2_NAME "2"
#define BUTTON_3_NAME "3"
#define BUTTON_4_NAME "4"
#define BUTTON_5_NAME "5"
#define BUTTON_6_NAME "6"
#define BUTTON_7_NAME "7"
#define BUTTON_8_NAME "8"
#define BUTTON_1_P2_NAME "9"
#define BUTTON_2_P2_NAME "10"
#define BUTTON_3_P2_NAME "11"
#define BUTTON_4_P2_NAME "12"
#define BUTTON_5_P2_NAME "13"
#define BUTTON_6_P2_NAME "14"
#define BUTTON_7_P2_NAME "15"
#define BUTTON_8_P2_NAME "16"

// Max 18 characters
#define ABOUT_LINE_1 "madanowi.cz/deej"
#define ABOUT_LINE_2 "Kochajmy Labno"
#define ABOUT_LINE_3 ""

#define LOGO "deej"

// ###############
// POTENTIOMETER MIDI RESCALE VALUES (0-127) example: Fader down sends 64, Fader up sends 127
// ###############

#define POTI_MIDI_SCALE_MIN 64
#define POTI_MIDI_SCALE_MAX 127

#define EEPROM_SIZE 19     // EEPROM size in bytes, 1 byte for each setting

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, OLED_SCL, OLED_SDA); // I2C screen declaration

// ###############
// MIDI SETTINGS
// ###############

const uint8_t midi_channel = 1;
const uint8_t controller_number = 0;
MIDI_CREATE_DEFAULT_INSTANCE();

// ###############
// VARIABLES AND FLAGS FOR INPUTS
// ###############

short poti_pin_number[4] = {POTI_1, POTI_2, POTI_3, POTI_4};
short button_pin_number[10] = {BUTTON_1, BUTTON_2, BUTTON_3, BUTTON_4, BUTTON_5, BUTTON_6, BUTTON_7, BUTTON_8, BUTTON_9, BUTTON_10};
const char *button_name[8] = {BUTTON_1_NAME, BUTTON_2_NAME, BUTTON_3_NAME, BUTTON_4_NAME, BUTTON_5_NAME, BUTTON_6_NAME, BUTTON_7_NAME, BUTTON_8_NAME};
const char *button_p2_name[8] = {BUTTON_1_P2_NAME, BUTTON_2_P2_NAME, BUTTON_3_P2_NAME, BUTTON_4_P2_NAME, BUTTON_5_P2_NAME, BUTTON_6_P2_NAME, BUTTON_7_P2_NAME, BUTTON_8_P2_NAME};


static int hysteresis[4] = {0,0,0,0};
static int poti_return[4] = {0,0,0,0};
int sensorValue[4] = {0,0,0,0};

bool button_state[10] = {0,0,0,0,0,0,0,0,0,0};

bool button_last_state[8] = {0,0,0,0,0,0,0,0};
bool button_p2_last_state[8] = {0,0,0,0,0,0,0,0};

bool button_type[8] = {0,0,0,0,0,0,0,0}; // 0 - momentary, 1 - toggle
bool button_p2_type[8] = {0,0,0,0,0,0,0,0}; // 0 - momentary, 1 - toggle

short buttons_page = 1;
short settings_page = 1;

// ###############
// VARIABLES AND FLAGS FOR SCREENS
// ###############

unsigned long confirmTime = 0;
bool confirmScreenLock = 0;

bool levels_screen_switch = 1;
bool menu_screen_switch = 0;
bool about_screen_switch = 0;
bool settings_screen_switch = 0;
bool lock_screen_switch = 0;
int menu_position = 0;

bool buttons_lock_for_change_type = 0;

const char *menu_items[4] = {"levels", "lock", "about", "settings"};
const char *about_items[4] = {ABOUT_LINE_1, ABOUT_LINE_2, ABOUT_LINE_3, "exit"};

// ###############
// SETTINGS
// ###############

short setting_comm_mode = 1; // 1 - MIDI, 2 - 
short setting_button_shift_mode = 1; // 1 - switch, 2 - shift
short setting_feedback_mode = 2; // 1 - on, 2 - off

// ###############
// FUNCTION DECLARATIONS
// ###############

void drawLevelsScreen();
void drawMenuScreen();
void drawAboutScreen();
void drawSettingsScreen();
void drawConfirm(const char* feature, const char* status);
void drawMsgInfinite(const char* msg);
void drawMsg(const char* msg);
void drawLockScreen();

void handleControlChange(byte channel, byte number, byte value);


// ###############
// SETUP FUNCTION, RUNS ONCE AT STARTUP
// ###############

void setup() {
  MIDI.begin();
  Serial.begin(115200);
  for (int i = 0; i < 4; i++) {
    pinMode(poti_pin_number[i], INPUT);
  }

  MIDI.turnThruOff();
  MIDI.setHandleControlChange(handleControlChange);

  // EEPROM READ SETTINGS ON STARTUP

  EEPROM.begin(EEPROM_SIZE);
  if (EEPROM.read(0) == 1 || EEPROM.read(0) == 2) {setting_comm_mode = EEPROM.read(0);}
  if (EEPROM.read(1) == 1 || EEPROM.read(1) == 2) {setting_button_shift_mode = EEPROM.read(1);}
  if (EEPROM.read(2) == 1 || EEPROM.read(2) == 2) {setting_feedback_mode = EEPROM.read(2);}

  for (int i = 0; i < 8; i++) {
    if (EEPROM.read(i+3) == 0 || EEPROM.read(i+3) == 1) {button_type[i] = EEPROM.read(i+3);}
  }

  for (int i = 0; i < 8; i++) {
    if (EEPROM.read(i+11) == 0 || EEPROM.read(i+11) == 1) {button_p2_type[i] = EEPROM.read(i+11);}
  }

  // OLED SCREEN INITIALIZATION AND INTRO ANIMATION

  u8g2.begin();
  u8g2.setFont(u8g2_font_crox5hb_tf);
  u8g2.clearBuffer();
  u8g2.setCursor((128 - u8g2.getStrWidth(LOGO)) / 2, 40);
  u8g2.print(LOGO);
  u8g2.sendBuffer();
  delay(2000);

  MIDI.sendControlChange(127, 127, 1);

}

// ###############
// MAIN LOOP FUNCTION
// ###############

void loop() {

  // ###############
  // POTENTIOMETER HANDLING
  // ###############

  for (int i = 0; i < 4; i++) {
    poti_return[i] = map(analogRead(poti_pin_number[i]), 4095, 0, POTI_MIDI_SCALE_MIN, POTI_MIDI_SCALE_MAX);
    if (abs(hysteresis[i] - sensorValue[i]) > 1) {
      hysteresis[i] = sensorValue[i];
      if (sensorValue[i] == POTI_MIDI_SCALE_MIN) {
        MIDI.sendControlChange((i+1), 0, 1);
      }
      else {
        MIDI.sendControlChange((i+1), hysteresis[i], 1);
      }
    }
  }

  // ###############
  // BUTTONS 1-8 HANDLING
  // ###############

  for (int i = 0; i < 8; i++)
  {
    if (!digitalRead(button_pin_number[i]))
    {
      if (button_state[i] == 0)
      {
        button_state[i] = 1;
        confirmScreenLock = 0;

        if (!lock_screen_switch) 
        {
          if (buttons_page == 1) 
          { 
            if (buttons_lock_for_change_type) 
            {
              if (button_type[i] == 0) {button_type[i] = 1;}
              else {button_type[i] = 0;}
              EEPROM.write(3, button_type[i]);
              EEPROM.commit();
              buttons_lock_for_change_type = 0;
              drawMsg("Saved");
            }
            else
            {
              if (button_type[i] == 0)
              {
                MIDI.sendControlChange((i+5), 127, 1);
              }
              else
              {
                if (button_last_state[i] == 0)
                {
                  button_last_state[i] = 1;
                  MIDI.sendControlChange((i+5), 127, 1);
                  if (setting_feedback_mode == 2) {drawConfirm(button_name[i], "ON");}
                }
                else
                {
                  button_last_state[i] = 0;
                  MIDI.sendControlChange((i+5), 0, 1);
                  if (setting_feedback_mode == 2) {drawConfirm(button_name[i], "OFF");}
                }
              }
            }
          }
          else if (buttons_page == 2) 
          {
            if (buttons_lock_for_change_type) 
            {
              if (button_p2_type[i] == 0) {button_p2_type[i] = 1;}
              else {button_p2_type[i] = 0;}
              EEPROM.write(11, button_p2_type[i]);
              EEPROM.commit();
              buttons_lock_for_change_type = 0;
              drawMsg("Saved");
            }
            else
            {
              if (button_p2_type[i] == 0) 
              {
                MIDI.sendControlChange((i+13), 127, 1);
              }
              else
              {
                if (button_p2_last_state[i] == 0)
                {
                  button_p2_last_state[i] = 1;
                  MIDI.sendControlChange((i+13), 127, 1);
                  if (setting_feedback_mode == 2) {drawConfirm(button_p2_name[i], "ON");}
                }
                else
                {
                  button_p2_last_state[i] = 0;
                  MIDI.sendControlChange((i+13), 0, 1);
                  if (setting_feedback_mode == 2) {drawConfirm(button_p2_name[i], "OFF");}
                }
              }
            }
            if (setting_button_shift_mode == 2) {buttons_page = 1;}
          }
        }
      }
    }
    else
    {
      button_state[i] = 0;
    }
  }

  // ###############
  // BUTTON 9 HANDLING - MENU BUTTON
  // ###############

  if (!digitalRead(button_pin_number[7])) {if (button_state[8] == 0) {
    button_state[8] = 1; 

    if (!lock_screen_switch) 
    {
      if (levels_screen_switch) { // ON LEVELS SCREEN GO TO MENU SCREEN
        levels_screen_switch = 0;
        menu_screen_switch = 1;
        settings_screen_switch = 0;
      } 
      else if (menu_screen_switch || settings_screen_switch) { // ON MENU OR SETTINGS SCREEN ADVANCE MENU POSITION
        if (menu_position < 3) {
          menu_position++;
        } 
        else {
          menu_position = 0;
        }
      } 
    }
    else
    {
      lock_screen_switch = 0;
    }
  }
  }
  else {button_state[8] = 0;}

  // ###############
  // BUTTON 10 HANDLING - OK BUTTON
  // ###############

  if (!digitalRead(button_pin_number[9])) {if (button_state[9] == 0) {
    button_state[9] = 1; 
    confirmScreenLock = 0;

    if (!lock_screen_switch) 
    {
      if (menu_screen_switch) { // ON MENU SCREEN
        if (menu_position == 0) {
          levels_screen_switch = 1;
          menu_screen_switch = 0;
          settings_screen_switch = 0;
          menu_position = 0;
        }
        if (menu_position == 1) {
          menu_position = 0;
          lock_screen_switch = 1;
        }
        if (menu_position == 2) {
          menu_position = 0;
          menu_screen_switch = 0;
          about_screen_switch = 1;
        }
        if (menu_position == 3) {
          menu_position = 0;
          levels_screen_switch = 0;
          menu_screen_switch = 0;
          settings_screen_switch = 1;
        }
      }
      else if (settings_screen_switch) { // ON SETTINGS SCREEN
        if (menu_position == 0) {
          menu_position = 0;
          if (settings_page == 1) { // PAGE 1
            if (setting_comm_mode == 1) { // CHANGE COMMUNICATION MODE
              setting_comm_mode = 2;
            }
            else {
              setting_comm_mode = 1;
            }
            EEPROM.write(0, setting_comm_mode);
            EEPROM.commit();
          }
          if (settings_page == 2) { // PAGE 2
            if (setting_feedback_mode == 1) { // CHANGE FEEDBACK MODE
              setting_feedback_mode = 2;
            }
            else {
              setting_feedback_mode = 1;
            }
            EEPROM.write(2, setting_feedback_mode);
            EEPROM.commit();
          }
          if (settings_page == 3) { // PAGE 3
            
          }
        }
        if (menu_position == 1) {
          menu_position = 1;
          if (settings_page == 1) { // PAGE 1
            if (setting_button_shift_mode == 1) { // CHANGE BUTTON SHIFT MODE
              setting_button_shift_mode = 2;
            }
            else {
              setting_button_shift_mode = 1;
            }
            EEPROM.write(1, setting_button_shift_mode);
            EEPROM.commit();
          }
          if (settings_page == 2) { // PAGE 2
            buttons_lock_for_change_type = 1;
            drawMsgInfinite("Press button");
          } 
          if (settings_page == 3) { // PAGE 3
            
          }
        }
        if (menu_position == 2) { // SWITCH PAGES
          menu_position = 2;
          if (settings_page == 1) {
           settings_page++; 
          }
          else if (settings_page == 2) {
            settings_page++;
          }
          else if (settings_page == 3) {
            settings_page = 1;
          }
        }
        if (menu_position == 3) { // GO BACK TO MENU SCREEN
          levels_screen_switch = 0;
          menu_screen_switch = 1;
          settings_screen_switch = 0;
          menu_position = 0;
          settings_page = 1;
        }
      }
      else if (levels_screen_switch) { // ON LEVELS SCREEN
        if (buttons_page == 1) {
          buttons_page = 2;
        }
        else if (buttons_page == 2) {
          buttons_page = 1;
        }
      }
      else if (about_screen_switch) { // ON ABOUT SCREEN
        if (menu_position == 0) {
          about_screen_switch = 0;
          menu_screen_switch = 1;
          menu_position = 0;
        }
      }
    }
    else
    {
      lock_screen_switch = 0;
    }
  }}
  else {button_state[9] = 0;}

  MIDI.read();

  // ###############
  // SCREEN DRAWING
  // ###############

  if (lock_screen_switch) {drawLockScreen();}
  else 
  {
    if (confirmScreenLock == 0) 
    {
      // LOGO
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_helvR10_tf);
      u8g2.setCursor(4, 12);
      u8g2.print(LOGO);

      // DRAW SCREENS
      if (levels_screen_switch) {drawLevelsScreen();}
      if (menu_screen_switch) {drawMenuScreen();}
      if (about_screen_switch) {drawAboutScreen();}
      if (settings_screen_switch) {drawSettingsScreen();}  
    }
  }

  u8g2.sendBuffer();

  // ###############
  // BLANK SCREEN LOCK FOR FULL SCREEN CONFIRMATIONS
  // ###############

  if (millis() - confirmTime < 2000) {confirmScreenLock = 1;}
  else if (millis() - confirmTime > 2000 && buttons_lock_for_change_type == 0) {confirmScreenLock = 0;}
}

// ###############
// DRAWING FUNCTIONS
// ###############

// ###############
// DRAW LEVELS SCREEN
// ###############

void drawLevelsScreen() {
  u8g2.setFont(u8g2_font_helvR10_tf);
  u8g2.setCursor(128 - 2 - u8g2.getStrWidth("levels"), 12);
  u8g2.print("levels");

  // SHIFT/PAGE INDICATOR

  if (setting_button_shift_mode == 1) 
  {
    if (buttons_page == 1) 
    {
      u8g2.setFont(u8g2_font_8x13_tf);
      u8g2.setCursor(57, 12);
      u8g2.print("1");

      u8g2.setFont(u8g2_font_6x10_tn);
      u8g2.setCursor(66, 12);
      u8g2.print("2");
    }
    else if (buttons_page == 2) 
    {
      u8g2.setFont(u8g2_font_6x10_tn);
      u8g2.setCursor(59, 12);
      u8g2.print("1");

      u8g2.setFont(u8g2_font_8x13_tf);
      u8g2.setCursor(66, 12);
      u8g2.print("2");
    }
  }
  else if (setting_button_shift_mode == 2) 
  {
    if (buttons_page == 2) 
    {
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.setCursor(48, 12);
      u8g2.print("SHIFT");
    }
  }

  // LEVELS

  for (int i = 0; i < 4; i++)
  {
    u8g2.setFont(u8g2_font_luRS10_tf);
    u8g2.setCursor(4, 14 + ((i+1) * 12));
    u8g2.print("L");
    u8g2.print(i+1);
    u8g2.drawFrame(21, 4 + ((i+1) * 12), 105, 10);

    if (setting_feedback_mode == 1) 
    {
      if (hysteresis[i] > POTI_MIDI_SCALE_MIN && poti_return[i] > POTI_MIDI_SCALE_MIN + 1) 
      {
        u8g2.drawBox(23, 6 + ((i+1) * 12), map(hysteresis[i], POTI_MIDI_SCALE_MIN, POTI_MIDI_SCALE_MAX, 0, 101), 3);
        u8g2.drawBox(23, 9 + ((i+1) * 12), map(poti_return[i], POTI_MIDI_SCALE_MIN, POTI_MIDI_SCALE_MAX, 0, 101), 3);
      }
      else 
      {
        u8g2.setCursor(23, 12 + (i * 12));
        u8g2.setFont(u8g2_font_spleen5x8_mf);
        u8g2.print("MUTE");
      }
    }
    else if (setting_feedback_mode == 2) 
    {
      if (hysteresis[i] > POTI_MIDI_SCALE_MIN) 
      {
        u8g2.drawBox(23, 6 + ((i+1) * 12), map(hysteresis[i], POTI_MIDI_SCALE_MIN, POTI_MIDI_SCALE_MAX, 0, 101), 6);
      }
      else 
      {
        u8g2.setCursor(23, 12 + ((i+1) * 12));
        u8g2.setFont(u8g2_font_spleen5x8_mf);
        u8g2.print("MUTE");
      }
    }
  }
}

void drawMenuScreen() {
  u8g2.setFont(u8g2_font_helvR10_tf);
  u8g2.setCursor(128 - 2 - u8g2.getStrWidth("menu"), 12);
  u8g2.print("menu");

  u8g2.setFont(u8g2_font_luRS10_tf);

  for (int i = 0; i < 4; i++) 
  {
    u8g2.setCursor(12, 26 + (i * 12));
    u8g2.print(menu_items[i]);
  }

  u8g2.setCursor(4, 26 + (menu_position * 12));
  u8g2.print(">");
}

void drawAboutScreen() {
  u8g2.setFont(u8g2_font_helvR10_tf);
  u8g2.setCursor(128 - 2 - u8g2.getStrWidth("about"), 12);
  u8g2.print("about");

  u8g2.setFont(u8g2_font_7x13_tf);

  for (int i = 0; i < 3; i++) 
  {
    u8g2.setCursor(4, 26 + (i * 12));
    u8g2.print(about_items[i]);
  }

  u8g2.setFont(u8g2_font_luRS10_tf);
  u8g2.setCursor(12, 62);
  u8g2.print(about_items[3]);

  u8g2.setCursor(4, 26 + (menu_position * 12));
  u8g2.print(">");
}

void drawSettingsScreen() {
  u8g2.setFont(u8g2_font_helvR10_tf);
  u8g2.setCursor(128 - 2 - u8g2.getStrWidth("settings"), 12);
  u8g2.print("settings");

  u8g2.setFont(u8g2_font_luRS10_tf);

  u8g2.setCursor(12, 26); // MENU POSITION 1
  if (settings_page == 1) 
  {
    if (setting_comm_mode == 1) {u8g2.print("mode: midi");}
    else if (setting_comm_mode == 2) {u8g2.print("mode:");}
  }
  else if (settings_page == 2) 
  {
    if (setting_feedback_mode == 1) {u8g2.print("feedback: on");}
    else if (setting_feedback_mode == 2) {u8g2.print("feedback: off");}
  }
  else if (settings_page == 3) {u8g2.print("");}

  u8g2.setCursor(12, 38); // MENU POSITION 2
  if (settings_page == 1) 
  {
    if (setting_button_shift_mode == 1) {u8g2.print("btn mode: switch");}
    else if (setting_button_shift_mode == 2) {u8g2.print("btn mode: shift");}
  }
  else if (settings_page == 2) {u8g2.print("chg btn type");}
  else if (settings_page == 3) {u8g2.print("");}

  u8g2.setCursor(12, 50); // MENU POSITION 3
  u8g2.print("next page");
  
  u8g2.setCursor(12, 62); // MENU POSITION 4
  u8g2.print("exit");

  u8g2.setCursor(4, 26 + (menu_position * 12));
  u8g2.print(">");
}

void drawConfirm(const char* feature, const char* status) {
  u8g2.clearBuffer();

  char buf[25];
  strcpy(buf, feature);
  strcat(buf, ": ");
  strcat(buf, status);
  u8g2.setFont(u8g2_font_helvB12_tf);
  u8g2.setCursor((128 - u8g2.getStrWidth(buf)) / 2, 40);
  u8g2.print(buf);

  u8g2.sendBuffer();

  confirmTime = millis();
  confirmScreenLock = 1;

}

void drawMsgInfinite(const char* msg) {
  u8g2.clearBuffer();

  char buf[25];
  strcpy(buf, msg);
  u8g2.setFont(u8g2_font_helvB12_tf);
  u8g2.setCursor((128 - u8g2.getStrWidth(buf)) / 2, 40);
  u8g2.print(buf);

  u8g2.sendBuffer();

  confirmScreenLock = 1;
}

void drawMsg(const char* msg) {
  u8g2.clearBuffer();

  char buf[25];
  strcpy(buf, msg);
  u8g2.setFont(u8g2_font_helvB12_tf);
  u8g2.setCursor((128 - u8g2.getStrWidth(buf)) / 2, 40);
  u8g2.print(buf);

  u8g2.sendBuffer();

  confirmTime = millis();
  confirmScreenLock = 1;
}

void drawLockScreen() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_helvR10_tf);
  u8g2.setCursor(128 - 2 - u8g2.getStrWidth("lock"), 12);
  u8g2.print("lock");
  u8g2.sendBuffer();
}

void handleControlChange(byte channel, byte number, byte value) {

  if (setting_feedback_mode)
  {
    for (int i = 0; i < 8; i++)
    {
      if (number == (i+5))
      {
        if (value == 0x7f)
        {
          button_last_state[i] = 1;
          drawConfirm(button_name[i], "ON");
        }
        else if (value == 0x00)
        {
          button_last_state[i] = 0;
          drawConfirm(button_name[i], "OFF");
        }
      }
    }

    for (int i = 0; i < 8; i++)
    {
      if (number == (i+13))
      {
        if (value == 0x7f)
        {
          button_p2_last_state[i] = 1;
          drawConfirm(button_p2_name[i], "ON");
        }
        else if (value == 0x00)
        {
          button_p2_last_state[i] = 0;
          drawConfirm(button_p2_name[i], "OFF");
        }
      }
    }
  }

  for (int i = 0; i < 4; i++)
  {
    if (number == (i+1))
    {
      if (value == 0x00) {poti_return[i] = POTI_MIDI_SCALE_MIN;}
      else if (static_cast<int>(value) > POTI_MIDI_SCALE_MIN) {poti_return[i] = static_cast<int>(value);}
    }
  }
}

  // Fonts
  // u8g2_font_helvR12_tf - normal
  // u8g2_font_helvB12_tf - bold
  // u8g2_font_helvR10_tf - normal
  // u8g2_font_helvB10_tf - bold
  // u8g2_font_luRS10_tf - 10px
  // u8g2_font_spleen5x8_mf - 6px

  // u8g2_font_6x12_tf
  // u8g2_font_6x10_tn 

  // u8g2_font_7x14_tf 
  // u8g2_font_7x13_tf 
  // u8g2_font_6x13_tf 

  // Black background with white text
  // u8g2.setFontMode(0);
  // u8g2.setDrawColor(1);

  // White background with black text
  // u8g2.setFontMode(0);
  // u8g2.setDrawColor(0);

  // White bitmap with transparent background
  // u8g2.setFontMode(1);
  // u8g2.setDrawColor(0);

  // Black bitmap with transparent background
  // u8g2.setFontMode(0);
  // u8g2.setDrawColor(1);