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
      mb.readHreg(1, 0, Mread1, 10, cbWrite);
      if (debug) {
        Serial.print("Mread[0] :");
        Serial.println(Mread1[0]);
        Serial.print("Mread[1] :");
        Serial.println(Mread1[1]);
        Serial.print("Mread[2] :");
        Serial.println(Mread1[2]);
        Serial.print("Mread[3] :");
        Serial.println(Mread1[3]);
        Serial.print("Mread[4] :");
        Serial.println(Mread1[4]);
        Serial.print("Mread[5] :");
        Serial.println(Mread1[5]);
        Serial.print("Mread[6] :");
        Serial.println(Mread1[6]);
        Serial.print("Mread[7] :");
        Serial.println(Mread1[7]);
        Serial.print("Mread[8] :");
        Serial.println(Mread1[8]);
        Serial.print("Mread[9] :");
        Serial.println(Mread1[9]);
      }

      read1 = 3;
    }
    else if (read1 == 1) {
      if (debug1)
        Serial.println(" Reading 6-15");
      mb.readHreg(1, 10, Mread2, 6, cbWrite);
      if (debug) {
        Serial.print("Mread[10] :");
        Serial.println(Mread2[0]);
        Serial.print("Mread[11] :");
        Serial.println(Mread2[1]);
        Serial.print("Mread[12] :");
        Serial.println(Mread2[2]);
        Serial.print("Mread[13] :");
        Serial.println(Mread2[3]);
        Serial.print("Mread[14] :");
        Serial.println(Mread2[4]);
        Serial.print("Mread[15] :");
        Serial.println(Mread2[5]);
        //        Serial.print("Mread[12] :");
        //        Serial.println(Mread2[6]);
        //        Serial.print("Mread[13] :");
        //        Serial.println(Mread2[7]);
        //        Serial.print("Mread[14] :");
        //        Serial.println(Mread2[8]);
        //        Serial.print("Mread[15] :");
        //        Serial.println(Mread2[9]);
      }
      read1 = 4;
    }
    else if (read1 == 2)
    {
      if (debug1)
        Serial.println(" Reading 14-23");
      mb.readHreg(1, 16, Mread3, 8, cbWrite);
      if (debug) {
        Serial.print("Mread[16] :");
        Serial.println(Mread3[0]);
        Serial.print("Mread[17] : ");
        Serial.println(Mread3[1]);
        Serial.print("Mread[18] :");
        Serial.println(Mread3[2]);
        Serial.print("Mread[19] :");
        Serial.println(Mread3[3]);
        Serial.print("Mread[20] :");
        Serial.println(Mread3[4]);
        Serial.print("Mread[21] :");
        Serial.println(Mread3[5]);
        Serial.print("Mread[22] :");
        Serial.println(Mread3[6]);
        Serial.print("Mread[23] :");
        Serial.println(Mread3[7]);

        Serial.println("\n\n####################################\n\n");
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
    //delay(5000);
  }

  mb.task();
  delay(1000);
  yield();
}
