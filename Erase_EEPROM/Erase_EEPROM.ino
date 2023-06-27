#include <EEPROM.h>

void eraseEEPROM() {
  EEPROM.begin(EEPROM.length());
  for (int i = 0; i < EEPROM.length(); i++) {
    EEPROM.write(i, 0xFF);
  }
  EEPROM.commit();
  EEPROM.end();
}

void setup() {
  Serial.begin(115200);

  // Call the eraseEEPROM() function to erase the EEPROM
  eraseEEPROM();

  Serial.println("EEPROM erased.");
}

void loop() {
  // Your code here
}
