#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Time.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include "DHT.h"
#define DHTPIN 5     // what digital pin we're connected to
// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)#include <OneWire.h>
#include <DallasTemperature.h>

#include <avr/sleep.h>
#include <avr/power.h>

#define PININTERRUP 2
#define LED_GREEN 10
#define LED_YELLOW 9
#define PIN_SD_POWER 6

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 3
//#define TEMPERATURE_PRECISION 9 // Lower resolution

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

int numberOfDevices; // Number of temperature devices found

DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address

DeviceAddress therm[5]; // адреса датчиков
float k[5]; // смещения для коректировки

byte coutDevice; //количество обнаруженных датчиков
const int chipSelect = 4;
DHT dht(DHTPIN, DHTTYPE);


String fileName;
File dataFile;
byte hour_file;

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
tmElements_t tm;


int pin = 2;
int aa;

void wakeUp()
{
  Serial.println("WakeUp"); //Проснулись
  detachInterrupt(0); //Отключаем прерывания
  // while(1); //Бесконечный цикл
}

void EnterSleep()
{

  attachInterrupt(0, wakeUp, FALLING ); //Если на 0-вом прерываниии - ноль, то просыпаемся.
  delay(100);

  sleep_enable(); //Разрешаем спящий режим
  sleep_mode(); //Спим (Прерывания продолжают работать.) Программа останавливается.
  sleep_disable(); //Запрещаем спящий режим
}

void setup()
{
  Serial.begin(9600);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); //Определяем режим сна
  pinMode(pin, INPUT);
  aa = 0;

  // Start up the library
  sensors.begin();
  coutDevice = (byte)sensors.getDeviceCount();

  // locate devices on the bus
  Serial.print(F("Locating devices..."));
  Serial.print(F("Found "));
  Serial.print(coutDevice, DEC);
  Serial.println(F(" devices."));

  for (byte i = 0; i < coutDevice; i++) {
    sensors.getAddress(therm[i], i);
    k[i] =  (float)sensors.getUserDataByIndex(i) / 100.0;

  }

  for (byte i = 0; i < coutDevice; i++) {
    Serial.print(F("Device "));
    Serial.print( i); Serial.print(F( " Address: "));
    Serial.print( printAddress(therm[i]));
    Serial.print( "   offset " );
    Serial.print( k[i] );
    Serial.println();
  }


  for (byte i = 0; i < coutDevice; i++) {
    sensors.setResolution(therm[i], 12 );
  }

  dht.begin();

  Serial.print(F("\nInitializing SD card..."));
  // see if the card is present and can be initialized:
  digitalWrite(PIN_SD_POWER, LOW);
  if (!SD.begin(chipSelect)) {
    Serial.println(F("Card failed, or not present"));

    digitalWrite(LED_YELLOW, LOW);
    delay(200);
    digitalWrite(LED_YELLOW, HIGH);
    delay(200);
    digitalWrite(LED_YELLOW, LOW);
    delay(200);
    digitalWrite(LED_YELLOW, HIGH);
    delay(200);

    // don't do anything more:
    return;
  }
  Serial.println(F("card initialized."));
  fileName = getFileName();
  hour_file = getHour();

}

void loop()
{
  Serial.println("Hello World");
  aa++;
  //delay(5000);
  Serial.println(aa);

  work();
  Serial.println("Sleep");

  EnterSleep(); //Пора спать
}


