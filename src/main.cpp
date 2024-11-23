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

static int averageValue_1 = 0;
static int averageValue_2 = 0;
static int averageValue_3 = 0;
static int averageValue_4 = 0;
static int hysteresis_1 = 0;
static int hysteresis_2 = 0;
static int hysteresis_3 = 0;
static int hysteresis_4 = 0;
static int poti_return_1 = 0;
static int poti_return_2 = 0;
static int poti_return_3 = 0;
static int poti_return_4 = 0;

bool button1_state = 0;
bool button2_state = 0;
bool button3_state = 0;
bool button4_state = 0;
bool button5_state = 0;
bool button6_state = 0;
bool button7_state = 0;
bool button8_state = 0;
bool button9_state = 0;
bool button10_state = 0;

bool button1_last_state = 0;
bool button2_last_state = 0;
bool button3_last_state = 0;
bool button4_last_state = 0;
bool button5_last_state = 0;
bool button6_last_state = 0;
bool button7_last_state = 0;
bool button8_last_state = 0;
bool button1_p2_last_state = 0;
bool button2_p2_last_state = 0;
bool button3_p2_last_state = 0;
bool button4_p2_last_state = 0;
bool button5_p2_last_state = 0;
bool button6_p2_last_state = 0;
bool button7_p2_last_state = 0;
bool button8_p2_last_state = 0;

