//Libraries:
#include <Arduino.h>
#include "ArduinoJson.h"
#include "Adafruit_I2CDevice.h"
#include "Adafruit_BusIO_Register.h"
#include "SPI.h"
#include "SdFat.h"
#include "Adafruit_ADS1015.h"
#include "SparkFun_AK975X_Arduino_Library.h"
#include "Adafruit_SI1145.h"
#include "Adafruit_TSL2591.h"
#include "SparkFun_SCD30_Arduino_Library.h"
//#include "neopixel.h"
#include "FastLED.h"
#include "Adafruit_TSL2591.h"
#include "Adafruit_BME280.h"
#include "Adafruit_BME680.h"
#include <Adafruit_AHTX0.h>
#include "ADAFRUIT_SI7021.h"
#include "Adafruit_SGP30.h"
#include "Adafruit_PM25AQI.h"
#include "SPS30.h"
#include "Adafruit_SGP40.h"
#include "Time.h"
#include "TimeLib.h"
#include "math.h"
#include "Adafruit_EPD.h"
#include "Adafruit_GFX.h"
//#include "FreeSans9pt7b.h"
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiUDP.h>
#include "Adafruit_SHT4x.h"
#include "cred.h"
#include <EEPROM.h>
#include <SensirionI2CScd4x.h>
#include <ESP32Ping.h>
#include <HTTPClient.h>

WiFiClient espClient;
WiFiClientSecure sec_espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

//SD card
SdFat SD;
#define SD_CS_PIN SS
File myFile;
bool sdwrite = 0; ///enable writing to SDcard


bool debug = true;

//Firmware version sent through Telemetry
const char version[] = "0.81.06.05.24";

/** List of Sensors and Details
 * bme680 Temperature, Humidity, Pressure, and Gas sensor | https://cdn-learn.adafruit.com/downloads/pdf/adafruit-bme680-humidity-temperature-barometic-pressure-voc-gas.pdf
 * -not currently used in any module
 * 
 * bme280 Temperature, Humidity, and Pressure sensor | https://cdn-learn.adafruit.com/downloads/pdf/adafruit-bme280-humidity-barometric-pressure-temperature-sensor-breakout.pdf
 * -not currently used in any module
 * 
 * Analog Sound Level Meter | https://wiki.dfrobot.com/Gravity__Analog_Sound_Level_Meter_SKU_SEN0232
 * -used in module III, connected to ADC 12 bit | https://www.adafruit.com/product/1083
 * 
 * Rev. C Wind sensor | https://moderndevice.com/product/wind-sensor/
 * -used in module I, connected to ADC 12 bit | https://www.adafruit.com/product/1083
 * 
 * Si7021 Temperature & Humidity sensor | https://cdn-learn.adafruit.com/assets/assets/000/035/931/original/Support_Documents_TechnicalDocs_Si7021-A20.pdf
 * -used in module I, part of Iter8 sensors list
 * 
 * AHTX0 Temperature & Humidity sensor | https://cdn-learn.adafruit.com/downloads/pdf/adafruit-aht20.pdf
 * -not currently used in any module, part of Iter8 sensors list 
 * 
 * TSL2591 High Dynamic Range Digital Light Sensor | https://cdn-learn.adafruit.com/assets/assets/000/078/658/original/TSL2591_DS000338_6-00.pdf?1564168468
 * -used in module III, part of Iter8 sensors list
 * 
 * SI1145 UV index sensor | https://cdn-learn.adafruit.com/downloads/pdf/adafruit-si1145-breakout-board-uv-ir-visible-sensor.pdf
 * -not currently used in any module
 * 
 * AK9753 Human Presence sensor Breakout | https://cdn.sparkfun.com/assets/6/7/9/8/e/AK9753_DS.pdf 
 * -used in module I, part of Iter8 sensors list
 * 
 * SCD30 co2 sensor | https://www.adafruit.com/product/4867
 * -used in module II, part of Iter8 sensors list, has auto calibration and calibration through the callback function from an MQTT publish: "cmnd/C4:4F:33:64:8F:85/co2calibration", value between 400 and 2000 for forced calibration
 * 
 * SGP30 Air Quality sensor | https://www.adafruit.com/product/3709
 * -used in module II, part of Iter8 sensors list, self calibration
 * 
 * SGP40 Air Quality Sensor | https://www.adafruit.com/product/4829
 * -used in module II, part of Iter8 sensors list, self calibration
 * 
 * SPS30 | https://www.digikey.com/en/products/detail/sensirion-ag/SPS30/9598990?utm_adgroup=Sensors%20%26%20Transducers&utm_source=google&utm_medium=cpc&utm_campaign=Dynamic%20Search_EN_Product&utm_term=&utm_content=Sensors%20%26%20Transducers&gclid=CjwKCAjwruSHBhAtEiwA_qCppjb0hZ14WV7e98Jf5-HIEBXHlB96oIYAf0hQ3B6h-kFWIZGY9Ci6VxoCJM8QAvD_BwE
 * -not currently used in any module
 * 
 * PMSA003I Air Quality sensor | https://cdn-shop.adafruit.com/product-files/4632/4505_PMSA003I_series_data_manual_English_V2.6.pdf
 * -used in module II, part of Iter8 sensors list
 * 
 * VEML7700 Light Sensor | https://cdn-learn.adafruit.com/downloads/pdf/adafruit-veml7700.pdf
 * -not currently used in any module
 * 
 * SHT4x Temperature & Humidity Sensor | https://www.digikey.com/en/products?keywords=SHT40-AD1B-R2&v=1649
 * -not currently used in any module
 */


// Adafruit_ADS1115 ads;                  /* 16-bit version */  
// Adafruit_ADS1015 ads1 = Adafruit_ADS1015(0x48); /* 12-bit version ()* dB meter*/
// Adafruit_ADS1015 ads = Adafruit_ADS1015(0x4A); /* 12-bit version (with SDA <==> ADDR) On mainboard (wind/Analog-in pins)*/
Adafruit_BME680 bme680;
Adafruit_BME280 bme280;
Adafruit_Si7021 thSi = Adafruit_Si7021();
// Adafruit_AHTX0 aht = Adafruit_AHTX0();
// Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); // pass in a number for the sensor identifier (for your use later)
// Adafruit_SI1145 uv = Adafruit_SI1145();
// AK975X movementSensor;
// AK975X movementSensorFr;
// SCD30 co2SCD30; //airSensor
// Adafruit_SGP30 sgp30;
// Adafruit_SGP40 sgp40;
// SPS30 pmSPS30;
Adafruit_PM25AQI aqi = Adafruit_PM25AQI(); //Adafruit PMSA003I
// Adafruit_SHT4x sht4 = Adafruit_SHT4x();
SensirionI2CScd4x scd4x;


DynamicJsonDocument devicename(2048);

WiFiUDP Udp;
unsigned int localPort = 8888; // local port to listen for UDP packets

/// which sensors are connected ?
bool sensor0 = 0;  //bme680
bool sensor1 = 0;  //bme280
bool sensor2 = 0;  //sgp30
bool sensor3 = 0;  //scd30
bool sensor4 = 0;  // Adafruit_TSL2591
bool sensor5 = 0;  //Adafruit_VEML7700
bool sensor6 = 0;  //SPS30
bool sensor7 = 0;  //AK975X
bool sensor8 = 0;  //SI1145
bool sensor9 = 0;  //Adafruit_SGP40
bool sensor10 = 0; //Adafruit PMSA003I
bool sensor11 = 0; //Adafruit Si7021
bool sensor12 = 1; // MAXX
bool sensor13 = 0; // Adafruit Sensirion SHT40
bool sensor14 = 1; // Gravity: Analog Sound Level Meter - SEN0232
bool sensor15 = 0; //Adafruit_AHTX0
bool sensor16 = 0; //Adafruit_SCD40
bool sensor17 = 1; //Another AK975X



typedef String (*Iter8Sensors[])(); //Setup for iterating through sensor read functions

time_t getNtpTime();
void digitalClockDisplay();
void printDigits(int digits);
void sendNTPpacket(IPAddress &address);

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

//BME680
double temperature680;
double relativeHumidity680;
double pressurekPa680;
double gasResistanceKOhms;
double Altitude680;
#define SEALEVELPRESSURE_HPA (1008.9) /// More accurate Altitude680 if we can poll this value

//BME280
double temperature280;
double relativeHumidity280;
double pressurekPa280;
double Altitude280;

//Si7021
double tdb_si7021c; //temperature celsius
double tdb_si7021f; //temperature fahrenheit
double rh_si7021;   //relative humidity
String si7021keys[] = {"tdb_si7021_C", "tdb_si7021_F", "rh_si7021"};
//char* si7021_values;

//AHTX0 
sensors_event_t humidity, temp;
double tdb_AHTX0c; //temperature celsius
double tdb_AHTX0f; //temperature fahrenheit
double rh_AHTX0;   //humidity
String AHTX0keys[] = {"tdb_AHTX0_C", "tdb_AHTX0_F", "rh_AHTX0"};
//char* AHTX0_values;