void work() {

  digitalWrite(LED_GREEN, LOW);
  //  if (stringComplete) {
  //    Serial.println(inputString);
  //    parsStr(inputString);
  //    // clear the string:
  //    inputString = "";
  //    stringComplete = false;
  //  }


  sensors.requestTemperatures();


  //  delay(5000);
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h  = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t  = dht.readTemperature();

  for (byte i = 0; i < coutDevice; i++) {
    float tempC = sensors.getTempC(therm[i]) ;
    Serial.print(getDateTime());
    Serial.print(F(";\""));
    Serial.print( printAddress(therm[i]));
    Serial.print(F("\";"));
    Serial.print( tempC + k[i]);
    //  Serial.print( tempC);
    //    Serial.print(F(";"));
    //    Serial.print(tempC);
    //    Serial.print(F(";"));
    //    Serial.println(k[i]);
    Serial.println();
  }
  Serial.print(getDateTime());
  Serial.print(F(";\""));
  Serial.print( F("DHT_Temper"));
  Serial.print(F("\";"));
  Serial.println(t);

  Serial.print(getDateTime());
  Serial.print(F(";\""));
  Serial.print( F("DHT_Humid"));
  Serial.print(F("\";"));
  Serial.println(h);

  Serial.println();
  if (hour_file != getHour()) {
    fileName = getFileName();
    hour_file = getHour();
  }
  digitalWrite(PIN_SD_POWER, LOW);
  dataFile = SD.open(fileName, FILE_WRITE);
  if (dataFile) {


    for (byte i = 0; i < coutDevice; i++) {
      float tempC = sensors.getTempC(therm[i]) + k[i];
      dataFile.println(F("<srt>"));
      dataFile.print(F("<date>"));
      dataFile.print(getDateTime());
      dataFile.println(F("</date>"));
      dataFile.print(F("<id>"));
      dataFile.print( printAddress(therm[i]));
      dataFile.println(F("</id>"));
      dataFile.print(F("<val>"));
      dataFile.print(tempC);
      dataFile.println(F("</val>"));
      dataFile.println(F("<tip>18B20</tip>"));
      dataFile.println(F("</srt>"));
    }
    dataFile.println(F("<srt>"));
    dataFile.print(F("<date>"));
    dataFile.print(getDateTime());
    dataFile.println(F("</date>"));
    dataFile.println(F("<id>DHT_Temper</id>"));
    dataFile.print(F("<val>"));
    dataFile.print(t);
    dataFile.println(F("</val>"));
    dataFile.println(F("<tip>DHT22</tip>"));
    dataFile.println(F("</srt>"));

    dataFile.println(F("<srt>"));
    dataFile.print(F("<date>"));
    dataFile.print(getDateTime());
    dataFile.println(F("</date>"));
    dataFile.println(F("<id>DHT_Humid</id>"));
    dataFile.print(F("<val>"));
    dataFile.print(h);
    dataFile.println(F("</val>"));
    dataFile.println(F("<tip>DHT22</tip>"));
    dataFile.println(F("</srt>"));



    dataFile.close();
    //    // print to the serial port too:
    Serial.println(F("Write"));
  }
  digitalWrite(PIN_SD_POWER, HIGH);
  digitalWrite(LED_GREEN, HIGH);
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

byte getHour() {
  tmElements_t tm;
  if (RTC.read(tm)) {
    return tm.Hour;
  } else {
    if (RTC.chipPresent()) {
      return -1;// F("The DS1307 is stopped.  Please run the SetTime");
    } else {
      return -2;//F("DS1307 read error!  Please check the circuitry.");
    }
  }
}

String getDateTime() {
  String dateTime = "";
  tmElements_t tm;

  if (RTC.read(tm)) {
    dateTime  += tmYearToCalendar(tm.Year);
    dateTime += F(".");
    dateTime += str2dig(tm.Month);
    dateTime += F(".");
    dateTime += str2dig(tm.Day);
    dateTime += F(" ");
    dateTime += str2dig(tm.Hour);
    dateTime += F(":");
    dateTime += str2dig(tm.Minute);
    dateTime += F(":");
    dateTime += str2dig(tm.Second);
    return dateTime;
  } else {
    if (RTC.chipPresent()) {
      return F("The DS1307 is stopped.  Please run the SetTime");
    } else {
      return F("DS1307 read error!  Please check the circuitry.");
    }
  }
}

String getFileName() {
  String dateTime = "";
  tmElements_t tm;

  if (RTC.read(tm)) {
    //  dateTime  += tmYearToCalendar(tm.Year);
    dateTime += str2dig(tm.Month);
    dateTime += str2dig(tm.Day);
    dateTime += str2dig(tm.Hour);
    dateTime += str2dig(tm.Minute);
    //   dateTime += str2dig(tm.Second);
    dateTime += F(".xml");
    return dateTime;
  } else {
    if (RTC.chipPresent()) {
      return F("The DS1307 is stopped.  Please run the SetTime");
    } else {
      return F("DS1307 read error!  Please check the circuitry.");
    }
  }
}

String str2dig(int number) {
  String str = "";
  if (number >= 0 && number < 10) {
    str += F("0");
  }
  str += number;
  return str;
}
bool getTime(const char *str)
{
  int Hour, Min, Sec;

  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  tm.Hour = Hour;
  tm.Minute = Min;
  tm.Second = Sec;
  return true;
}

bool getDate(const char *str)
{
  //  char Month[12];
  int Day, Year, Month;
  //  uint8_t monthIndex;

  if (sscanf(str, "%d-%d-%d", &Year, &Month, &Day) != 3) return false;

  tm.Day = Day;
  tm.Month = Month;
  tm.Year = CalendarYrToTm(Year);
  return true;
}
void sortAdress(DeviceAddress adress[], int count)
{




}


