// rf95_reliable_datagram_client.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple addressed, reliable messaging client
// with the RHReliableDatagram class, using the RH_RF95 driver to control a RF95 radio.
// It is designed to work with the other example rf95_reliable_datagram_server
// Tested with Anarduino MiniWirelessLoRa, Rocket Scream Mini Ultra Pro with the RFM95W

#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <U8x8lib.h>
#include "LowPower.h"

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

//new small OLED screen, mostly based on SSD1306
#define OLED
#define LOW_POWER_PERIOD 8

// Singleton instance of the radio driver
//RH_RF95 driver;
RH_RF95 driver(10, 3); //cs,int pinout  RFM95W

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, CLIENT_ADDRESS);

#ifdef OLED
//reset is not used
#include <U8x8lib.h>
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/A5, /* data=*/A4, /* reset=*/U8X8_PIN_NONE);
char oled_msg[20];
#endif

uint8_t message[100];
uint8_t NumPktSent = 0;
uint8_t NumPktRcv = 0;
bool Ok = false;

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE TIME IN MINUTES BETWEEN 2 READING & TRANSMISSION
unsigned int idlePeriodInMin = 1;

//unsigned int nCycle = idlePeriodInMin*60/LOW_POWER_PERIOD;
unsigned int nCycle = 0;

void setup()
{
  // Rocket Scream Mini Ultra Pro with the RFM95W only:
  // Ensure serial flash is not interfering with radio communication on SPI bus
  //  pinMode(4, OUTPUT);
  //  digitalWrite(4, HIGH);

  Serial.begin(9600);
  while (!Serial)
    ; // Wait for serial port to be available
  if (!manager.init())
    Serial.println("init failed");
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  driver.setFrequency(868.0);
  driver.setTxPower(23, false); // Also tried with 20
  driver.setModemConfig(RH_RF95::Bw125Cr45Sf128);
  // If you are using Modtronix inAir4 or inAir9,or any other module which uses the
  // transmitter RFO pins and not the PA_BOOST pins
  // then you can configure the power transmitter power for -1 to 14 dBm and with useRFO true.
  // Failure to do that will result in extremely low transmit powers.
  //  driver.setTxPower(14, true);
  // You can optionally require this module to wait until Channel Activity
  // Detection shows no activity on the channel before transmitting by setting
  // the CAD timeout to non-zero:
  //  driver.setCADTimeout(10000);
#ifdef OLED
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.drawString(0, 1, "Test Range");
#endif
}

uint8_t data[] = "Hello World!";
// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void loop()
{
#ifdef OLED
  u8x8.drawString(0, 2, "Sending Ping....");
  u8x8.clearLine(3);
  u8x8.clearLine(4);
  u8x8.clearLine(5);
  u8x8.clearLine(6);
  u8x8.clearLine(7);
#endif
  Serial.println("Sending to rf95_reliable_datagram_server");

  // Send a message to manager_server
  if (manager.sendtoWait(data, sizeof(data), SERVER_ADDRESS))
  {
    // Now wait for a reply from the server
    NumPktSent++;
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
    {
      Serial.print("got reply from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char *)buf);
      NumPktRcv++;
#ifdef OLED
      u8x8.clearLine(2);
      u8x8.drawString(0, 3, "ACK from GW OK");
      sprintf((char *)message, "Sent=%d,Rcv=%d", NumPktSent, NumPktRcv);
      u8x8.drawString(0, 4, (char *)message);
      Ok = true;
#endif

    }
    else
    {
      Serial.println("No reply, is rf95_reliable_datagram_server running?");
#ifdef OLED
      u8x8.clearLine(2);
      u8x8.drawString(0, 3, "No reply ");
#endif
      Ok =false;
    }
  }
  else
  {
    Serial.println("sendtoWait failed");
#ifdef OLED
    u8x8.clearLine(2);
    u8x8.drawString(0, 3, "SendtoWait NOK");
#endif
    Ok = false;
  }
  if (Ok == true)
  {
    
    Ok = false;
    uint16_t nCycle = idlePeriodInMin * 60 / LOW_POWER_PERIOD + random(2, 4);

    for (int i = 0; i < nCycle; i++)
    {

#ifdef OLED
      sprintf((char *)message, "Power mode %d", i);
      u8x8.drawString(0, 5, (char *)message);
#endif // ATmega328P, ATmega168, ATmega32U4
      LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);

    }
  }
  delay(1000);
}