//SGP30
double Tvoc;
double Eco2;
uint16_t tvoc_base, eco2_base; // SGP30 baseline correction algorithm value for CO2 [raw]

struct SGP30Data
{
   SGP30Data()
   {
      temperature = NAN;
      humidity = NAN;
      voc = NAN;
      co2 = NAN;
      absoluteHumidity = NAN;
      eco2_base = 0;
      tvoc_base = 0;
   };

   float temperature;      // [°C]
   float humidity;         // [%RH]
   float voc;              // [ppb]
   float co2;              // [ppm]
   float absoluteHumidity; // [mg/m^3]
   uint16_t eco2_base;     // SGP30 baseline correction algorithm value for CO2 [raw]
   uint16_t tvoc_base;     // SGP30 baseline correction algorithm value for VOC [raw]
};

struct EepromStorage
{
   uint8_t version;    // helper value to detect if EEPROM needs to be initialized (default 0xFF for empty EEPROM)
   bool baselineValid; // flag set once SGP30 baseline calibration sequence is completed (12 hours)
   long timeStamp;     // timestamp when baseline calibration sequence has been started or last valid baseline has been stored [Unix time format in seconds since January 1, 1970]
   uint16_t eco2_base; // SGP30 baseline value for CO2
   uint16_t tvoc_base; // SGP30 baseline value for TVOC
};

EepromStorage persistency;
bool baselineCalibrationInProgress = false;

//SGP40
double Tvoc_indx; // The VOC Index represents an air quality value on a scale from 0 to 500 where a lower value represents cleaner air.
double Tvoc_raw;
String SGP40keys[] = {"Tvoc_indx", "Tvoc_raw"};
//char* SGP40values;

//TSL2591
uint16_t TSL2591_VIS;
uint16_t TSL2591_FULL;
uint16_t TSL2591_INFR;
String TSL2591keys[] = {"tsl2591_vis", "tsl2591_full", "tsl2591_infr"};
//char* TSL2591values;

//SCD30
double Rco2 = 0;
double temperaturescd_C = 0;
double temperaturescd_F = 0;
double relativeHumidityscd = 0;
String SCD30keys[] = {"rco2_scd30", "tdb_scd30_C", "tdb_scd30_F", "rh_scd30"};


//SCD40
double Rco2_SCD40 = 0;
double temperaturescd40_C = 0;
double temperaturescd40_F = 0;
double relativeHumidityscd40 = 0;
String SCD40keys[] = {"rco2_scd40", "tdb_scd40_C", "tdb_scd40_F", "rh_scd40"};

//char* SCD30values;

//SPS30
//float mass_concen[4];
//float num_concen[5];
//double PM1_SPS30 = mass_concen[1];
//double PM2_SPS30 = mass_concen[2];
//double PM10_SPS30 = mass_concen[3];

//PMSA0030I
float mass_concen[4];
float num_concen[5];
double PM1 = mass_concen[1];
double PM2 = mass_concen[2];
double PM10 = mass_concen[3];
String PMSkeys[] = {"pms_pm1", "pms_pm25", "pms_pm10"};
//char* pmsvalues;

//String tmeStamp;
time_t epoch;
//unsigned long now;


/** Intervals
 * Serial Print interval, MQTT publish interval, Read interval, Display interval, Telemetry publish interval, battery level measurement interval
*/

//Soft reset
uint32_t lastReset = 0;
//int reboot_interval = 60; // minutes to reboot system
int reboot_interval = 1440; // minutes to reboot system
#define reboot_MILLIS (reboot_interval*60000UL)

//Ping host
uint32_t lastPing = 0;
int ping_interval = 5; // minutes to reboot system
#define ping_MILLIS (ping_interval*60000UL)


int msinterval = 15; // seconds for sensor read interval
#define ms_MILLIS (msinterval * 1000)
unsigned long mslastRead = 0; ///seting 0 on init will post on first boot

#define ONE_DAY_MILLIS (24 * 60 * 60 * 1000)
unsigned long lastSync = millis();

int Tinterval = 1800; // seconds for telemetry publish interval
#define TPost_MILLIS (Tinterval * 1000)
unsigned long TlastPost = 0; ///seting 0 on init will post on first boot

int force = 1;

int DPinterval = 60; // seconds for display refresh interval
#define Display_MILLIS (Tinterval * 1000)
unsigned long displayPost = 0; ///seting 0 on init will post on first boot

int SDinterval = 60; // seconds for Serial publish interval
#define SDwrite_MILLIS (SDinterval * 1000)
unsigned long lastWrite = 0; ///seting 0 on init will post on first boot

int interval = 60; // seconds for MQTT publish interval
#define Post_MILLIS (interval * 1000)
unsigned long lastPost = 0; ///seting 0 on init will post on first boot

int rdinterval = 5; // seconds for sensor read interval
#define Read_MILLIS (rdinterval * 1000)
unsigned long lastRead = 0; ///seting 0 on init will post on first boot

int epdinterval = 180 * 2; // seconds for sensor read interval
#define epd_MILLIS (epdinterval * 1000)
unsigned long lastepd = 0; ///seting 0 on init will post on first boot

int batInterval = 60; //seconds for battery level measurement interval
#define batterVal (batInterval * 1000)
unsigned long old_time = 0; //seting 0 on init will post on first boot
float voltage;
float batt;

//coordinate values to locate sensor in space w.r.t the building's coordinate system
double Coord_X;
double Coord_Y;
double Coord_Z;
double CoordRotation; //degrees rotation CCW from Earth North to building Y axis
String location;


String DeviceID = WiFi.macAddress();
String device_name;

char sensor[1000];
char Tele2[187];
typedef double(output[]);
typedef char *(datakeys[]);

int loc = 3;
int counter = 0;

#define analogPinForRV A1 // change to pins you the analog pins are using *not now we use ADC input
#define analogPinForTMP A2

//const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)

/////////// Variables for DBA levels Analog Sound Level Meter SKU:SEN0232
uint16_t sample;
String dbakeys[] = {"DBA_SEN0232"};
double dbvalue;
double voltagevalue;
double meter;
double meterraw;
int samples = 8;            //For FFT this value MUST ALWAYS be a power of 2*
int samplingFrequency = 50; //Period
float bias_level = 0;
float multiplier = 0.1875; //milli Volts per bit for ADS1115 !!! change if gain is adjusted
//0.1875mV (default)
//0.125mV (gain XX)
float multiplier1 = 3;  //milli Volts per bit for ADS1015 !!! change if gain is adjusted

/// Airspeed constants
//const double zeroWindAdjustment =  0.2; // negative numbers yield smaller wind speeds and vice versa.
// Paul says "negative" but when I plugged in an actual negative it messed up the power equation.
//0.05 sets the speed close to 0.01 when sensor is under a jar, but powered by a USB port which is delivering something like 5.12V
double zeroWindAdjustment = 0.2;

const bool debug_wind = true;
String windkeys[] = {"AS_mph", "AS_ms", "tdb_rev_C", "tdb_rev_F","0V", "RV"};
// with voltage values for advanced debug
//String windkeys[] = {"AS_mph", "AS_ms", "tdb_rev_C", "tdb_rev_F","0V_revc", "RV_revc"};
char *windvalues;
//average mulitple analog reads to calm the signal down a bit.
int nSamples = 16; //how many analog reads to average:
int anaReadSum = 0;
int anaReadDelay = 3; //in ms

//Airspeed(AS) variables read_AS()
int Tref;         //temp termistor value from wind sensor. //expects output from a 10bit AD
int Tref_3p3p12b; //reference T voltage as read by particle AD
int Tref_x100_C;
int Tref_x100_F;
float Tref_V;
float RawAS_5Vp10b; //RV output from wind sensor //expects output from a 10bit AD
int RawAS_3p3p12b;
float RawAS_V;
float AS_MPH;
float AS_mps;
float zeroAS_5Vp10b; //reported/calculated as if it was 5V/10bits
float zeroAS_V;

/// movementsensor read_AK975X()
int16_t ir_dwn, ir_lft, ir_up, ir_rgt;
float ir_tempc, ir_tempf;
String AK975Xkeys[] = {"ir_up", "ir_down", "ir_left", "ir_right", "ir_temp_C", "ir_temp_F"};
char *AK975Xvalues;

/// movementsensor read_AK975X()
int16_t ir_dwn2, ir_lft2, ir_up2, ir_rgt2;
float ir_tempc2, ir_tempf2;
String AK975X2keys[] = {"ir_up1", "ir_down1", "ir_left1", "ir_right1", "ir_temp_C1", "ir_temp_F1"};
char *AK975X2values;

// VEML
double Lum = 0;

// UV index sensor
int lum_visible_si1145;
int lum_ir__si1145;
int lum_uv__si1145;

//loop delay-er
unsigned long lastMillis;

