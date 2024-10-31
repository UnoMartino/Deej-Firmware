#include <Arduino.h>
#include <MIDI.h>
#include <U8g2lib.h>
#include <ArduinoJson.h>
#include <Wire.h>

/*
Pin assignments
*/

#define POTI_1 27
#define POTI_2 14
#define POTI_3 12
#define POTI_4 13

#define OLED_SCL 22
#define OLED_SDA 21

#define BUTTON_1 26
#define BUTTON_2 25
#define BUTTON_3 33
#define BUTTON_4 32
#define BUTTON_5 35
#define BUTTON_6 34
#define BUTTON_7 15
#define BUTTON_8 2
#define BUTTON_9 4
#define BUTTON_10 16

#define POTI_MIDI_SCALE_MIN 64
#define POTI_MIDI_SCALE_MAX 127

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, OLED_SCL, OLED_SDA);

const uint8_t midi_channel = 1;
const uint8_t controller_number = 0;

static int averageValue_1 = 0;
static int averageValue_2 = 0;
static int averageValue_3 = 0;
static int averageValue_4 = 0;
static int hysteresis_1 = 0;
static int hysteresis_2 = 0;
static int hysteresis_3 = 0;
static int hysteresis_4 = 0;

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

short comm_mode = 1; // 1 - MIDI, 2 - 

bool levels_screen_switch = 1;
bool menu_screen_switch = 0;
bool settings_screen_switch = 0;

int menu_position = 0;

void drawLevelsScreen();
void drawMenuScreen();
void drawSettingsScreen();

MIDI_CREATE_DEFAULT_INSTANCE();

void setup() {
  // put your setup code here, to run once:
  MIDI.begin(1);
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

  u8g2.begin();

}

void loop() {

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

  bool button_1 = !digitalRead(BUTTON_1);
  if (button_1) {if (button1_state == 0) {button1_state = 1; MIDI.sendControlChange(5, 127, 1); MIDI.sendNoteOn(1, 127, 1); delay(100); MIDI.sendNoteOff(1, 127, 1);}}
  else {button1_state = 0;}

  bool button_2 = !digitalRead(BUTTON_2);
  if (button_2) {if (button2_state == 0) {button2_state = 1; MIDI.sendControlChange(6, 127, 1);}}
  else {button2_state = 0;}

  bool button_3 = !digitalRead(BUTTON_3);
  if (button_3) {if (button3_state == 0) {button3_state = 1; MIDI.sendControlChange(7, 127, 1);}}
  else {button3_state = 0;}

  bool button_4 = !digitalRead(BUTTON_4);
  if (button_4) {if (button4_state == 0) {button4_state = 1; MIDI.sendControlChange(8, 127, 1);}}
  else {button4_state = 0;}

  bool button_5 = !digitalRead(BUTTON_5);
  if (button_5) {if (button5_state == 0) {button5_state = 1; MIDI.sendControlChange(9, 127, 1);}}
  else {button5_state = 0;}

  bool button_6 = !digitalRead(BUTTON_6);
  if (button_6) {if (button6_state == 0) {button6_state = 1; MIDI.sendControlChange(10, 127, 1);}}
  else {button6_state = 0;}

  bool button_7 = !digitalRead(BUTTON_7);
  if (button_7) {if (button7_state == 0) {button7_state = 1; MIDI.sendControlChange(11, 127, 1);}}
  else {button7_state = 0;}

  bool button_8 = !digitalRead(BUTTON_8);
  if (button_8) {if (button8_state == 0) {button8_state = 1; MIDI.sendControlChange(12, 127, 1);}}
  else {button8_state = 0;}

  bool button_9 = !digitalRead(BUTTON_9); // MENU BUTTON
  if (button_9) {if (button9_state == 0) {
    button9_state = 1; 
  
    if (levels_screen_switch) { // ON LEVELS SCREEN GO TO MENU SCREEN
      levels_screen_switch = 0;
      menu_screen_switch = 1;
      settings_screen_switch = 0;
    } 
    else if (menu_screen_switch) { // ON MENU SCREEN ADVANCE MENU POSITION
      if (menu_position < 3) {
        menu_position++;
      } 
      else {
        menu_position = 0;
      }
    } 
    else if (settings_screen_switch) { // ON SETTINGS SCREEN ADVANCE MENU POSITION
      if (menu_position < 3) {
        menu_position++;
      } 
      else {
        menu_position = 0;
      }
    }
  }
  else {button9_state = 0;}}

  bool button_10 = !digitalRead(BUTTON_10); // OK BUTTON
  if (button_10) {if (button10_state == 0) {
    button10_state = 1; 

    if (menu_screen_switch) { // ON MENU SCREEN
      if (menu_position == 0) {
        levels_screen_switch = 1;
        menu_screen_switch = 0;
        settings_screen_switch = 0;
        menu_position = 0;
      }
      if (menu_position == 1) {
        levels_screen_switch = 0;
        menu_screen_switch = 0;
        settings_screen_switch = 0;
        menu_position = 0;
      }
      if (menu_position == 2) {
        levels_screen_switch = 0;
        menu_screen_switch = 0;
        settings_screen_switch = 0;
        menu_position = 0;
      }
      if (menu_position == 3) {
        levels_screen_switch = 0;
        menu_screen_switch = 0;
        settings_screen_switch = 1;
        menu_position = 0;
      }
    }

    if (settings_screen_switch) { // ON SETTINGS SCREEN
      if (menu_position == 0) {
        menu_position = 0;
        if (comm_mode == 1) {
          comm_mode = 2;
        }
        else {
          comm_mode = 1;
        }
      }
      if (menu_position == 1) {
        menu_position = 0;
      }
      if (menu_position == 2) {
        menu_position = 0;
      }
      if (menu_position == 3) {
        levels_screen_switch = 0;
        menu_screen_switch = 1;
        settings_screen_switch = 0;
        menu_position = 0;
      }
    }
  
  
  
  }}
  else {button10_state = 0;}


  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_helvR10_tf);

  u8g2.setCursor(4, 12);
  u8g2.print("deej");

  if (levels_screen_switch) {
    drawLevelsScreen();
  }

  if (menu_screen_switch) {
    drawMenuScreen();
  }

  if (settings_screen_switch) {
    drawSettingsScreen();
  }  

  u8g2.sendBuffer();
  delay(50);

}

