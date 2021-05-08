
/*
 * DD0VS 14th March 2020
 * DD0VS 08th May 2021
 * 
 * https://github.com/adafruit/Adafruit_INA219/blob/master/examples/getcurrent/getcurrent.ino
 * 
 * https://www.arduino.cc/en/Tutorial/DhcpAddressPrinter
 * 
 * https://pubsubclient.knolleary.net/api.html
 * 
 * https://www.heise.de/developer/artikel/Kommunikation-ueber-MQTT-3238975.html
 * 
 */

/* for INA 2019 */ 
#include <Wire.h>
#include <Adafruit_INA219.h>

/* for Ethernet Shield */
#include <SPI.h>
#include <Ethernet.h> 

/* for MQTT */
#include <PubSubClient.h> // MQTT Bibliothek


Adafruit_INA219 ina219_A;
bool bIna219A = false;

float shuntvoltage = 0;
float busvoltage = 0;
float current_mA = 0;
float loadvoltage = 0;
float power_mW = 0;
char sCurrent_mA [10];


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network.
// gateway and subnet are optional:
byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};

EthernetClient ethClient;
PubSubClient client;

void reconnect() {
  // Solange wiederholen bis Verbindung wiederhergestellt ist
  while (!client.connected()) {
    Serial.print(F("Versuch des MQTT Verbindungsaufbaus..."));
    //Versuch die Verbindung aufzunehmen
    if (client.connect("SDR-12v-supply")) {
      Serial.println("Erfolgreich verbunden!");
      // Nun versendet der Arduino eine Nachricht in outTopic ...
      client.publish("DD0VS/SDR-12V","connected");
      // und meldet sich für bei inTopic für eingehende Nachrichten an
      //client.subscribe("inTopic");
    } else {
      Serial.print("Fehler, rc=");
      Serial.print(client.state());
      Serial.println(F(" Nächster Versuch in 5 Sekunden"));
      // 5 Sekunden Pause vor dem nächsten Versuch
      myDelay(5000);
    }
  }
}

void setup(void) 
{
  Serial.begin(115200);
  while (!Serial) {
      // will pause Zero, Leonardo, etc until serial console opens
      myDelay(1);
  }

  uint32_t currentFrequency;
    
  Serial.println(F("Hello!"));
  
  // Initialize the INA219.
  // By default the initialization will use the largest range (32V, 2A).  However
  // you can call a setCalibration function to change this range (see comments).
  bIna219A = ina219_A.begin();
  if (!bIna219A)
    Serial.println(F("INA219_A not exists"));
  // To use a slightly lower 32V, 1A range (higher precision on amps):
  //ina219.setCalibration_32V_1A();
  // Or to use a lower 16V, 400mA range (higher precision on volts and amps):
  //ina219.setCalibration_16V_400mA();

  Serial.println("Measuring voltage and current with INA219 ... and writing it back via MQTT");

  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println(F("Ethernet shield was not found.  Sorry, can't run without hardware. :("));
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // no point in carrying on, so do nothing forevermore:
    while (true) {
      myDelay(1);
    }
  }
  // print your local IP address:
  Serial.print(F("My IP address: "));
  Serial.println(Ethernet.localIP());

  client.setClient(ethClient);
  client.setServer("shf-sdr",1883);

}

// myDelay
void myDelay(int iMyDel) {
  unsigned int uiMyDelMillis;

  uiMyDelMillis = millis();
  while ( (millis() - uiMyDelMillis) < iMyDel)
  {}

}


void loop(void) 
{

  shuntvoltage = ina219_A.getShuntVoltage_mV();
  busvoltage   = ina219_A.getBusVoltage_V();
  current_mA   = ina219_A.getCurrent_mA();
  power_mW     = ina219_A.getPower_mW();
  loadvoltage  = busvoltage + (shuntvoltage / 1000);
  
  Serial.print(F("Bus Voltage:   ")); Serial.print(busvoltage); Serial.println(F(" V"));
  Serial.print(F("Shunt Voltage: ")); Serial.print(shuntvoltage); Serial.println(F(" mV"));
  Serial.print(F("Load Voltage:  ")); Serial.print(loadvoltage); Serial.println(F(" V"));
  Serial.print(F("Current:       ")); Serial.print(current_mA); Serial.println(F(" mA"));
  Serial.print(F("Power:         ")); Serial.print(power_mW); Serial.println(F(" mW"));
  Serial.println("");

  if (!client.connected()) {
      reconnect();
  }
  client.loop();
  dtostrf(current_mA,7,1,sCurrent_mA);
  client.publish("DD0VS/SDR-12V/amps",sCurrent_mA);

  switch (Ethernet.maintain()) {
    case 1:
      //renewed fail
      Serial.println("Error: renewed fail");
      break;

    case 2:
      //renewed success
      Serial.println("Renewed success");
      //print your local IP address:
      Serial.print("My IP address: ");
      Serial.println(Ethernet.localIP());
      break;

    case 3:
      //rebind fail
      Serial.println("Error: rebind fail");
      break;

    case 4:
      //rebind success
      Serial.println("Rebind success");
      //print your local IP address:
      Serial.print("My IP address: ");
      Serial.println(Ethernet.localIP());
      break;

    default:
      //nothing happened
      break;
  }
  myDelay(2000);
}