//translate values from the ADC and voltage of Arduino (10bit and 5V) to Particle (12bit and 3.3V)
float scale_Ard2Part = ((3.3 / 5.0) * 1 / 4.0);

int lOopDelay = 300;

// 'shop_logo', 46x25px
const unsigned char shoplogo1[] PROGMEM = {
    0x00, 0x07, 0x80, 0x00, 0x00, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x00, 0x00, 0x07, 0x80, 0x00,
    0x00, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x00, 0x1f, 0xc7,
    0xbe, 0x00, 0xf7, 0xe0, 0x3f, 0xf7, 0xff, 0x00, 0xff, 0xf0, 0x7f, 0xff, 0xff, 0x80, 0xff, 0xf8,
    0x78, 0xff, 0xcf, 0x80, 0xf8, 0xf8, 0x78, 0x4f, 0x87, 0x00, 0x70, 0x7c, 0x7f, 0x87, 0x87, 0x00,
    0x70, 0x3c, 0x3f, 0xe7, 0x86, 0x00, 0x70, 0x3c, 0x1f, 0xff, 0x86, 0x00, 0x70, 0x3c, 0x07, 0xff,
    0x86, 0x00, 0x70, 0x3c, 0x00, 0x7f, 0x87, 0x00, 0x70, 0x7c, 0x78, 0x7f, 0x87, 0x00, 0x78, 0x7c,
    0x7f, 0xff, 0x87, 0x80, 0xff, 0xf8, 0x3f, 0xf7, 0x87, 0x80, 0xff, 0xf0, 0x1f, 0xe7, 0x87, 0x80,
    0xf7, 0xe0, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
void printUint16Hex(uint16_t value) {
    Serial.print(value < 4096 ? "0" : "");
    Serial.print(value < 256 ? "0" : "");
    Serial.print(value < 16 ? "0" : "");
    Serial.print(value, HEX);
}

void printSerialNumber(uint16_t serial0, uint16_t serial1, uint16_t serial2) {
    Serial.print("Serial: 0x");
    printUint16Hex(serial0);
    printUint16Hex(serial1);
    printUint16Hex(serial2);
    Serial.println();
}

void getSCD40Serial(){
   uint16_t error;
   char errorMessage[256];
   // stop potentially previously started measurement
   error = scd4x.stopPeriodicMeasurement();
   if (error) {
      Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
      errorToString(error, errorMessage, 256);
      Serial.println(errorMessage);
   }

   uint16_t serial0;
   uint16_t serial1;
   uint16_t serial2;
   error = scd4x.getSerialNumber(serial0, serial1, serial2);
   if (error) {
      Serial.print("Error trying to execute getSerialNumber(): ");
      errorToString(error, errorMessage, 256);
      Serial.println(errorMessage);
      sensor16 = 0;
   } else {
      printSerialNumber(serial0, serial1, serial2);
      sensor16 = 1;
   }

   // Start Measurement
   error = scd4x.startPeriodicMeasurement();
   if (error) {
      Serial.print("Error trying to execute startPeriodicMeasurement(): ");
      errorToString(error, errorMessage, 256);
      Serial.println(errorMessage);
   }

   Serial.println("SCD4X INIT");

}

void debugPrint(String line)
{
   if (debug)
   {
      Serial.println(line);
   }
   else
   {
   }
}

String convert_Payload(byte *payload, unsigned int length) //converts byte to string
{
   String Str_payload;
   for (int i = 0; i < length; i++)
   {
      Str_payload += (char)payload[i];
   }
   return Str_payload;
}

int roundoff(float val)
{
   int value = round(val * 100);
   return value;
}

void writeDouble(int ee, double value)
{
   byte *p = (byte *)(void *)&value;
   for (int i = 0; i < sizeof(value); i++)
      EEPROM.write(ee++, *p++);
}

double readDouble(int ee)
{
   double value;
   byte *p = (byte *)(void *)&value;
   for (int i = 0; i < sizeof(value); i++)
      *p++ = EEPROM.read(ee++);
   return value;
}

int WScalibration(String command)
{  //Adust zerowind value
   // Ritual incantation to convert String into Int
   char inputStr[64];
   command.toCharArray(inputStr, 64);
   int locaddr = 20; //this needs to be different for every function that's writing to EEPROM
   double value = strtof(inputStr, NULL);
   //int val = 2;
   EEPROM.writeFloat(locaddr, value);
   //writeDouble(locaddr, value);
   Serial.print("zeroWindAdjustment Updated: ");
   Serial.println(value);
   zeroWindAdjustment = value;
   float zeroWindAdjustment1 = EEPROM.readFloat(locaddr);
   Serial.print("zeroWindAdjustment Written?: ");
   Serial.println(zeroWindAdjustment);
   Serial.println(zeroWindAdjustment1);
   //zeroWindAdjustment = value;
   if (client.connected()){
      String resptopic = "SHoPSense/resp";
      resptopic += "/WScalibration"+WiFi.macAddress();
      //const char* zerowindchar = zeroWindAdjustment;
      client.publish(resptopic.c_str(), String(zeroWindAdjustment).c_str());
   }

   //Respond
   return 1;
}

int co2calibration(String command)
{
   // Ritual incantation to convert String into Int
   char inputStr[64];
   command.toCharArray(inputStr, 64);
   int i = atoi(inputStr);

   co2SCD30.setForcedRecalibrationFactor(i);

   return 1;
}

String reSerializeIt(String keys[], output out, int bufferSize)
{
   String values;
   size_t capacity = JSON_ARRAY_SIZE(bufferSize) + JSON_OBJECT_SIZE(2);
   DynamicJsonDocument foc(capacity);
   //JsonObject obj = foc.as<JsonObject>();
   for (uint i = 0; i < bufferSize; i++)
   {
      foc[keys[i].c_str()] = float(out[i]);
      //Serial.println(out[i]);
   }
   //const size_t size = measureJson(foc);
   //Serial.print("size: ");
   //Serial.println(size);
   serializeJson(foc, values);
   //Serial.print(values);
   return values;
}

double CtoF(double tempC){
   double tempf;
   tempf = tempC * 9 / 5 + 32;
   return tempf;
}

//Adafruit Si7021 Temperature & Humidity Sensor | https://cdn-learn.adafruit.com/assets/assets/000/035/931/original/Support_Documents_TechnicalDocs_Si7021-A20.pdf
String read_Si7021()
{
   String si7021values;
   if (sensor11)
   {
      tdb_si7021c = thSi.readTemperature();
      tdb_si7021f = tdb_si7021c * 9 / 5 + 32;
      rh_si7021 = thSi.readHumidity();
      double roundval = 100.00;
      output out = {roundoff(tdb_si7021c) / roundval, roundoff(tdb_si7021f) / roundval, roundoff(rh_si7021) / roundval};
      // output out = {roundoff(tdb_si7021c), roundoff(tdb_si7021f), roundoff(rh_si7021)};
      uint bufferSize = ARRAY_SIZE(out);
      //Serial.print("bufferSize: ");
      //Serial.println(bufferSize);
      String si7021values = reSerializeIt(si7021keys, out, bufferSize);
      return si7021values;   
   }
   else if (!sensor11)
   {
      tdb_si7021c = -1;
      tdb_si7021f = -1;
      rh_si7021 = -1;
      String si7021values = "";
      return si7021values;
   }

   return si7021values;
}

//Adafruit AHTX0 Temperature & Humidity Sensor | https://cdn-learn.adafruit.com/downloads/pdf/adafruit-aht20.pdf
String read_AHTX0()
{
   String AHTX0values;
   if (sensor15)
   {                                  //if the sensor is there
      aht.getEvent(&humidity, &temp); // reading for temperature and humidity
      tdb_AHTX0c = (temp.temperature);
      tdb_AHTX0f = tdb_AHTX0c * 9 / 5 + 32;
      rh_AHTX0 = (humidity.relative_humidity);
      output out = {roundoff(tdb_AHTX0c) / 100.00, roundoff(tdb_AHTX0f) / 100.00, roundoff(rh_AHTX0) / 100.00}; //rounding the readings
      uint bufferSize = ARRAY_SIZE(out);
      //Serial.print("bufferSize: ");
      //Serial.println(bufferSize);
      String AHTX0values = reSerializeIt(AHTX0keys, out, bufferSize);     \
      return AHTX0values; 
   }
   else if (!sensor15)
   { //if the sensor is not there
      tdb_AHTX0c = -1;
      tdb_AHTX0f = -1;
      rh_AHTX0 = -1;
      String AHTX0values = "";
      return AHTX0values;
   }

   return AHTX0values;
}

/// PMSA003I Air Quality Breakout  | https://cdn-shop.adafruit.com/product-files/4632/4505_PMSA003I_series_data_manual_English_V2.6.pdf | https://www.adafruit.com/product/4632
String read_PMSA003I()
{
   String PMSvalues;
   PM25_AQI_Data data;
   if (! aqi.read(&data))
   { //PM sensor
      //Serial.println("Error trying to execute aqi.read");
      PM1 = -1;
      PM2 = -1;
      PM10 = -1;
      String PMSvalues = "";
      return PMSvalues;   
   }
   PM1 = data.pm10_standard;
   PM2 = data.pm25_standard;
   PM10 = data.pm100_standard;
   output out = {roundoff(PM1) / 100.00, roundoff(PM2) / 100.00, roundoff(PM10) / 100.00};
   uint bufferSize = ARRAY_SIZE(out);
   //Serial.print("bufferSize: ");
   //Serial.println(bufferSize);
   PMSvalues = reSerializeIt(PMSkeys, out, bufferSize);
   //Serial.print("pmPMS.read:");
   //Serial.println(PMSvalues);
   return PMSvalues;
}

/// Adafruit TSL2591 High Dynamic Range Digital Light Sensor | https://cdn-learn.adafruit.com/assets/assets/000/078/658/original/TSL2591_DS000338_6-00.pdf?1564168468
String read_TSL2591()
{
   String TSL2591values;
   if (sensor4)
   {
      ///light sensor
      TSL2591_VIS = tsl.getLuminosity(TSL2591_VISIBLE);
      TSL2591_FULL = tsl.getLuminosity(TSL2591_FULLSPECTRUM);
      TSL2591_INFR = tsl.getLuminosity(TSL2591_INFRARED);
      if (TSL2591_FULL==65535){
         TSL2591_VIS = -1;
         TSL2591_FULL = -1;
         TSL2591_INFR = -1;
         String TSL2591values = "" ;
         return TSL2591values;         
      }else {
      output out = {roundoff(TSL2591_VIS) / 100.00, roundoff(TSL2591_FULL) / 100.00, roundoff(TSL2591_INFR) / 100.00};
      uint bufferSize = ARRAY_SIZE(out);
      //Serial.print("bufferSize: ");
      //Serial.println(bufferSize);
      String TSL2591values = reSerializeIt(TSL2591keys, out, bufferSize);
      return TSL2591values;
      }
   }
   else if (!sensor4)
   {
      TSL2591_VIS = -1;
      TSL2591_FULL = -1;
      TSL2591_INFR = -1;
      String TSL2591values = "" ;
      return TSL2591values;
   }

   return TSL2591values;
}

//Adafruit co2 sensors | https://www.adafruit.com/product/4867
String read_SCD30Co2()
{
   String SCD30values;
   if (sensor3)
   {
      Rco2 = co2SCD30.getCO2();
      temperaturescd_C = co2SCD30.getTemperature();
      temperaturescd_F = temperaturescd_C * 9 / 5 + 32;
      relativeHumidityscd = co2SCD30.getHumidity();
      output out = {roundoff(Rco2) / 100.00, roundoff(temperaturescd_C) / 100.00, roundoff(temperaturescd_F) / 100.00, roundoff(relativeHumidityscd) / 100.00};
      uint bufferSize = ARRAY_SIZE(out);
      //Serial.print("bufferSize: ");
      //Serial.println(bufferSize);
      //SGP40values = reSerializeIt(SGP40keys,out,bufferSize);
      String SCD30values = reSerializeIt(SCD30keys, out, bufferSize);
      return SCD30values;  
   }
   else if (!sensor3)
   {
      Rco2 = -1;
      temperaturescd_C = -1;
      temperaturescd_F = -1;
      relativeHumidityscd = -1;
      String SCD30values = "";
      return SCD30values;
   }

   return SCD30values;
}

//Adafruit co2 sensors | #### ADDD
String read_SCD40Co2()
{
   uint16_t error;
   char errorMessage[256];
   uint16_t co2 = 0;
   float temperature = 0.0f;
   float humidity = 0.0f;
   String SCD40values;
   if (sensor16)
   {
      error = scd4x.readMeasurement(co2, temperature, humidity);
      if (error) {
         //Serial.print("Error trying to execute readMeasurement(): ");
         //errorToString(error, errorMessage, 256);
         //Serial.println(errorMessage);
      } else if (co2 == 0) {
         Serial.println("Invalid CO2 sample detected, skipping.");
         Rco2_SCD40 = -1;
         temperaturescd40_C = -1;
         temperaturescd40_F = -1;
         relativeHumidityscd40 = -1;
      } else {

         Rco2_SCD40 = co2;
         temperaturescd40_C = temperature;
         temperaturescd40_F = temperaturescd40_C * 9 / 5 + 32;
         relativeHumidityscd40 = humidity;
         output out = {roundoff(Rco2_SCD40) / 100.00, roundoff(temperaturescd40_C) / 100.00, roundoff(temperaturescd40_F) / 100.00, roundoff(relativeHumidityscd40) / 100.00};
         uint bufferSize = ARRAY_SIZE(out);
         //Serial.print("bufferSize: ");
         //Serial.println(bufferSize);
         //SGP40values = reSerializeIt(SGP40keys,out,bufferSize);
         String SCD40values = reSerializeIt(SCD40keys, out, bufferSize);    
         return SCD40values;     
      }

   }
   else if (!sensor16)
   {
      ///xx
      String SCD40values = "";
      return SCD40values;

   }

   return SCD40values;
}

// Adafruit SGP40 Air Quality Sensor Breakout - VOC Index | https://cdn-learn.adafruit.com/assets/assets/000/097/511/original/Sensirion_Gas-Sensors_SGP40_Datasheet.pdf?1607381770
String read_SGP40()
{
   String SGP40values;
   if (sensor9)
   {
      float temp = co2SCD30.getTemperature(); // Read SCD30 Temp/Humidity
      float hum = co2SCD30.getHumidity();     // Read SCD30 Temp/Humidity
      Tvoc_indx = sgp40.measureVocIndex(temp, hum);
      Tvoc_raw = sgp40.measureRaw(temp, hum);
      output out = {roundoff(Tvoc_indx) / 100.00, roundoff(Tvoc_raw) / 100.00};
      uint bufferSize = ARRAY_SIZE(out);
      //Serial.print("bufferSize: ");
      //Serial.println(bufferSize);
      //SGP40values = reSerializeIt(SGP40keys,out,bufferSize);
      String SGP40values = reSerializeIt(SGP40keys, out, bufferSize);
      //Serial.print(SGP40values);
      return SGP40values;
   }
   else if (!sensor9)
   {
      Tvoc_raw = -1;
      Tvoc_indx = -1;
      String SGP40values =  "";
      return SGP40values;
   }

   return SGP40values;
}

/// SparkFun Human Presence Sensor Breakout - AK9753 | https://cdn.sparkfun.com/assets/6/7/9/8/e/AK9753_DS.pdf
String read_AK975X()
{

   String AK975Xvalues;
   if (sensor7) //Single HP sensor mode
   {
      if (movementSensor.available())
      {
         ir_dwn = movementSensor.getIR1();
         ir_lft = movementSensor.getIR2();
         ir_up = movementSensor.getIR3();
         ir_rgt = movementSensor.getIR4();
         ir_tempc = movementSensor.getTemperature();
         ir_tempf = ir_tempc * 9 / 5 + 32;
         movementSensor.refresh(); //Read dummy register after new data is read
         // Serial.print("ir_up");
         // Serial.println(ir_up);
         // ir_up,ir_dwn,ir_lft,ir_rgt,roundoff(ir_tempc)/100.00
         output out = {ir_up, ir_dwn, ir_lft, ir_rgt, roundoff(ir_tempc) / 100.00, roundoff(ir_tempf) / 100.00};
         uint bufferSize = ARRAY_SIZE(out);
         //Serial.print("bufferSize: ");
         //Serial.println(bufferSize);
         String AK975Xvalues = reSerializeIt(AK975Xkeys, out, bufferSize);
         return AK975Xvalues;                
      }
   }
   else if (!sensor7)
   {
      ir_dwn = -1;
      ir_lft = -1;
      ir_up = -1;
      ir_rgt = -1;
      ir_tempc = -1;
      ir_tempf = -1;
      String AK975Xvalues = "";
      return AK975Xvalues;
   }
  
   return AK975Xvalues;
}


String read_AK975X2nd ()
{
   String AK975X2values;
   if (sensor17) //Dual HP sensor mode
   {
      ir_dwn2 = movementSensorFr.getIR1();
      ir_lft2 = movementSensorFr.getIR2();
      ir_up2 = movementSensorFr.getIR3();
      ir_rgt2 = movementSensorFr.getIR4();
      ir_tempc2 = movementSensorFr.getTemperature();
      ir_tempf2 = ir_tempc2 * 9 / 5 + 32;
      movementSensorFr.refresh(); //Read dummy register after new data is read
      output out = {ir_up2, ir_dwn2, ir_lft2, ir_rgt2, roundoff(ir_tempc2) / 100.00, roundoff(ir_tempf2) / 100.00};
      uint bufferSize = ARRAY_SIZE(out);
      //Serial.print("bufferSize: ");
      //Serial.println(bufferSize);
      String AK975X2values = reSerializeIt(AK975X2keys, out, bufferSize);
      return AK975X2values;
   }
   else if (!sensor17)
   {
      ir_dwn2 = -1;
      ir_lft2 = -1;
      ir_up2 = -1;
      ir_rgt2 = -1;
      ir_tempc2 = -1;
      ir_tempf2 = -1;
      String AK975X2values = "";
      return AK975X2values;
   }
   return AK975X2values;
}

//dB meter used for calibration
String dBmeter_read(int pin)
{
   int16_t adc2, adc3;
   typedef double(output[]);
   const char keys[] = {"Voltage"};

   double av2, av3;
   anaReadSum = 0;
   for (byte i = 0; i < samples; i++)
   {
      anaReadSum = anaReadSum + ads.readADC_SingleEnded(pin);
   }
   adc3 = anaReadSum / samples;
   av3 = adc3 * (multiplier1) / 1000; // Voltage
   output out = {(roundoff(av3)) / 100.00};
   uint bufferSize = 1;
   const size_t capacity = JSON_ARRAY_SIZE(bufferSize * 2) + 2 * JSON_OBJECT_SIZE(2);
   DynamicJsonDocument foc(capacity);

   foc["key"][0] = keys[0];
   foc["value"][0] = out[0];
   //Serial.println(out[i]);

   int size = measureJson(foc);
   //Serial.print("size: ");
   //Serial.println(size);
   //serializeJson(foc, adcvalues);
   String adcvalues;
   return adcvalues;
}

/// Gravity: Analog Sound Level Meter - SKU:SEN0232 | https://wiki.dfrobot.com/Gravity__Analog_Sound_Level_Meter_SKU_SEN0232#target_0
String DBA_read()
{
   int16_t adc;

   unsigned long startMillis = millis(); // Start of sample window
   double db;
   double av;
   anaReadSum = 0;
   for (byte i=0; i<samples; i++){
      anaReadSum = anaReadSum + ads1.readADC_SingleEnded(0);
      //Serial.print("DBA read: ");
      //Serial.print(ads1.readADC_SingleEnded(0));
   }
   adc = anaReadSum/samples;
   //adc = ads1.readADC_SingleEnded(0);
   String db_lvl ;
   if (adc < 4094){
      av = adc * (multiplier1) / 1000; // Voltage
      db = av * 50.0;
      output out = {db};
      uint bufferSize = ARRAY_SIZE(out);
      //Serial.print("bufferSize: ");
      //Serial.println(bufferSize);
      String db_lvl = reSerializeIt(dbakeys, out, bufferSize);
      return db_lvl;     
   }else {
      db = -1;
      String db_lvl = "" ;
      
   }

   return db_lvl;
   //delay(5);
}


//Airspeed Sensor Read
String read_AS()
{
   int16_t adc0, adc1;
   double av0, av1;

   //get voltage for raw AS signal
   anaReadSum = 0;
   int anaReadSum1 = 0;
   for (byte i = 0; i < nSamples; i++)
   {
      anaReadSum = anaReadSum + ads.readADC_SingleEnded(0);
      anaReadSum1 = anaReadSum1 + ads.readADC_SingleEnded(1);  
      //Serial.print("adc0: ");
      //Serial.println(adc0);
   }
   adc0 = anaReadSum / nSamples;
   adc1 = anaReadSum1 / nSamples;
   String windvalues = "" ;
//   if (adc0 < 4094){
   av0 = adc0 * (multiplier1) / 1000;    // Voltage
   av1 = adc1 * (multiplier1) / 1000;    // Voltage
   //Tref = map(adc1, 0, 26560, 0, 1023); // scaling from 16-bit ADC reading to expected 10 bit on Arduino ///cut to 4.9V max
   Tref = map(adc1,0,2048,0,1023); // scaling from 12-bit ADC reading to expected 10 bit on Arduino

   RawAS_V = av0;
   Tref_x100_C = (0.005 * ((float)Tref * (float)Tref)) - (16.862 * (float)Tref) + 9075.4;
   zeroAS_5Vp10b = -0.0006 * ((float)Tref * (float)Tref) + 1.0727 * (float)Tref + 47.172; //  13.0C  553  482.39

   zeroAS_V = (float(zeroAS_5Vp10b) * 0.0048828125) - zeroWindAdjustment;
   //zeroAS_V = (float(zeroAS_5Vp10b)* muli) - zeroWindAdjustment;

   // This from a regression from data in the form of
   // Vraw = V0 + b * AS ^ c
   // V0 is zero wind at a particular temperature
   // The constants b and c were determined by some Excel wrangling with the solver.
   if ((RawAS_V - zeroAS_V) >= 0)
   { //prevent negative values
      AS_MPH = pow(((RawAS_V - zeroAS_V) / .2300), 2.7265);
      AS_mps = AS_MPH * 1609.344 / 3600.0;
   }
   else
   {
      AS_MPH = -1;
      AS_mps = -1;
   }
   if (debug_wind == true)
   {
      //Serial.print("Tref: ");
      //Serial.print(Tref);
      //Serial.print(",");
      //   Serial.print("T_C: ");
      //   Serial.print(float(Tref_x100_C)/100, 1);
      //   Serial.print(",");

      //   Serial.print("0Wind_V: ");
      //   Serial.print(zeroAS_V, 3);
      //   Serial.print(",");
      //   Serial.print("Raw_V: ");
      //   Serial.println(RawAS_V, 3);
      //Serial.print(",");
      //Serial.print("Raw_AD:  ");
      //Serial.print(adc3);
      //Serial.println(" ");
   }
   //Serial.println("---------------_________---------------");
   //Serial.print("AirSp_mps:");
   //Serial.println(AS_mps, 2);
   //Serial.println(",");
   //Serial.print("AirSp_MPH: ");
   //Serial.println(AS_MPH, 2);
   //Serial.println("\t________________----------________________");
   Tref_x100_F = ((float(Tref_x100_C) / 100) * 9.00 / 5.00 + 32.00) * 100;
   output out = {(roundoff(AS_MPH)) / 100.00, (roundoff(AS_mps)) / 100.00, float(Tref_x100_C) / 100, roundoff(float(Tref_x100_F) / 100) / 100.00,float(av0),float(av1)};
   uint bufferSize = ARRAY_SIZE(out);
   //Serial.print("bufferSize: ");
   //Serial.println(bufferSize);
   windvalues = reSerializeIt(windkeys, out, bufferSize);
   //Serial.print(windvalues);  



   // //get voltage for Tref signal
   // anaReadSum = 0;
   // for (byte i=0; i<nSamples; i++){
   //    anaReadSum = anaReadSum + ads.readADC_SingleEnded(1);
   // }
   // adc1 = anaReadSum/nSamples;
   return windvalues;
}

/* Helper to calculate absolute humidity [mg/m^3] with approximation formula based on temperature [°C] and humidity [%RH].
* @param const SGP30Data& sample
* @return absolute humidity [mg/m^3]
*/
uint32_t getAbsoluteHumidity(const SGP30Data &sample)
{
   // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
   const float absoluteHumidity = 216.7f * ((sample.humidity / 100.0f) * 6.112f * exp((17.62f * sample.temperature) / (243.12f + sample.temperature)) / (273.15f + sample.temperature)); // [g/m^3]
   const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity);                                                                                            // [mg/m^3]
   return absoluteHumidityScaled;
}

