/* alarm_api.cpp
  This implements the "abstract" alarm api.

  Copyright (C) 2022 Robert Read

  This program includes free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  See the GNU Affero General Public License for more details.
  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

*/
#include "alarm_api.h"
#include "gpad_utility.h"
#include <Arduino.h>

// here is the abstract "state" of the machine,
// completely independent of hardware.
// This is very simple version of what is probably needed.
// For example, perhaps the abstract machine should know how long
// it has been in a given alarm state. At present, this
// is a "dumb" machine---the user of it is expected to do
// all alarm management.
AlarmLevel currentLevel = silent;
bool currentlyMuted = false;
char AlarmMessageBuffer[MAX_BUFFER_SIZE];
unsigned long muteTimeoutEndMillis = 0;
unsigned long muteTimeoutStartMillis = 0;
unsigned long muteTimeoutDurationMillis = 0;
const char *AlarmNames[] = {"OK   ", "INFO.", "PROB.", "WARN ", "CRIT.", "PANIC"};
char currentAlarmId[11] = "";
char currentAlarmType[4] = "";
// This is the abstract alarm function. It CANNOT
// assume the msg buffer will exist after this call.
// str must be null-terminated string!
// It returns the PREVIOUS ALARM LEVEL
int alarm_event(AlarmEvent &event, Stream *serialport)
{
  return alarm((AlarmLevel)event.lvl, event.msg, serialport);
}
int alarm(AlarmLevel level, char *str, Stream *serialport)
{
  if (!(level >= 0 && level < NUM_LEVELS))
  {
    if (serialport != nullptr)
    {
      serialport->println(F("Bad Level!"));
      printError(serialport);
    }
    return -1;
  }
  int previousLevel = currentLevel;
  currentLevel = level;
  // This makes sure we erase the buffer even if msg is an empty string
  AlarmMessageBuffer[0] = '\0';
  if (str != nullptr)
  {
    strncpy(AlarmMessageBuffer, str, MAX_BUFFER_SIZE - 1);
    AlarmMessageBuffer[MAX_BUFFER_SIZE - 1] = '\0';
  }
  return previousLevel;
}

void setMuted(bool muted)
{
  currentlyMuted = muted;
}

bool isMuted()
{
  return currentlyMuted;
}

void toggleMuted()
{
  currentlyMuted = !currentlyMuted;
}

void setMuteTimeoutMinutes(unsigned long minutes)
{
  setMuted(true);
  muteTimeoutStartMillis = millis();
  muteTimeoutDurationMillis = minutes * 60000UL;
  muteTimeoutEndMillis = muteTimeoutStartMillis + muteTimeoutDurationMillis;
}

void clearMuteTimeout()
{
  muteTimeoutEndMillis = 0;
  muteTimeoutStartMillis = 0;
  muteTimeoutDurationMillis = 0;
}

bool serviceMuteTimeout()
{
  if (muteTimeoutEndMillis == 0)
  {
    return false;
  }

  if ((millis() - muteTimeoutStartMillis) >= muteTimeoutDurationMillis)
  {
    clearMuteTimeout();
    setMuted(false);
    return true;
  }

  return false;
}

AlarmLevel getCurrentAlarmLevel()
{
  return currentLevel;
}

char *getCurrentMessage()
{
  return AlarmMessageBuffer;
}

// TODO: Add "setup" routine here to initialize the buffer.
