#ifndef GPAD_MENU_H
#define GPAD_MENU_H

void setup_GPAD_menu();

void poll_GPAD_menu();

void navigate_to_n_and_execute(int n);
void open_settings_menu_at(int n);

void registerRotationEvent(bool CW);
void registerRotaryEncoderPress();

void reset_menu_navigation();
void returnToMainPage();

void executeAlarmAction(uint8_t actionIndex);

#endif