/* Read the temperature and humidity from SHT31-D and store in the given sample
* @param SGP30Data& sample - to store temperature and humidity
*/
void readTemperatureHumidity(SGP30Data &sample)
{
   if (sensor3)
   {
      sample.temperature = temperaturescd_C;
      sample.humidity = relativeHumidityscd;
      //Serial.print("Temp = "); Serial.print(sample.temperature); Serial.println(" °C");
      //Serial.print("Humidity = "); Serial.print(sample.humidity); Serial.println(" %RH");
   }
   else
   {
      Serial.println("SHT31D measurement failed");
   }
}

/* Set the absolute humidity on the SGP30 for more acurate readings of air quality signals (TVOC & eCO2) and store absolute humidity in the given sample.
* @param SGP30Data& sample - to store absolute humidity value
*/
void setHumidityCompensation(SGP30Data &sample)
{
   uint32_t absoluteHumidity = getAbsoluteHumidity(sample);
   if (sgp30.setHumidity(absoluteHumidity))
   {
      sample.absoluteHumidity = absoluteHumidity;
      Serial.print("Absolute Humidity = ");
      Serial.print(sample.absoluteHumidity);
      Serial.println(" mg/m^3");
   }
   else
   {
      Serial.println("SGP30 setting absolute humidity failed");
   }
}

