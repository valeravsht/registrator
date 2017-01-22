//
// This sketch does not use the ALARM registers and uses those 2 bytes as a counter
// these 2 bytes can be used for other purposes as well e.g. last temperature or
// a specific ID.
//

#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 3

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

int count = 0;
int calib = -19;
float temper;
float k;

DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address

void setup(void)
{
  // start serial port
  Serial.begin(9600);
  Serial.println("Dallas Temperature IC Control Library Demo");

  // Start up the library
  sensors.begin();

  // sensors.setUserDataByIndex(0, calib);
  for (byte i = 0; i < 5; i++) {
    sensors.getAddress(tempDeviceAddress, i);
    String adr = printAddress(tempDeviceAddress);

    if ( adr == "288e627c0600001e") {
      calib = 10;
    }
    if ( adr == "28ffb011911503c2") {
      calib = 40;
    }
    if ( adr == "28ffb8da501603fc") {
      calib = 30;
    }
    if ( adr == "28ff11d2a41501da") {
      calib = 40;
    }
    if ( adr == "28ff5da250160340") {
      calib = 10;
    }

    sensors.setResolution(tempDeviceAddress, 12 );
    sensors.setUserData(tempDeviceAddress, calib);
    // запись в ЕПРОМ 18b20
    oneWire.reset();
    oneWire.select(tempDeviceAddress);
    oneWire.write(0x48);
    delay(200);


    int x = sensors.getUserData(tempDeviceAddress);

    k = (float)x / 100.0;
    Serial.print("temperatures offset ");
    Serial.println(x);

  }
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");

  Serial.print("Temperature raw ");
  temper = sensors.getTempCByIndex(0);
  Serial.println(temper);
  Serial.print("Temperature calib");

  Serial.println(temper + k);

}

void loop(void)
{
  //  // call sensors.requestTemperatures() to issue a global temperature
  //  // request to all devices on the bus
  //  Serial.print("Requesting temperatures...");
  //  sensors.requestTemperatures(); // Send the command to get temperatures
  //  Serial.println("DONE");
  //
  //  Serial.print("Temperature raw ");
  //  temper =sensors.getTempCByIndex(0);
  //  Serial.println(temper);
  //   Serial.print("Temperature calib");
  //
  //  Serial.println(temper + k);
  //  count++;
  //
  //
  //  Serial.println(count);
}


String printAddress(DeviceAddress deviceAddress)
{
  String ret = "";
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) ret = ret + "0";
    ret = ret + String(deviceAddress[i], HEX);
  }
  return ret;
}

