// Arduino9x_RX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (receiver)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Arduino9x_TX

#include <RHReliableDatagram.h>
#include <Arduino.h>
#include <SPI.h>
#include <RH_RF95.h>

// Arduino's Pins
// #define RFM95_CS 10
// #define RFM95_RST 9
// #define RFM95_INT 2

// #define LED_ON_PIN 7
// #define LED_OFF_PIN 6

// ESP32's Pins
#define RFM95_CS 5
#define RFM95_RST 14
#define RFM95_INT 2

#define LED_ON_PIN 13
#define LED_OFF_PIN 12

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(rf95, CLIENT_ADDRESS);

uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
uint8_t data[RH_RF95_MAX_MESSAGE_LEN];
uint8_t len;
uint8_t from;
uint8_t n;
uint8_t i;
String master_message;
String response;

void setup()
{
  pinMode(LED_ON_PIN, OUTPUT);
  pinMode(LED_OFF_PIN, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  while (!Serial)
    ;
  Serial.begin(9600);
  delay(100);

  Serial.println("Arduino LoRa RX Test!");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!manager.init())
  {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ))
  {
    Serial.println("setFrequency failed");
    while (1)
      ;
  }
  Serial.print("Set Freq to: ");
  Serial.println(RF95_FREQ);

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
}

void loop()
{
  if (rf95.available())
  {
    // Should be a message for us now
    len = sizeof(buf);
    if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
    {
      RH_RF95::printBuffer("Received: ", buf, len);
      master_message = (char *)buf;

      Serial.print("got reply from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");      
      Serial.println(master_message);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);

      if (master_message == "ON") {
        digitalWrite(LED_ON_PIN, HIGH);
        digitalWrite(LED_OFF_PIN, LOW);
        response = "MOTOR ON!";

      } else if (master_message == "OFF") {
        digitalWrite(LED_OFF_PIN, HIGH);
        digitalWrite(LED_ON_PIN, LOW);
        response = "MOTOR OFF!";

      } else
        response = "NO ACTION EXECUTED!";
      
      // Send a reply
      memset(data, 0, sizeof(data)); // clean var
      n = response.length() + 1;
      for (i = 0; i < n; ++i) {
        data[i] = response[i];
      }

      if (manager.sendtoWait(data, sizeof(data), SERVER_ADDRESS)) {
        Serial.print("Sent: ");
        Serial.println((char *)data);

      } else
        Serial.println("sendtoWait failed");
    }
    else
      Serial.println("Receive failed");
  }

  delay(750);
}