//// SDWRITE  //////////////////////////////////////////////
void sdWrite(char *line)
{
   if (sdwrite == 1)
   {
      String date = String(month()) + "-" + String(day()) + "-" + String(year());
      String file = (("SHoPSense_" + date + "_.csv"));
      myFile = SD.open(file, FILE_WRITE);
      if (myFile)
      {
         Serial.print("Writing to: " + file + " ..........");
         myFile.println(line);
         //myFile.println("testing 1, 2, 3.");
         // close the file:
         myFile.close();
         Serial.println("done.");
      }
      else
      {
         // if the file didn't open, print an error:
         Serial.println("error opening: " + file);
      }
   }
   // if the file opened okay, write to it:
}

double get_voltage()
{
   /*
   //get voltage for Tref signal
   anaReadSum = 0;
   for (byte i=0; i<nSamples; i++){
      anaReadSum = anaReadSum + ads.readADC_SingleEnded(1);
   }    
   adc1 = anaReadSum/nSamples;
   
   av1 = adc1 * (multiplier)/1000; // Voltage
   Tref = map(adc1,0,32768,0,4095); // 
   */
}

void printLine()
{
   Serial.println(" ");
   for (int i = 0; i < 30; i++)
      Serial.print("-");
   Serial.println(" ");
}