bool button1_type = 0; // 0 - momentary, 1 - toggle
bool button2_type = 0;
bool button3_type = 0;
bool button4_type = 0;
bool button5_type = 0;
bool button6_type = 0;
bool button7_type = 0;
bool button8_type = 0;
bool button1_p2_type = 0;
bool button2_p2_type = 0;
bool button3_p2_type = 0;
bool button4_p2_type = 0;
bool button5_p2_type = 0;
bool button6_p2_type = 0;
bool button7_p2_type = 0;
bool button8_p2_type = 0;


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
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);
  pinMode(BUTTON_3, INPUT_PULLUP);
  pinMode(BUTTON_4, INPUT_PULLUP);
  pinMode(BUTTON_5, INPUT_PULLUP);
  pinMode(BUTTON_6, INPUT_PULLUP);
  pinMode(BUTTON_7, INPUT_PULLUP);
  pinMode(BUTTON_8, INPUT_PULLUP);
  pinMode(BUTTON_9, INPUT_PULLUP);
  pinMode(BUTTON_10, INPUT_PULLUP);

  MIDI.turnThruOff();
  MIDI.setHandleControlChange(handleControlChange);

  // EEPROM READ SETTINGS ON STARTUP

  EEPROM.begin(EEPROM_SIZE);
  if (EEPROM.read(0) == 1 || EEPROM.read(0) == 2) {setting_comm_mode = EEPROM.read(0);}
  if (EEPROM.read(1) == 1 || EEPROM.read(1) == 2) {setting_button_shift_mode = EEPROM.read(1);}
  if (EEPROM.read(2) == 1 || EEPROM.read(2) == 2) {setting_feedback_mode = EEPROM.read(2);}
  if (EEPROM.read(3) == 0 || EEPROM.read(3) == 1) {button1_type = EEPROM.read(3);}
  if (EEPROM.read(4) == 0 || EEPROM.read(4) == 1) {button2_type = EEPROM.read(4);}
  if (EEPROM.read(5) == 0 || EEPROM.read(5) == 1) {button3_type = EEPROM.read(5);}
  if (EEPROM.read(6) == 0 || EEPROM.read(6) == 1) {button4_type = EEPROM.read(6);}
  if (EEPROM.read(7) == 0 || EEPROM.read(7) == 1) {button5_type = EEPROM.read(7);}
  if (EEPROM.read(8) == 0 || EEPROM.read(8) == 1) {button6_type = EEPROM.read(8);}
  if (EEPROM.read(9) == 0 || EEPROM.read(9) == 1) {button7_type = EEPROM.read(9);}
  if (EEPROM.read(10) == 0 || EEPROM.read(10) == 1) {button8_type = EEPROM.read(10);}
  if (EEPROM.read(11) == 0 || EEPROM.read(11) == 1) {button1_p2_type = EEPROM.read(11);}
  if (EEPROM.read(12) == 0 || EEPROM.read(12) == 1) {button2_p2_type = EEPROM.read(12);}
  if (EEPROM.read(13) == 0 || EEPROM.read(13) == 1) {button3_p2_type = EEPROM.read(13);}
  if (EEPROM.read(14) == 0 || EEPROM.read(14) == 1) {button4_p2_type = EEPROM.read(14);}
  if (EEPROM.read(15) == 0 || EEPROM.read(15) == 1) {button5_p2_type = EEPROM.read(15);}
  if (EEPROM.read(16) == 0 || EEPROM.read(16) == 1) {button6_p2_type = EEPROM.read(16);}
  if (EEPROM.read(17) == 0 || EEPROM.read(17) == 1) {button7_p2_type = EEPROM.read(17);}
  if (EEPROM.read(18) == 0 || EEPROM.read(18) == 1) {button8_p2_type = EEPROM.read(18);}
  

  // OLED SCREEN INITIALIZATION AND INTRO ANIMATION

  u8g2.begin();
  u8g2.setFont(u8g2_font_crox5hb_tf);
  u8g2.clearBuffer();
  u8g2.setCursor((128 - u8g2.getStrWidth("deej")) / 2, 40);
  u8g2.print("deej");
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

  int sensorValue_1 = map(analogRead(POTI_1), 4095, 0, POTI_MIDI_SCALE_MIN, POTI_MIDI_SCALE_MAX);
  if (abs(hysteresis_1 - sensorValue_1) > 1) {
    hysteresis_1 = sensorValue_1;
    if (sensorValue_1 == POTI_MIDI_SCALE_MIN) {
      MIDI.sendControlChange(1, 0, 1);
    }
    else {
      MIDI.sendControlChange(1, hysteresis_1, 1);
    }
  }

  int sensorValue_2 = map(analogRead(POTI_2), 4095, 0, POTI_MIDI_SCALE_MIN, POTI_MIDI_SCALE_MAX);
  if (abs(hysteresis_2 - sensorValue_2) > 1) {
    hysteresis_2 = sensorValue_2;
    if (sensorValue_2 == POTI_MIDI_SCALE_MIN) {
      MIDI.sendControlChange(2, 0, 1);
    }
    else {
      MIDI.sendControlChange(2, hysteresis_2, 1);
    }
  }

  int sensorValue_3 = map(analogRead(POTI_3), 4095, 0, POTI_MIDI_SCALE_MIN, POTI_MIDI_SCALE_MAX);
  if (abs(hysteresis_3 - sensorValue_3) > 1) {
    hysteresis_3 = sensorValue_3;
    if (sensorValue_3 == POTI_MIDI_SCALE_MIN) {
      MIDI.sendControlChange(3, 0, 1);
    }
    else {
      MIDI.sendControlChange(3, hysteresis_3, 1);
    }
  }

  int sensorValue_4 = map(analogRead(POTI_4), 4095, 0, POTI_MIDI_SCALE_MIN, POTI_MIDI_SCALE_MAX);
  if (abs(hysteresis_4 - sensorValue_4) > 1) {
    hysteresis_4 = sensorValue_4;
    if (sensorValue_4 == POTI_MIDI_SCALE_MIN) {
      MIDI.sendControlChange(4, 0, 1);
    }
    else {
      MIDI.sendControlChange(4, hysteresis_4, 1);
    }
  }

  // ###############
  // BUTTON HANDLING
  // ###############

  // ###############
  // BUTTON 1
  // ###############

  bool button_1 = !digitalRead(BUTTON_1);
  if (button_1)
  {
    if (button1_state == 0)
    {
      button1_state = 1;
      confirmScreenLock = 0;

      if (!lock_screen_switch) 
      {
        if (buttons_page == 1) 
        { 
          if (buttons_lock_for_change_type) 
          {
            if (button1_type == 0) {button1_type = 1;}
            else {button1_type = 0;}
            EEPROM.write(3, button1_type);
            EEPROM.commit();
            buttons_lock_for_change_type = 0;
            drawMsg("Saved");
          }
          else
          {
            if (button1_type == 0)
            {
              MIDI.sendControlChange(5, 127, 1);
            }
            else
            {
              if (button1_last_state == 0)
              {
                button1_last_state = 1;
                MIDI.sendControlChange(5, 127, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_1_NAME, "ON");}
              }
              else
              {
                button1_last_state = 0;
                MIDI.sendControlChange(5, 0, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_1_NAME, "OFF");}
              }
            }
          }
        }
        else if (buttons_page == 2) 
        {
          if (buttons_lock_for_change_type) 
          {
            if (button1_p2_type == 0) {button1_p2_type = 1;}
            else {button1_p2_type = 0;}
            EEPROM.write(11, button1_p2_type);
            EEPROM.commit();
            buttons_lock_for_change_type = 0;
            drawMsg("Saved");
          }
          else
          {
            if (button1_p2_type == 0) 
            {
              MIDI.sendControlChange(13, 127, 1);
            }
            else
            {
              if (button1_p2_last_state == 0)
              {
                button1_p2_last_state = 1;
                MIDI.sendControlChange(13, 127, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_1_P2_NAME, "ON");}
              }
              else
              {
                button1_p2_last_state = 0;
                MIDI.sendControlChange(13, 0, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_1_P2_NAME, "OFF");}
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
    button1_state = 0;
  }

  // ###############
  // BUTTON 2
  // ###############

  bool button_2 = !digitalRead(BUTTON_2);
  if (button_2)
  {
    if (button2_state == 0)
    {
      button2_state = 1;
      confirmScreenLock = 0;

      if (!lock_screen_switch)
      {
        if (buttons_page == 1) {
          if (buttons_lock_for_change_type) {
            if (button2_type == 0) {button2_type = 1;}
            else {button2_type = 0;}
            EEPROM.write(4, button2_type);
            EEPROM.commit();
            buttons_lock_for_change_type = 0;
            drawMsg("Saved");
          }
          else {
            if (button2_type == 0) {
              MIDI.sendControlChange(6, 127, 1);
            }
            else {
              if (button2_last_state == 0) {
                button2_last_state = 1;
                MIDI.sendControlChange(6, 127, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_2_NAME, "ON");}
              }
              else {
                button2_last_state = 0;
                MIDI.sendControlChange(6, 0, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_2_NAME, "OFF");}
              }
            }
          }
        }
        else if (buttons_page == 2) {
          if (buttons_lock_for_change_type) {
            if (button2_p2_type == 0) {button2_p2_type = 1;}
            else {button2_p2_type = 0;}
            EEPROM.write(12, button2_p2_type);
            EEPROM.commit();
            buttons_lock_for_change_type = 0;
            drawMsg("Saved");
          }
          else {
            if (button2_p2_type == 0) {
              MIDI.sendControlChange(14, 127, 1);
            }
            else {
              if (button2_p2_last_state == 0) {
                button2_p2_last_state = 1;
                MIDI.sendControlChange(14, 127, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_2_P2_NAME, "ON");}
              }
              else {
                button2_p2_last_state = 0;
                MIDI.sendControlChange(14, 0, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_2_P2_NAME, "OFF");}
              }
            }
          }
          if (setting_button_shift_mode == 2) {buttons_page = 1;}
        }
      }
    }
  }
  else {button2_state = 0;}

  // ###############
  // BUTTON 3
  // ###############

  bool button_3 = !digitalRead(BUTTON_3);
  if (button_3)
  {
    if (button3_state == 0)
    {
      button3_state = 1;
      confirmScreenLock = 0;

      if (!lock_screen_switch) 
      {
        if (buttons_page == 1) {
          if (buttons_lock_for_change_type) 
          {
            if (button3_type == 0) {button3_type = 1;}
            else {button3_type = 0;}
            EEPROM.write(5, button3_type);
            EEPROM.commit();
            buttons_lock_for_change_type = 0;
            drawMsg("Saved");
          }
          else {
            if (button3_type == 0) {
              MIDI.sendControlChange(7, 127, 1);
            }
            else {
              if (button3_last_state == 0) {
                button3_last_state = 1;
                MIDI.sendControlChange(7, 127, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_3_NAME, "ON");}
              }
              else {
                button3_last_state = 0;
                MIDI.sendControlChange(7, 0, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_3_NAME, "OFF");}
              }
            }
          }
        }
        else if (buttons_page == 2) {
          if (buttons_lock_for_change_type) 
          {
            if (button3_p2_type == 0) {button3_p2_type = 1;}
            else {button3_p2_type = 0;}
            EEPROM.write(13, button3_p2_type);
            EEPROM.commit();
            buttons_lock_for_change_type = 0;
            drawMsg("Saved");
          }
          else {
            if (button3_p2_type == 0) {
              MIDI.sendControlChange(15, 127, 1);
            }
            else {
              if (button3_p2_last_state == 0) {
                button3_p2_last_state = 1;
                MIDI.sendControlChange(15, 127, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_3_P2_NAME, "ON");}
              }
              else {
                button3_p2_last_state = 0;
                MIDI.sendControlChange(15, 0, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_3_P2_NAME, "OFF");}
              }
            }
          }
          if (setting_button_shift_mode == 2) {buttons_page = 1;}
        }
      }
    }
  }
  else {button3_state = 0;}

  // ###############
  // BUTTON 4
  // ###############

  bool button_4 = !digitalRead(BUTTON_4);
  if (button_4)
  {
    if (button4_state == 0)
    {
      button4_state = 1;
      confirmScreenLock = 0;

      if (!lock_screen_switch) 
      {
        if (buttons_page == 1) {
          if (buttons_lock_for_change_type) {
            if (button4_type == 0) {button4_type = 1;}
            else {button4_type = 0;}
            EEPROM.write(6, button4_type);
            EEPROM.commit();
            buttons_lock_for_change_type = 0;
            drawMsg("Saved");
          }
          else {
            if (button4_type == 0) {
              MIDI.sendControlChange(8, 127, 1);
            }
            else {
              if (button4_last_state == 0) {
                button4_last_state = 1;
                MIDI.sendControlChange(8, 127, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_4_NAME, "ON");}
              }
              else {
                button4_last_state = 0;
                MIDI.sendControlChange(8, 0, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_4_NAME, "OFF");}
              }
            }
          }
        }
        else if (buttons_page == 2) {
          if (buttons_lock_for_change_type) {
            if (button4_p2_type == 0) {button4_p2_type = 1;}
            else {button4_p2_type = 0;}
            EEPROM.write(14, button4_p2_type);
            EEPROM.commit();
            buttons_lock_for_change_type = 0;
            drawMsg("Saved");
          }
          else {
            if (button4_p2_type == 0) {
              MIDI.sendControlChange(16, 127, 1);
            }
            else {
              if (button4_p2_last_state == 0) {
                button4_p2_last_state = 1;
                MIDI.sendControlChange(16, 127, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_4_P2_NAME, "ON");}
              }
              else {
                button4_p2_last_state = 0;
                MIDI.sendControlChange(16, 0, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_4_P2_NAME, "OFF");}
              }
            }
          }
          if (setting_button_shift_mode == 2) {buttons_page = 1;}
        }
      }
    }
  }
  else {button4_state = 0;}

  // ###############
  // BUTTON 5
  // ###############

  bool button_5 = !digitalRead(BUTTON_5);
  if (button_5)
  {
    if (button5_state == 0)
    {
      button5_state = 1;
      confirmScreenLock = 0;

      if (!lock_screen_switch) 
      {
        if (buttons_page == 1) {
          if (buttons_lock_for_change_type)
          {
            if (button5_type == 0) {button5_type = 1;}
            else {button5_type = 0;}
            EEPROM.write(7, button5_type);
            EEPROM.commit();
            buttons_lock_for_change_type = 0;
            drawMsg("Saved");
          }
          else {
            if (button5_type == 0) {
              MIDI.sendControlChange(9, 127, 1);
            }
            else {
              if (button5_last_state == 0) {
                button5_last_state = 1;
                MIDI.sendControlChange(9, 127, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_5_NAME, "ON");}
              }
              else {
                button5_last_state = 0;
                MIDI.sendControlChange(9, 0, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_5_NAME, "OFF");}
              }
            }
          }
        }
        else if (buttons_page == 2) {
          if (buttons_lock_for_change_type) {
            if (button5_p2_type == 0) {button5_p2_type = 1;}
            else {button5_p2_type = 0;}
            EEPROM.write(15, button5_p2_type);
            EEPROM.commit();
            buttons_lock_for_change_type = 0;
            drawMsg("Saved");
          }
          else {
            if (button5_p2_type == 0) {
              MIDI.sendControlChange(17, 127, 1);
            }
            else {
              if (button5_p2_last_state == 0) {
                button5_p2_last_state = 1;
                MIDI.sendControlChange(17, 127, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_5_P2_NAME, "ON");}
              }
              else {
                button5_p2_last_state = 0;
                MIDI.sendControlChange(17, 0, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_5_P2_NAME, "OFF");}
              }
            }
          }
          if (setting_button_shift_mode == 2) {buttons_page = 1;}
        }
      }
    }
  }
  else {button5_state = 0;}

  // ###############
  // BUTTON 6
  // ###############

  bool button_6 = !digitalRead(BUTTON_6);
  if (button_6)
  {
    if (button6_state == 0)
    {
      button6_state = 1;
      confirmScreenLock = 0;

      if (!lock_screen_switch) 
      {
        if (buttons_page == 1) {
          if (buttons_lock_for_change_type)
          {
            if (button6_type == 0) {button6_type = 1;}
            else {button6_type = 0;}
            EEPROM.write(8, button6_type);
            EEPROM.commit();
            buttons_lock_for_change_type = 0;
            drawMsg("Saved");
          }
          else {
            if (button6_type == 0) {
              MIDI.sendControlChange(10, 127, 1);
            }
            else {
              if (button6_last_state == 0) {
                button6_last_state = 1;
                MIDI.sendControlChange(10, 127, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_6_NAME, "ON");}
              }
              else {
                button6_last_state = 0;
                MIDI.sendControlChange(10, 0, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_6_NAME, "OFF");}
              }
            }
          }
        }
        else if (buttons_page == 2) {
          if (buttons_lock_for_change_type) {
            if (button6_p2_type == 0) {button6_p2_type = 1;}
            else {button6_p2_type = 0;}
            EEPROM.write(16, button6_p2_type);
            EEPROM.commit();
            buttons_lock_for_change_type = 0;
            drawMsg("Saved");
          }
          else {
            if (button6_p2_type == 0) {
              MIDI.sendControlChange(18, 127, 1);
            }
            else {
              if (button6_p2_last_state == 0) {
                button6_p2_last_state = 1;
                MIDI.sendControlChange(18, 127, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_6_P2_NAME, "ON");}
              }
              else {
                button6_p2_last_state = 0;
                MIDI.sendControlChange(18, 0, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_6_P2_NAME, "OFF");}
              }
            }
          }
          if (setting_button_shift_mode == 2) {buttons_page = 1;}
        }
      }
    }
  }
  else {button6_state = 0;}

  // ###############
  // BUTTON 7
  // ###############

  bool button_7 = !digitalRead(BUTTON_7);
  if (button_7)
  {
    if (button7_state == 0)
    {
      button7_state = 1;
      confirmScreenLock = 0;

      if (!lock_screen_switch) 
      {
        if (buttons_page == 1) {
          if (buttons_lock_for_change_type)
          {
            if (button7_type == 0) {button7_type = 1;}
            else {button7_type = 0;}
            EEPROM.write(9, button7_type);
            EEPROM.commit();
            buttons_lock_for_change_type = 0;
            drawMsg("Saved");
          }
          else {
            if (button7_type == 0) {
              MIDI.sendControlChange(11, 127, 1);
            }
            else {
              if (button7_last_state == 0) {
                button7_last_state = 1;
                MIDI.sendControlChange(11, 127, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_7_NAME, "ON");}
              }
              else {
                button7_last_state = 0;
                MIDI.sendControlChange(11, 0, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_7_NAME, "OFF");}
              }
            }
          }
        }
        else if (buttons_page == 2) {
          if (buttons_lock_for_change_type) {
            if (button7_p2_type == 0) {button7_p2_type = 1;}
            else {button7_p2_type = 0;}
            EEPROM.write(17, button7_p2_type);
            EEPROM.commit();
            buttons_lock_for_change_type = 0;
            drawMsg("Saved");
          }
          else {
            if (button7_p2_type == 0) {
              MIDI.sendControlChange(19, 127, 1);
            }
            else {
              if (button7_p2_last_state == 0) {
                button7_p2_last_state = 1;
                MIDI.sendControlChange(19, 127, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_7_P2_NAME, "ON");}
              }
              else {
                button7_p2_last_state = 0;
                MIDI.sendControlChange(19, 0, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_7_P2_NAME, "OFF");}
              }
            }
          }
          if (setting_button_shift_mode == 2) {buttons_page = 1;}
        }
      }
    }
  }
  else {button7_state = 0;}

  // ###############
  // BUTTON 8
  // ###############

  bool button_8 = !digitalRead(BUTTON_8);
  if (button_8)
  {
    if (button8_state == 0)
    {
      button8_state = 1;
      confirmScreenLock = 0;

      if (!lock_screen_switch) 
      {
        if (buttons_page == 1) {
          if (buttons_lock_for_change_type) {
            if (button8_type == 0) {button8_type = 1;}
            else {button8_type = 0;}
            EEPROM.write(10, button8_type);
            EEPROM.commit();
            buttons_lock_for_change_type = 0;
            drawMsg("Saved");
          }
          else {
            if (button8_type == 0) {
              MIDI.sendControlChange(12, 127, 1);
            }
            else {
              if (button8_last_state == 0) {
                button8_last_state = 1;
                MIDI.sendControlChange(12, 127, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_8_NAME, "ON");}
              }
              else {
                button8_last_state = 0;
                MIDI.sendControlChange(12, 0, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_8_NAME, "OFF");}
              }
            }
          }
        }
        else if (buttons_page == 2) {
          if (buttons_lock_for_change_type) {
            if (button8_p2_type == 0) {button8_p2_type = 1;}
            else {button8_p2_type = 0;}
            EEPROM.write(18, button8_p2_type);
            EEPROM.commit();
            buttons_lock_for_change_type = 0;
            drawMsg("Saved");
          }
          else {
            if (button8_p2_type == 0) {
              MIDI.sendControlChange(20, 127, 1);
            }
            else {
              if (button8_p2_last_state == 0) {
                button8_p2_last_state = 1;
                MIDI.sendControlChange(20, 127, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_8_P2_NAME, "ON");}
              }
              else {
                button8_p2_last_state = 0;
                MIDI.sendControlChange(20, 0, 1);
                if (setting_feedback_mode == 2) {drawConfirm(BUTTON_8_P2_NAME, "OFF");}
              }
            }
          }
          if (setting_button_shift_mode == 2) {buttons_page = 1;}
        }
      }
    }
  }
  else {button8_state = 0;}

  // ###############
  // BUTTON 9 - MENU BUTTON
  // ###############

  bool button_9 = !digitalRead(BUTTON_9); // MENU BUTTON
  if (button_9) {if (button9_state == 0) {
    button9_state = 1; 

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
  else {button9_state = 0;}

  // ###############
  // BUTTON 10 - OK BUTTON
  // ###############

  bool button_10 = !digitalRead(BUTTON_10); // OK BUTTON
  if (button_10) {if (button10_state == 0) {
    button10_state = 1; 
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
  else {button10_state = 0;}


  MIDI.read();

  // ###############
  // SCREEN DRAWING
  // ###############

  if (lock_screen_switch) 
  {
    drawLockScreen();
  }
  else 
  {
    if (confirmScreenLock == 0) 
    {
      // LOGO
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_helvR10_tf);
      u8g2.setCursor(4, 12);
      u8g2.print("deej");

      // DRAW SCREENS
      if (levels_screen_switch) {
        drawLevelsScreen();
      }

      if (menu_screen_switch) {
        drawMenuScreen();
      }

      if (about_screen_switch) {
        drawAboutScreen();
      }

      if (settings_screen_switch) {
        drawSettingsScreen();
      }  
    }
  }

  u8g2.sendBuffer();

  // ###############
  // BLANK SCREEN LOCK FOR FULL SCREEN CONFIRMATIONS
  // ###############

  if (millis() - confirmTime < 2000) {
    confirmScreenLock = 1;
  }
  else if (millis() - confirmTime > 2000 && buttons_lock_for_change_type == 0) {
    confirmScreenLock = 0;
  }
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

  u8g2.setFont(u8g2_font_luRS10_tf);
  u8g2.setCursor(4, 26);
  u8g2.print("L1");
  u8g2.drawFrame(21, 16, 105, 10);

  if (setting_feedback_mode == 1) {
    if (hysteresis_1 > POTI_MIDI_SCALE_MIN && poti_return_1 > POTI_MIDI_SCALE_MIN + 1) {
      u8g2.drawBox(23, 18, map(hysteresis_1, POTI_MIDI_SCALE_MIN, POTI_MIDI_SCALE_MAX, 0, 101), 3);
      u8g2.drawBox(23, 21, map(poti_return_1, POTI_MIDI_SCALE_MIN, POTI_MIDI_SCALE_MAX, 0, 101), 3);
    }
    else {
      u8g2.setCursor(23, 24);
      u8g2.setFont(u8g2_font_spleen5x8_mf);
      u8g2.print("MUTE");
    }
  }
  else if (setting_feedback_mode == 2) {
    if (hysteresis_1 > POTI_MIDI_SCALE_MIN) {
      u8g2.drawBox(23, 18, map(hysteresis_1, POTI_MIDI_SCALE_MIN, POTI_MIDI_SCALE_MAX, 0, 101), 6);
    }
    else {
      u8g2.setCursor(23, 24);
      u8g2.setFont(u8g2_font_spleen5x8_mf);
      u8g2.print("MUTE");
    }
  }

  u8g2.setFont(u8g2_font_luRS10_tf);
  u8g2.setCursor(4, 38);
  u8g2.print("L2");
  u8g2.drawFrame(21, 28, 105, 10);

  if (setting_feedback_mode == 1) {
    if (hysteresis_2 > POTI_MIDI_SCALE_MIN && poti_return_2 > POTI_MIDI_SCALE_MIN + 1) {
      u8g2.drawBox(23, 30, map(hysteresis_2, POTI_MIDI_SCALE_MIN, POTI_MIDI_SCALE_MAX, 0, 101), 3);
      u8g2.drawBox(23, 33, map(poti_return_2, POTI_MIDI_SCALE_MIN, POTI_MIDI_SCALE_MAX, 0, 101), 3);
    }
    else {
      u8g2.setCursor(23, 36);
      u8g2.setFont(u8g2_font_spleen5x8_mf);
      u8g2.print("MUTE");
    }
  }
  else if (setting_feedback_mode == 2) {
    if (hysteresis_2 > POTI_MIDI_SCALE_MIN) {
      u8g2.drawBox(23, 30, map(hysteresis_2, POTI_MIDI_SCALE_MIN, POTI_MIDI_SCALE_MAX, 0, 101), 6);
    }
    else {
      u8g2.setCursor(23, 36);
      u8g2.setFont(u8g2_font_spleen5x8_mf);
      u8g2.print("MUTE");
    }
  }

  u8g2.setFont(u8g2_font_luRS10_tf);
  u8g2.setCursor(4, 50);
  u8g2.print("L3");
  u8g2.drawFrame(21, 40, 105, 10);

  if (setting_feedback_mode == 1) {
    if (hysteresis_3 > POTI_MIDI_SCALE_MIN && poti_return_3 > POTI_MIDI_SCALE_MIN + 1) {
      u8g2.drawBox(23, 42, map(hysteresis_3, POTI_MIDI_SCALE_MIN, POTI_MIDI_SCALE_MAX, 0, 101), 3);
      u8g2.drawBox(23, 45, map(poti_return_3, POTI_MIDI_SCALE_MIN, POTI_MIDI_SCALE_MAX, 0, 101), 3);
    }
    else {
      u8g2.setCursor(23, 48);
      u8g2.setFont(u8g2_font_spleen5x8_mf);
      u8g2.print("MUTE");
    }
  }
  else if (setting_feedback_mode == 2) {
    if (hysteresis_3 > POTI_MIDI_SCALE_MIN) {
      u8g2.drawBox(23, 42, map(hysteresis_3, POTI_MIDI_SCALE_MIN, POTI_MIDI_SCALE_MAX, 0, 101), 6);
    }
    else {
      u8g2.setCursor(23, 48);
      u8g2.setFont(u8g2_font_spleen5x8_mf);
      u8g2.print("MUTE");
    }
  }

  u8g2.setFont(u8g2_font_luRS10_tf);
  u8g2.setCursor(4, 62);
  u8g2.print("L4");
  u8g2.drawFrame(21, 52, 105, 10);

  if (setting_feedback_mode == 1) {
    if (hysteresis_4 > POTI_MIDI_SCALE_MIN && poti_return_4 > POTI_MIDI_SCALE_MIN + 1) {
      u8g2.drawBox(23, 54, map(hysteresis_4, POTI_MIDI_SCALE_MIN, POTI_MIDI_SCALE_MAX, 0, 101), 3);
      u8g2.drawBox(23, 57, map(poti_return_4, POTI_MIDI_SCALE_MIN, POTI_MIDI_SCALE_MAX, 0, 101), 3);
    }
    else {
      u8g2.setCursor(23, 60);
      u8g2.setFont(u8g2_font_spleen5x8_mf);
      u8g2.print("MUTE");
    }
  }
  else if (setting_feedback_mode == 2) {
    if (hysteresis_4 > POTI_MIDI_SCALE_MIN) {
      u8g2.drawBox(23, 54, map(hysteresis_4, POTI_MIDI_SCALE_MIN, POTI_MIDI_SCALE_MAX, 0, 101), 6);
    }
    else {
      u8g2.setCursor(23, 60);
      u8g2.setFont(u8g2_font_spleen5x8_mf);
      u8g2.print("MUTE");
    }
  }
}

void drawMenuScreen(){
  u8g2.setFont(u8g2_font_helvR10_tf);
  u8g2.setCursor(128 - 2 - u8g2.getStrWidth("menu"), 12);
  u8g2.print("menu");

  u8g2.setFont(u8g2_font_luRS10_tf);

  u8g2.setCursor(12, 26);
  u8g2.print("levels");
  if (menu_position == 0) {
    u8g2.setCursor(4, 26);
    u8g2.print(">");
  }

  u8g2.setCursor(12, 38);
  u8g2.print("lock");
  if (menu_position == 1) {
    u8g2.setCursor(4, 38);
    u8g2.print(">");
  }

  u8g2.setCursor(12, 50);
  u8g2.print("about");
  if (menu_position == 2) {
    u8g2.setCursor(4, 50);
    u8g2.print(">");
  }

  u8g2.setCursor(12, 62);
  u8g2.print("settings");
  if (menu_position == 3) {
    u8g2.setCursor(4, 62);
    u8g2.print(">");
  }
}

void drawAboutScreen(){
  u8g2.setFont(u8g2_font_helvR10_tf);
  u8g2.setCursor(128 - 2 - u8g2.getStrWidth("about"), 12);
  u8g2.print("about");

  u8g2.setFont(u8g2_font_7x13_tf);

  u8g2.setCursor(4, 26);
  u8g2.print("madanowi.cz/deej");
  // if (menu_position == 0) {
  //   u8g2.setCursor(4, 26);
  //   u8g2.print(">");
  // }

  u8g2.setCursor(4, 38);
  u8g2.print("Kochajmy Labno");
  // if (menu_position == 1) {
  //   u8g2.setCursor(4, 38);
  //   u8g2.print(">");
  // }

  u8g2.setCursor(4, 50);
  u8g2.print("");
  // if (menu_position == 2) {
  //   u8g2.setCursor(4, 50);
  //   u8g2.print(">");
  // }

  u8g2.setFont(u8g2_font_luRS10_tf);
  u8g2.setCursor(12, 62);
  u8g2.print("exit");
  if (menu_position == 0) {
    u8g2.setCursor(4, 62);
    u8g2.print(">");
  }
}

void drawSettingsScreen() {
  u8g2.setFont(u8g2_font_helvR10_tf);
  u8g2.setCursor(128 - 2 - u8g2.getStrWidth("settings"), 12);
  u8g2.print("settings");

  u8g2.setFont(u8g2_font_luRS10_tf);

  u8g2.setCursor(12, 26);
  if (settings_page == 1) {
    if (setting_comm_mode == 1) {
      u8g2.print("mode: midi");
    }
    else if (setting_comm_mode == 2) {
      u8g2.print("mode:");
    }
  }
  else if (settings_page == 2) {
    if (setting_feedback_mode == 1) {
      u8g2.print("feedback: on");
    }
    else if (setting_feedback_mode == 2) {
      u8g2.print("feedback: off");
    }
  }
  else if (settings_page == 3) {
    u8g2.print("");
  }

  if (menu_position == 0) {
    u8g2.setCursor(4, 26);
    u8g2.print(">");
  }


  u8g2.setCursor(12, 38);
  if (settings_page == 1) {
    if (setting_button_shift_mode == 1) {
      u8g2.print("btn mode: switch");
    }
    else if (setting_button_shift_mode == 2) {
      u8g2.print("btn mode: shift");
    }
  }
  else if (settings_page == 2) {
    u8g2.print("chg btn type");
  }
  else if (settings_page == 3) {
    u8g2.print("");
  }

  if (menu_position == 1) {
    u8g2.setCursor(4, 38);
    u8g2.print(">");
  }


  u8g2.setCursor(12, 50);
  u8g2.print("next page");
  if (menu_position == 2) {
    u8g2.setCursor(4, 50);
    u8g2.print(">");
  }

  u8g2.setCursor(12, 62);
  u8g2.print("exit");
  if (menu_position == 3) {
    u8g2.setCursor(4, 62);
    u8g2.print(">");
  }
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
  if (number == 5) {
    if (value == 0x7f) {
      button1_last_state = 1;
      drawConfirm("RVB", "ON");
    }
    else if (value == 0x00) {
      button1_last_state = 0;
      drawConfirm("RVB", "OFF");
    }
  }

  if (number == 1) {
    if (value == 0x00) {
      poti_return_1 = POTI_MIDI_SCALE_MIN;
    }
    else if (static_cast<int>(value) > POTI_MIDI_SCALE_MIN) {
      poti_return_1 = static_cast<int>(value);
    }
  }
  if (number == 2) {
    if (value == 0x00) {
      poti_return_2 = POTI_MIDI_SCALE_MIN;
    }
    else if (static_cast<int>(value) > POTI_MIDI_SCALE_MIN) {
      poti_return_2 = static_cast<int>(value);
    }
  }
  if (number == 3) {
    if (value == 0x00) {
      poti_return_3 = POTI_MIDI_SCALE_MIN;
    }
    else if (static_cast<int>(value) > POTI_MIDI_SCALE_MIN) {
      poti_return_3 = static_cast<int>(value);
    }
  }
  if (number == 4) {
    if (value == 0x00) {
      poti_return_4 = POTI_MIDI_SCALE_MIN;
    }
    else if (static_cast<int>(value) > POTI_MIDI_SCALE_MIN) {
      poti_return_4 = static_cast<int>(value);
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