void drawLevelsScreen() {
  u8g2.setFont(u8g2_font_helvR10_tf);
  u8g2.setCursor(128 - 2 - u8g2.getStrWidth("levels"), 12);
  u8g2.print("levels");

  u8g2.setFont(u8g2_font_luRS10_tf);
  u8g2.setCursor(4, 26);
  u8g2.print("L1");
  u8g2.drawFrame(21, 16, 105, 10);
  if (hysteresis_1 > POTI_MIDI_SCALE_MIN) {
    u8g2.drawBox(23, 18, map(hysteresis_1, POTI_MIDI_SCALE_MIN, POTI_MIDI_SCALE_MAX, 0, 101), 6);
  }
  else {
    u8g2.setCursor(23, 24);
    u8g2.setFont(u8g2_font_spleen5x8_mf);
    u8g2.print("MUTE");
  }
  
  u8g2.setFont(u8g2_font_luRS10_tf);
  u8g2.setCursor(4, 38);
  u8g2.print("L2");
  u8g2.drawFrame(21, 28, 105, 10);
  if (hysteresis_2 > POTI_MIDI_SCALE_MIN) {
    u8g2.drawBox(23, 30, map(hysteresis_2, POTI_MIDI_SCALE_MIN, POTI_MIDI_SCALE_MAX, 0, 101), 6);
  }
  else {
    u8g2.setCursor(23, 36);
    u8g2.setFont(u8g2_font_spleen5x8_mf);
    u8g2.print("MUTE");
  }
  
  u8g2.setFont(u8g2_font_luRS10_tf);
  u8g2.setCursor(4, 50);
  u8g2.print("L3");
  u8g2.drawFrame(21, 40, 105, 10);
  if (hysteresis_3 > POTI_MIDI_SCALE_MIN) {
    u8g2.drawBox(23, 42, map(hysteresis_3, POTI_MIDI_SCALE_MIN, POTI_MIDI_SCALE_MAX, 0, 101), 6);
  }
  else {
    u8g2.setCursor(23, 48);
    u8g2.setFont(u8g2_font_spleen5x8_mf);
    u8g2.print("MUTE");
  }

  u8g2.setFont(u8g2_font_luRS10_tf);
  u8g2.setCursor(4, 62);
  u8g2.print("L4");
  u8g2.drawFrame(21, 52, 105, 10);
  if (hysteresis_4 > POTI_MIDI_SCALE_MIN) {
    u8g2.drawBox(23, 54, map(hysteresis_4, POTI_MIDI_SCALE_MIN, POTI_MIDI_SCALE_MAX, 0, 101), 6);
  }
  else {
    u8g2.setCursor(23, 60);
    u8g2.setFont(u8g2_font_spleen5x8_mf);
    u8g2.print("MUTE");
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
  u8g2.print("m2");
  if (menu_position == 1) {
    u8g2.setCursor(4, 38);
    u8g2.print(">");
  }

  u8g2.setCursor(12, 50);
  u8g2.print("m3");
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

void drawSettingsScreen() {
  u8g2.setFont(u8g2_font_helvR10_tf);
  u8g2.setCursor(128 - 2 - u8g2.getStrWidth("settings"), 12);
  u8g2.print("settings");

  u8g2.setFont(u8g2_font_luRS10_tf);

  u8g2.setCursor(12, 26);
  if (comm_mode == 1) {
    u8g2.print("mode: midi");
  }
  else if (comm_mode == 2) {
    u8g2.print("mode:");
  }
  
  if (menu_position == 0) {
    u8g2.setCursor(4, 26);
    u8g2.print(">");
  }

  u8g2.setCursor(12, 38);
  u8g2.print("s2");
  if (menu_position == 1) {
    u8g2.setCursor(4, 38);
    u8g2.print(">");
  }

  u8g2.setCursor(12, 50);
  u8g2.print("s3");
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

// Fonts
  // u8g2_font_helvR12_tf - normal
  // u8g2_font_helvB12_tf - bold
  // u8g2_font_helvR10_tf - normal
  // u8g2_font_helvB10_tf - bold
  // u8g2_font_luRS10_tf - 10px
  // u8g2_font_spleen5x8_mf - 6px


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