/// MQTT Callback variables

//functions for updating any intervals
int serialinterval(String command)
{ ///Serial publish interval
   // Ritual incantation to convert String into Int
   char inputStr[64];
   command.toCharArray(inputStr, 64);
   int i = atoi(inputStr);

   SDinterval = i;
   // Respond
   return 1;
}

int postinterval(String command)
{ ///MQTT sensor post interval
   // Ritual incantation to convert String into Int
   char inputStr[64];
   command.toCharArray(inputStr, 64);
   int i = atoi(inputStr);

   interval = i;
   // Respond
   return 1;
}

int readinterval(String command)
{ ///sensor read interval
   // Ritual incantation to convert String into Int
   char inputStr[64];
   command.toCharArray(inputStr, 64);
   int i = atoi(inputStr);

   rdinterval = i;
   // Respond
   return 1;
}

int teleinterval(String command)
{ ///MQTT telemetry post interval
   // Ritual incantation to convert String into Int
   char inputStr[64];
   command.toCharArray(inputStr, 64);
   int i = atoi(inputStr);

   Tinterval = i;
   // Respond
   return 1;
}

int displayinterval(String command)
{ //display refresh interval
   // Ritual incantation to convert String into Int
   char inputStr[64];
   command.toCharArray(inputStr, 64);
   int i = atoi(inputStr);

   DPinterval = i;
   // Respond
   return 1;
}

//List of sensors to iterate through
Iter8Sensors sensorslist = {DBA_read, read_AS, read_Si7021, read_AHTX0, read_SCD30Co2,read_SCD40Co2, read_PMSA003I, read_SGP40, read_TSL2591,read_AK975X};

void JsonSensor()
{
   //const size_t capacity2 = 3*JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2);
   const size_t capacity2 = JSON_OBJECT_SIZE(50) + JSON_OBJECT_SIZE(20);
   DynamicJsonDocument json(capacity2);
   //JsonObject object = json.to<JsonVariant>();
   json["Device_id"] = WiFi.macAddress();
   long time = now();
   json["timestamp"] = time;
   JsonObject arr = json.createNestedObject("data");
   for (int i = 0; i < (ARRAY_SIZE(sensorslist)); i++)
   {
      DynamicJsonDocument foc(capacity2);
      //Serial.println(sensorslist[i]());
      DeserializationError err = deserializeJson(foc, sensorslist[i]());
      if (err)
      {
         //Serial.print(F("deserializeJson() failed with code "));  ///adding "Memory" to Serial.Print
         //Serial.println(err.c_str());
      }
      JsonObject obj = foc.as<JsonObject>();
      for (JsonPair value : obj)
      {
         arr[value.key()] = (value.value());
      }
   }
   //const size_t size = measureJson(json);
   ///debugPrint("JSONsize: "+size);
   // Serial.print("JSONsize: ");
   // Serial.println(size);
   serializeJson(json, sensor);

   if ((millis() - lastWrite) >= SDwrite_MILLIS)
   {
      //Serial.println(sensor);
      Serial.println("SensorRead");
      lastWrite = millis();
   }
   if ((millis() - lastPost) >= Post_MILLIS)
   {
      // char sensetopic[80];
      // strcpy (sensetopic,"SHoPSense/");
      // strcpy (sensetopic,"SHoPSense_Gamma");
      // strcpy (sensetopic,"/sensor");
      if (client.connected())
      {
         if (client.publish("SHoPSense/sensor", sensor))
         {
            Serial.print("MQTT Publish success");
            Serial.println(sensor);
            String devicetopic = "SHoPSense/sensor/" + device_name; 
            client.publish(devicetopic.c_str(), sensor);
         }
         else
         {
            Serial.println("MQTT Publish Fail :( ");
         }
      }
      lastPost = millis();
   }
}

void Jsontele()
{
   const size_t capacity2 = JSON_OBJECT_SIZE(11);
   DynamicJsonDocument foc(capacity2);
   //zeroWindAdjustment = readDouble(10); // read zerowindadjustment
   //JsonObject object = doc.to<JsonVariant>();

   long time = now();
   foc["timestamp"] = time;
   foc["device_id"] = WiFi.macAddress();
   foc["RSSI"] = WiFi.RSSI();
   foc["fw_ver"] = version;
   foc["zerowind"] = zeroWindAdjustment;
   //foc["eco2_base"] = eco2_base;
   //foc["tvoc_base"] = tvoc_base;

   foc["name"] = device_name;

   //Serial.print("SizeTele:");
   //Serial.print(measureJson(foc));
   //Serial.println();

   ////publish json string using interval

   if ((millis() - TlastPost) >= TPost_MILLIS)
   {
      serializeJson(foc, Tele2);
      if (client.connected())
      {
         String teletopic = "SHoPSense/tele/";
         teletopic += WiFi.macAddress() + "/STATE";
         client.publish(teletopic.c_str(), Tele2);
         //Serial.println("*********");
         //Serial.println(Tele2);
         //Serial.println();
         TlastPost = millis();
      }
   }
}
void connectToWiFi(const char *ssid, const char *pwd)
{
   int ledState = 0;
   printLine();
   Serial.println("Connecting to WiFi network: " + String(ssid));

   WiFi.begin(ssid, pwd); //connecting to Wi-Fi using network name and password from cred.h
   if (WiFi.status() == WL_CONNECT_FAILED){
      Serial.println("WiFi CONNECT_FAILED!");
   }
   while (WiFi.status() != WL_CONNECTED)
   {
      // Blink LED while we're connecting:
      digitalWrite(LED_BUILTIN, ledState);
      ledState = (ledState + 1) % 2; // Flip ledState
      delay(500);
      Serial.println(WiFi.status());
      Serial.print(".");
   }
   Serial.println();
   Serial.println("WiFi connected!");
   Serial.print("IP address: ");
   Serial.println(WiFi.localIP());
}

/*-------------------------------- NTP code --------------------------------*/

const int timeZone = 0; // UTC
//const int timeZone = -5;  // Eastern Standard Time (USA)
//const int timeZone = -4;  // Eastern Daylight Time (USA)
//const int timeZone = -8;  // Pacific Standard Time (USA)
//const int timeZone = -7;  // Pacific Daylight Time (USA)

// NTP Servers:
static const char ntpServerName[] = "us.pool.ntp.org";
//static const char ntpServerName[] = "time.nist.gov";
//static const char ntpServerName[] = "time-a.timefreq.bldrdoc.gov";
//static const char ntpServerName[] = "time-b.timefreq.bldrdoc.gov";
//static const char ntpServerName[] = "time-c.timefreq.bldrdoc.gov";

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48;     // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

void rebootfix(){
   Serial.println("Restarting...");
   ESP.restart();
}

// Specify IP address or hostname
const char* remote_ip = "www.google.com";
//const IPAddress remote_ip(192, 168, 0, 1);
int pingResult;

void pingcheck(){
   Serial.print("Pinging ");
   Serial.print(remote_ip);
   Serial.print(": ");

  if(Ping.ping(remote_ip)) {
    Serial.println("Success!!");
  } else {
    Serial.println("Error :(");
  }
}

