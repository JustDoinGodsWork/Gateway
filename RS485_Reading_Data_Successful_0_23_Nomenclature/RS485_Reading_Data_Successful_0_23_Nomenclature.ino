/*Below data displaying
Va : 2460
Vb : 0
Vc : 0
Vab : 2433
Vbc : 0
Vca : 2445
Ia : 9
Ib : 0
Ic : 8

pfa : 2459
pfb : 0
pfc : 0
Pa : 1
Pb : 0
Pc : 0

App Power (a) : 0
App Power (b) : 0
App Power (c) : 2
Total Active Power : 2
Total App Power : 0
kWh Lower : 183
kWh Upper : 0
kVAh Lower : 0
kVAh Upper : 0
*/

#include <ModbusRTU.h>
ModbusRTU mb;

#define debug 1
#define debug1 0

int read1 = 0;

#include <SoftwareSerial.h>
int DE_RE = 4; //D4  For MAX485 chip
int rx = 19;
int tx = 18;

SoftwareSerial S(rx, tx);//D6/D7  (RX , TX)

uint16_t Mread1[11], Mread2[11], Mread3[11];

bool cbWrite(Modbus::ResultCode event, uint16_t transactionId, void* data) {
  if (debug1)
    Serial.printf_P("Request result: 0x%02X, Mem: %d\n", event, ESP.getFreeHeap());
  return true;
}

void setup() {
  Serial.begin(115200);
  S.begin(9600, SWSERIAL_8N1);
  mb.begin(&S, DE_RE); //Assing Software serial port to Modbus instance for MAX485 chip having DI,DE,RE,RO Pin at TTL side

  Serial.println(); //Print empty line
  Serial.println(sizeof(Mread1)); //Reaing size of first array

  mb.master(); //Assing Modbus function as master
}
void loop() {

  if (debug1) {
    Serial.print("When Mread = ");
    Serial.print(read1);
  }
  if (!mb.slave()) {
    if (read1 == 0) {
      if (debug1)
        Serial.println(" Reading 0-10");
      mb.readHreg(1, 0, Mread1, 9, cbWrite);
      if (debug) {
        Serial.print("Va : ");
        Serial.println(Mread1[0]);
        Serial.print("Vb : ");
        Serial.println(Mread1[1]);
        Serial.print("Vc : ");
        Serial.println(Mread1[2]);
        Serial.print("Vab : ");
        Serial.println(Mread1[3]);
        Serial.print("Vbc : ");
        Serial.println(Mread1[4]);
        Serial.print("Vca : ");
        Serial.println(Mread1[5]);
        Serial.print("Ia : ");
        Serial.println(Mread1[6]);
        Serial.print("Ib : ");
        Serial.println(Mread1[7]);
        Serial.print("Ic : ");
        Serial.println(Mread1[8]);     
      }
      read1 = 3;
    }
    else if (read1 == 1) {
      if (debug1)
        Serial.println(" Reading 6-15");
      mb.readHreg(1, 9, Mread2, 6, cbWrite);
      if (debug) {
        Serial.print("pfa : ");
        Serial.println(Mread2[0]);
        Serial.print("pfb : ");
        Serial.println(Mread2[1]);
        Serial.print("pfc : ");
        Serial.println(Mread2[2]);
        Serial.print("Pa : ");
        Serial.println(Mread2[3]);
        Serial.print("Pb : ");
        Serial.println(Mread2[4]);
        Serial.print("Pc : ");
        Serial.println(Mread2[5]);
      }
      read1 = 4;
    }
    else if (read1 == 2)
    {
      if (debug1)
        Serial.println(" Reading 14-23");
      mb.readHreg(1, 16, Mread3, 8, cbWrite);
      if (debug) {
        Serial.print("App Power (a) : ");
        Serial.println(Mread3[0]);
        Serial.print("App Power (b) : ");
        Serial.println(Mread3[1]);
        Serial.print("App Power (c) : ");
        Serial.println(Mread3[2]);
        Serial.print("Total Active Power : ");
        Serial.println(Mread3[3]);
        Serial.print("Total App Power : ");
        Serial.println(Mread3[4]);
        Serial.print("kWh Lower : ");
        Serial.println(Mread3[5]);
        Serial.print("kWh Upper : ");
        Serial.println(Mread3[6]);
        Serial.print("kVAh Lower : ");
        Serial.println(Mread3[7]);
        Serial.print("kVAh Upper : ");
        Serial.println(Mread3[8]);

        Serial.println("\n\n####################################");
      }
      read1 = 5;
    }
  }
  else if (debug1) {
    Serial.println(" : mb is SLAVE cannot read");
  }


  if (read1 == 3) {
    read1 = 1;
    if (debug1) {
      Serial.println("Setting Read1 =1, will read 6-15");
    }
  }
  else if (read1 == 4) {
    read1 = 2;
    if (debug1) {
      Serial.println("Setting Read1 =2, will read 14-23");
    }
  }
  else if (read1 == 5) {
    read1 = 0;
    if (debug1) {
      Serial.println("Setting Read1 =0, will read 0-9");
    }
    delay(1000);
  }

  mb.task();
  delay(500);
  yield();
}