time_t getNtpTime()
{
   IPAddress ntpServerIP; // NTP server's ip address
   while (Udp.parsePacket() > 0)
      ; // discard any previously received packets
   Serial.println("Transmit NTP Request");
   // get a random server from the pool
   WiFi.hostByName(ntpServerName, ntpServerIP);
   Serial.print(ntpServerName);
   Serial.print(": ");
   Serial.println(ntpServerIP); /// Optional print line
   sendNTPpacket(ntpServerIP);
   uint32_t beginWait = millis();
   while (millis() - beginWait < 1500)
   {
      int size = Udp.parsePacket();
      if (size >= NTP_PACKET_SIZE)
      {
         Serial.println("Receive NTP Response");
         Udp.read(packetBuffer, NTP_PACKET_SIZE); // read packet into the buffer
         unsigned long secsSince1900;
         // convert four bytes starting at location 40 to a long integer
         secsSince1900 = (unsigned long)packetBuffer[40] << 24;
         secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
         secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
         secsSince1900 |= (unsigned long)packetBuffer[43];
         return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
      }
   }
   Serial.println("No NTP Response :-(");
   return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
   // set all bytes in the buffer to 0
   memset(packetBuffer, 0, NTP_PACKET_SIZE);
   // Initialize values needed to form NTP request
   // (see URL above for details on the packets)
   packetBuffer[0] = 0b11100011; // LI, Version, Mode
   packetBuffer[1] = 0;          // Stratum, or type of clock
   packetBuffer[2] = 6;          // Polling Interval
   packetBuffer[3] = 0xEC;       // Peer Clock Precision
   // 8 bytes of zero for Root Delay & Root Dispersion
   packetBuffer[12] = 49;
   packetBuffer[13] = 0x4E;
   packetBuffer[14] = 49;
   packetBuffer[15] = 52;
   // all NTP fields have been given values, now
   // you can send a packet requesting a timestamp:
   Udp.beginPacket(address, 123); //NTP requests are to port 123
   Udp.write(packetBuffer, NTP_PACKET_SIZE);
   Udp.endPacket();
}



// callback for MQTT publishes to CMND/Macaddreess
void callback(char *topic, byte *payload, unsigned int length)
{ //the topic, message, and length of message recieved
   Serial.print("Message arrived [");
   Serial.println(topic);
   char cmnd_Topic[24] = "cmnd/";
   char macid[18] = {0};
   //Serial.print("macid: ");
   //Serial.println(macid);
   strcpy(macid,WiFi.macAddress().c_str());
   strcat(cmnd_Topic,macid);
   //Serial.print("macid: ");
   //Serial.println(macid);
   
   //cmnd_Topic += macid;
   //valid topics
   //char * control_topic = "cmnd/C4:DD:57:9C:C9:88/displaycontrol";
   // char * control_topic1 = "cmnd/C4:DD:57:9C:C9:88/LEDcontrol";
   // char *interval_topic = "cmnd/C4:DD:57:9C:C9:88/serialinterval";
   // char *interval_topic1 = "cmnd/C4:DD:57:9C:C9:88/postinterval";
   // char *interval_topic2 = "cmnd/C4:DD:57:9C:C9:88/readinterval";
   // char *interval_topic3 = "cmnd/C4:DD:57:9C:C9:88/teleinterval";
   // char *interval_topic4 = "cmnd/C4:DD:57:9C:C9:88/displayinterval";
   // char *calibration_topic = "cmnd/C4:DD:57:9C:C9:88/WScalibration";
   // char *calibration_topic1 = "cmnd/C4:DD:57:9C:C9:88/co2calibration";
   // char *control_topic = "cmnd/C4:DD:57:9C:C9:88/displaycontrol";
   
   String ctopic = "/displaycontrol";
   String ctopic1 = "/LEDcontrol";
   String int_topic = "/serialinterval";
   String int_topic1 = "/postinterval";
   String int_topic2 = "/readinterval";
   String int_topic3 = "/teleinterval";
   String int_topic4 = "/displayinterval";
   String cal_topic = "/WScalibration";
   String cal_topic1 = "/co2calibration";

   char control_topic[50] = {};
   char control_topic1[50] = {};
   char interval_topic[50] = {};
   char interval_topic1[50] = {};
   char interval_topic2[50] = {};
   char interval_topic3[50] = {};
   char interval_topic4[50] = {};
   char calibration_topic[50] = {};
   char calibration_topic1[50] = {};

   strcat(control_topic,cmnd_Topic);
   strcat(control_topic1, cmnd_Topic);
   strcat(interval_topic,cmnd_Topic);
   strcat(interval_topic1,cmnd_Topic);
   strcat(interval_topic2,cmnd_Topic);
   strcat(interval_topic3,cmnd_Topic);
   strcat(interval_topic4,cmnd_Topic);
   strcat(calibration_topic,cmnd_Topic);
   strcat(calibration_topic1,cmnd_Topic);


   strcat(control_topic,ctopic.c_str());
   strcat(control_topic1, ctopic1.c_str());
   strcat(interval_topic,int_topic.c_str());
   strcat(interval_topic1,int_topic1.c_str());
   strcat(interval_topic2,int_topic2.c_str());
   strcat(interval_topic3,int_topic3.c_str());
   strcat(interval_topic4,int_topic4.c_str());
   strcat(calibration_topic,cal_topic.c_str());
   strcat(calibration_topic1,cal_topic1.c_str());

   if (strcmp(topic, interval_topic) == 0)
   {
      serialinterval(convert_Payload(payload, length)); //serial print interval
   }
   else if (strcmp(topic, interval_topic1) == 0)
   {
      postinterval(convert_Payload(payload, length)); //MQTT post interval
   }
   else if (strcmp(topic, interval_topic2) == 0)
   {
      readinterval(convert_Payload(payload, length)); //reading the sensors interval
   }
   else if (strcmp(topic, interval_topic3) == 0)
   {
      teleinterval(convert_Payload(payload, length)); //telemetry publish interval
   }
   else if (strcmp(topic, interval_topic4) == 0)
   {
      displayinterval(convert_Payload(payload, length)); //display refresh interval
   }
   else if (strcmp(topic, calibration_topic) == 0)
   {
      WScalibration(convert_Payload(payload, length)); //wind speed callibration value
   }
   else if (strcmp(topic, calibration_topic1) == 0)
   {
      co2calibration(convert_Payload(payload, length)); //co2 callibration value
   }
   else if (strcmp(topic, control_topic) == 0)
   {
      //displaycontrol(convert_Payload(payload,length));
   }
   else if (strcmp(topic, control_topic1) == 0)
   {
      //LEDcontrol(convert_Payload(payload,length));
   }
   else
   {
      Serial.print("--not a valid topic");
   }
   Serial.print("] ");
   for (int i = 0; i < length; i++)
   {
      Serial.print((char)payload[i]);
   }
   Serial.println();

}


/* Reconnect to MQTT broker */
bool reconnectMQTT()
{
   // Keep trying until we're reconnected
   while (!client.connected())
   {
      Serial.print("Attempting MQTT connection...");
      // Create a random client ID
      String clientId = device_name+"_";
      
      clientId += WiFi.macAddress();
      // String clientId = WiFi.macAddress();

      String LWT_TOPIC = "SHoPSense/LWT/"+clientId;
      // Attempt to connect
      //if (client.connect( "TEST",MQTT_USER, MQTT_PASS,LWT_TOPIC.c_str(),0,true,"Offline")) //
      //
      if (client.connect(clientId.c_str(),MQTT_USER, MQTT_PASS,LWT_TOPIC.c_str(),0,true,"Offline")) //clientId.c_str(),
      {
         Serial.println("connected");
         // Once connected, publish an announcement...
         String teletopic = "tele";
         teletopic += "/SHoPSense/"+WiFi.macAddress() ;
         //client.publish("tele/SHoPSense/LWT", "Online");
         client.publish(LWT_TOPIC.c_str(), "Online",false);
         // ... and resubscribe
         String cmnd_Topic = "cmnd/";
         cmnd_Topic += WiFi.macAddress() + "/#";
         Serial.println(cmnd_Topic);
         client.subscribe(cmnd_Topic.c_str());
         client.setCallback(callback);
      }
      else
      {
         Serial.print("failed, rc=");
         Serial.print(client.state());
         Serial.println(" try again in 5 seconds");
         // Wait 5 seconds before retrying
         return 0;
      }
   }
}

void setup()
{
   Serial.begin(115200);
   if (!SD.begin(SD_CS_PIN))
   {
      Serial.println("SD initialization failed!");
      sdwrite = 0;
   }
   else
   {
      Serial.println("SD initialization done.");
      sdwrite = 1;
   }
   if (!bme680.begin())
   {
      //Particle.publish("Log", "Could not find a valid BME680 sensor, check wiring!");
      Serial.println("Could not find a valid BME680 sensor, check wiring!");
      sensor0 = 0;
   }
   else
   {
      //Particle.publish("Log", "BME680 success =)");
      Serial.println("BME680 success =)");
      sensor0 = 1;
      // Set up oversampling and filter initialization
      bme680.setTemperatureOversampling(BME680_OS_8X);
      bme680.setHumidityOversampling(BME680_OS_2X);
      bme680.setPressureOversampling(BME680_OS_4X);
      bme680.setIIRFilterSize(BME680_FILTER_SIZE_3);
      //bme.setGasHeater(320, 150); // 320*C for 150 ms
   }
   delay(100);
   if (!sgp30.begin())
   {
      // Particle.publish("Log", "Could not find a valid SGP30 VOC sensor, check wiring!");
      Serial.println("Could not find a valid SGP30 VOC sensor, check wiring!");
      sensor2 = 0;
      sgp30.IAQinit();
      //restoreBaseline();
      //sensor2 = 0;
   }
   else
   {
      sensor2 = 1;
      sgp30.IAQinit();
      //restoreBaseline();
      //Particle.publish("Log", "SGP30 VOC success =)");
      Serial.println("SGP30 VOC success =)");
   }
   delay(100);
   // If you have a baseline measurement from before you can assign it to start, to 'self-calibrate'
   //sgp.setIAQBaseline(0x8E68, 0x8F41);  // Will vary for each sensor!
   // if (!bme.begin(0x76)) {
   if (!bme280.begin())
   {
      //Particle.publish("Log", "Could not find a valid BME280 sensor, check wiring!");
      Serial.println("Could not find a valid BME280 sensor, check wiring!");
      sensor1 = 0;
   }
   else
   {
      sensor1 = 1;
      //Particle.publish("Log", "BME280 success =)");
      Serial.println("BME280 success =)");
   }
   delay(100);
   if (!aht.begin())
   { //starting the sensor
      //Particle.publish("Log", "Could not find a valid AHTX0 sensor, check wiring!");
      Serial.println("Could not find a valid AHTX0 sensor, check wiring!");
      sensor15 = 0;
   }
   else
   {
      sensor15 = 1;
      //Particle.publish("Log", "AHTX0 success =)");
      Serial.println("AHTX0 success =)");
   }
   delay(100);
   if (!thSi.begin())
   {
      //Particle.publish("Log", "Could not find a valid Si7021 sensor, check wiring!");
      Serial.println("Could not find a valid Si7021 sensor, check wiring!");
      sensor11 = 0;
   }
   else
   {
      sensor11 = 1;
      //Particle.publish("Log", "Si7021 success =)");
      Serial.println("Si7021 success =)");
   }
   delay(100);
   if (!co2SCD30.begin())
   { //This will cause readings to occur every two seconds
      //Particle.publish("Log", "Could not find a valid SCD30 CO2 sensor, check wiring!");
      Serial.println("Could not find a valid SCD30 CO2 sensor, check wiring!");
      sensor3 = 0;
   }
   else
   {
      sensor3 = 1;
      co2SCD30.getAutoSelfCalibration();
      //Particle.publish("Log", "SCD30 CO2 success =)");
      Serial.println("SCD30 CO2 success =)");
   }
   if (!movementSensor.begin(Wire,I2C_SPEED_STANDARD,0x64))
   { 
      Serial.println("Could not find a valid AK975X sensor(0x67), check wiring!");
      sensor7 = 0;
   }
   else
   {
      sensor7 = 1;
      movementSensor.setMode(AK975X_MODE_0);
      Serial.println("AK975X#1 success =)");
   }
   /*
   if (!movementSensorFr.begin(Wire, I2C_SPEED_FAST, 0x64))
   { 
      Serial.println("Could not find a valid 2nd AK975X sensor(0x64), check wiring!");
      sensor17 = 0;
   }
   else
   {
      sensor17 = 1;
      //Particle.publish("Log", "AK975X2 success =)");
      movementSensorFr.setMode(AK975X_MODE_0);
      Serial.println("2nd AK975X#2 success =)");
   }
   */
   // if (!pmSPS30.begin())
   // { 
   //    //Particle.publish("Log", "Could not find a valid SCD30 CO2 sensor, check wiring!");
   //    Serial.println("Could not find a valid SPS30 PM sensor, check wiring!");
   //    sensor6 = 0;
   // }
   // else
   // {
   //    sensor6 = 1;
   //    //Particle.publish("Log", "SCD30 CO2 success =)");
   //    Serial.println("SPS30 PM success =)");
   // }
   if (!tsl.begin())
   {
      //Particle.publish("Log", "No TSL2591 light sensor found ... check your wiring?");
      Serial.println("No TSL2591 light sensor found ... check your wiring?");
      sensor4 = 0;
   }
   else
   {
      //Particle.publish("Log", "Found a TSL2591 sensor");
      Serial.println("Found a TSL2591 sensor");
      //configureSensor();
      sensor4 = 1;
   }
   delay(100);
   if (! aqi.begin_I2C())
   {
      //Particle.publish("Log", "No  PM 2.5 sensor found ... check your wiring?");
      Serial.println("No PM 2.5 sensor found ... check your wiring?");
      sensor10 = 0;
   }
   else
   {
      //Particle.publish("Log", "Found a  PM 2.5 sensor sensor");
      Serial.println("Found a  PM 2.5 sensor sensor");
      sensor10 = 1;
   }
   delay(100);
   if (!uv.begin())
   {
      // Particle.publish("Log", "Could not find a valid SGP30 VOC sensor, check wiring!");
      Serial.println("Could not find a valid Si1145 sensor, check wiring!");
      sensor8 = 0;
   }
   else
   {

      //Particle.publish("Log", "Si1145 success =)");
      sensor8 = 1;
      Serial.println("Si1145 success =)");
   }
   if (!sgp40.begin())
   {
      //Particle.publish("Log", "No  PM 2.5 sensor found ... check your wiring?");
      Serial.println("No SGP40 sensor found ... check your wiring?");
      sensor9 = 0;
   }
   else
   {
      //Particle.publish("Log", "Found a  PM 2.5 sensor sensor");
      Serial.println("Found a  SGP40 sensor");
      sensor9 = 1;
   }
   if (!sht4.begin())
   {
      Serial.println("Couldn't find SHT4x");
      sensor13 = 0;
   }
   else
   {
      sensor13 = 1;
      Serial.println("Found SHT4x sensor");
      Serial.print("Serial number 0x");
      Serial.println(sht4.readSerial(), HEX);
   }
   /*
   if (!scd4x.begin(Wire))
   {
      Serial.println("Couldn't find SDC4x");
      sensor16 = 0;
   }
   else
   {
      sensor16 = 1;
      Serial.println("Found SDC4x sensor");
      //getSCD40Serial();
   }
   */
   //Wire.begin();
   scd4x.begin(Wire);
   getSCD40Serial();
   ////ADC INIT/////
   // The ADC input range (or gain) can be changed via the following
   // functions, but be careful never to exceed VDD +0.3V max, or to
   // exceed the upper and lower limits if you adjust the input range!
   // Setting these values incorrectly may destroy your ADC!
   //                                                                ADS1015  ADS1115
   //                                                                -------  -------
   ads.setGain(GAIN_TWOTHIRDS); // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
   ads1.setGain(GAIN_TWOTHIRDS); // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
   //ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
   // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
   // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
   // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
   // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
   ads.begin(); /* On mainboard (wind/Analog-in pins)*/
   ads1.begin(); /* Light/Sound module dB meter*/
   Serial.println("ADC Init");
   Serial.println("ADC1 Init");
   //zeroWindAdjustment = EEPROM.readDouble(20); // read zerowindadjustment
   Serial.print("zeroWindAdjustment Init: ");
   Serial.println(zeroWindAdjustment);
   ///WIFI/NTP INIT
   // Serial.println("TimeNTP Example");
   connectToWiFi(ssid,pass);
   // Serial.print("Connecting to ");
   // Serial.println(ssid);
   // WiFi.begin(ssid, pass);

   // while (WiFi.status() != WL_CONNECTED)
   // {
   //    delay(500);
   //    Serial.print(".WIFI CONNECTED");
   // }
   // delay(500);
   client.setServer(mqtt_server, 1883);
   uint16_t mqttbuffersize = 1000;
   client.setBufferSize(mqttbuffersize);

   setSyncProvider(getNtpTime);
   setSyncInterval(300);
   sec_espClient.setInsecure();
   delay(100);
   Serial.println("What's my name? - "+requestURL + DeviceID);  // Add this line to print the URL
   http.begin(sec_espClient,requestURL+DeviceID);
   http.addHeader("Content-Type", "application/json");
   String httpRequestData = DeviceID;
   //Use HTTP GET request
   int httpResponseCode = http.GET();
   delay(100);
   if (httpResponseCode == 200)
   {
         //Response from server
      device_name = http.getString();  
      Serial.print("HTTP_CODE_OK: ");
      Serial.println("My Name is: "+device_name);
      //Serial.println(httpRequestData);
   }
   else
   {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
      device_name ="SHoPSense";  

   }
   

   //Close connection 
   http.end();

}

void loop()
{
   if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi-D/C");
      Serial.println("Re-connecting to WiFi network: " + String(ssid));
      WiFi.reconnect();
   }
   if (!client.connected())
   {
      reconnectMQTT();
      //might get stuck in a loop if we can't connect to internet or a broker
   }
   client.loop();
   if (millis() - lastRead > Read_MILLIS)
   {                //read interval
      JsonSensor(); // Create  JSON string for publish
      lastRead = millis();
      //delay(50);
   }
   if (millis() - lastReset > reboot_interval*60000UL) {  /////reset after x minutes
   rebootfix();
   lastReset = millis();
   }  
   /*
   if (millis() - lastPing > ping_interval*60000UL) {  /////reset after x minutes
   //rebootfix();
   pingcheck();
   lastPing = millis();
   }    
   */
   Jsontele();
}
