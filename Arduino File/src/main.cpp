#include <Arduino.h>
#include <Wire.h>
#include <SensirionCore.h>
#include <SPI.h>
#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_PM25AQI.h"
#include <SensirionI2CScd4x.h>
#include "Adafruit_SGP40.h"
#include "Adafruit_BME280.h"




// If your PM2.5 is UART only, for UNO and others (without hardware serial) 
// we must use software serial...
// pin #2 is IN from sensor (TX pin on sensor), leave pin #3 disconnected
// comment these two lines if using hardware serial
//#include <SoftwareSerial.h>
//SoftwareSerial pmSerial(2, 3);

Adafruit_PM25AQI aqi = Adafruit_PM25AQI();
SensirionI2CScd4x scd4x;
Adafruit_SGP40 sgp40;
Adafruit_BME280 bme280;

double temperature280;
double relativeHumidity280;
double pressurekPa280;
double Altitude280;
double seaLevelPressure;
double Tvoc_indx;


// void getSCD40Serial(){
//    uint16_t error;
//    char errorMessage[256];
//    // stop potentially previously started measurement
//    error = scd4x.stopPeriodicMeasurement();
//    if (error) {
//       Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
//       errorToString(error, errorMessage, 256);
//       Serial.println(errorMessage);
//    }

//    uint16_t serial0;
//    uint16_t serial1;
//    uint16_t serial2;
//    error = scd4x.getSerialNumber(serial0, serial1, serial2);
//    if (error) {
//       Serial.print("Error trying to execute getSerialNumber(): ");
//       errorToString(error, errorMessage, 256);
//       Serial.println(errorMessage);
//       sensor16 = 0;
//    } else {
//       printSerialNumber(serial0, serial1, serial2);
//       sensor16 = 1;
//    }

//    // Start Measurement
//    error = scd4x.startPeriodicMeasurement();
//    if (error) {
//       Serial.print("Error trying to execute startPeriodicMeasurement(): ");
//       errorToString(error, errorMessage, 256);
//       Serial.println(errorMessage);
//    }

//    Serial.println("SCD4X INIT");

// }

// String read_SGP40()
// {
//    String SGP40values;
//    if (sensor9)
//    {
//       float temp = co2SCD30.getTemperature(); // Read SCD30 Temp/Humidity
//       float hum = co2SCD30.getHumidity();     // Read SCD30 Temp/Humidity
//       Tvoc_indx = sgp40.measureVocIndex(temp, hum);
//       Tvoc_raw = sgp40.measureRaw(temp, hum);
//       output out = {roundoff(Tvoc_indx) / 100.00, roundoff(Tvoc_raw) / 100.00};
//       uint bufferSize = ARRAY_SIZE(out);
//       //Serial.print("bufferSize: ");
//       //Serial.println(bufferSize);
//       //SGP40values = reSerializeIt(SGP40keys,out,bufferSize);
//       String SGP40values = reSerializeIt(SGP40keys, out, bufferSize);
//       //Serial.print(SGP40values);
//       return SGP40values;
//    }
//    else if (!sensor9)
//    {
//       Tvoc_raw = -1;
//       Tvoc_indx = -1;
//       String SGP40values =  "";
//       return SGP40values;
//    }

//    return SGP40values;
// }

// bool sensor16 = 0; //Adafruit_SCD40
// void getSCD40Serial(){
//    uint16_t error;
//    char errorMessage[256];
//    // stop potentially previously started measurement
//    error = scd4x.stopPeriodicMeasurement();
//    if (error) {
//       Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
//       errorToString(error, errorMessage, 256);
//       Serial.println(errorMessage);
//    }

//    uint16_t serial0;
//    uint16_t serial1;
//    uint16_t serial2;
//    error = scd4x.getSerialNumber(serial0, serial1, serial2);
//    if (error) {
//       Serial.print("Error trying to execute getSerialNumber(): ");
//       errorToString(error, errorMessage, 256);
//       Serial.println(errorMessage);
//       sensor16 = 0;
//    } else {
//       printSerialNumber(serial0, serial1, serial2);
//       sensor16 = 1;
//    }

//    // Start Measurement
//    error = scd4x.startPeriodicMeasurement();
//    if (error) {
//       Serial.print("Error trying to execute startPeriodicMeasurement(): ");
//       errorToString(error, errorMessage, 256);
//       Serial.println(errorMessage);
//    }

//    Serial.println("SCD4X INIT");
// }

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

void setup() {
  // Wait for serial monitor to open
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("Adafruit PMSA003I Air Quality Sensor");

  // Wait three seconds for sensor to boot up!
  delay(3000);

  // If using serial, initialize it and set baudrate before starting!
  // Uncomment one of the following
  //Serial1.begin(9600);
  //pmSerial.begin(9600);

  // There are 3 options for connectivity!
  if (! aqi.begin_I2C()) {      // connect to the sensor over I2C
  //if (! aqi.begin_UART(&Serial1)) { // connect to the sensor over hardware serial
  //if (! aqi.begin_UART(&pmSerial)) { // connect to the sensor over software serial 
    Serial.println("Could not find PM 2.5 sensor!");
    while (1) delay(10);
  }
  Serial.println("PM25 found!");

  // getSCD40Serial();
  sgp40.begin();
  bme280.begin();

  Wire.begin();

  uint16_t error;
  char errorMessage[256];

  scd4x.begin(Wire);

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
  } else {
      printSerialNumber(serial0, serial1, serial2);
  }

  // Start Measurement
  error = scd4x.startPeriodicMeasurement();
  if (error) {
      Serial.print("Error trying to execute startPeriodicMeasurement(): ");
      errorToString(error, errorMessage, 256);
      Serial.println(errorMessage);
  }

  Serial.println("Waiting for first measurement... (5 sec)");

}

void loop() {
  char * combined_data;
  PM25_AQI_Data data;

  if (! aqi.read(&data)) {
    Serial.println("Could not read from AQI");
    
    delay(500);  // try again in a bit!
    return;
  }

  Serial.println(F("---------------------------------------"));
  Serial.println("Data from PM Sensor");
  Serial.println(F("---------------------------------------"));

  // // Serial.println(F("Concentration Units (standard)"));
  // // Serial.print(F("PM 1.0: ")); Serial.print(data.pm10_standard);
  // // Serial.print(F("\t\tPM 2.5: ")); Serial.print(data.pm25_standard);
  // // Serial.print(F("\t\tPM 10: ")); Serial.println(data.pm100_standard);
  // // Serial.println(F("---------------------------------------"));
  // // Serial.println(F("Concentration Units (environmental)"));
  // // Serial.print(F("PM 1.0: ")); Serial.print(data.pm10_env);
  // // Serial.print(F("\t\tPM 2.5: ")); Serial.print(data.pm25_env);
  // // Serial.print(F("\t\tPM 10: ")); Serial.println(data.pm100_env);
  // // Serial.println(F("---------------------------------------"));
  // // Serial.print(F("Particles > 0.3um / 0.1L air:")); Serial.println(data.particles_03um);
  // // Serial.print(F("Particles > 0.5um / 0.1L air:")); Serial.println(data.particles_05um);
  // // Serial.print(F("Particles > 1.0um / 0.1L air:")); Serial.println(data.particles_10um);
  // // Serial.print(F("Particles > 2.5um / 0.1L air:")); Serial.println(data.particles_25um);
  // // Serial.print(F("Particles > 5.0um / 0.1L air:")); Serial.println(data.particles_50um);
  // // Serial.print(F("Particles > 10 um / 0.1L air:")); Serial.println(data.particles_100um);
  // // Serial.println(F("---------------------------------------"));
  Serial.println(F("AQI"));
  Serial.print(F("PM2.5 AQI US: ")); Serial.print(data.aqi_pm25_us);
  Serial.print(F("\tPM10  AQI US: ")); Serial.println(data.aqi_pm100_us);
//  Serial.print(F("PM2.5 AQI China: ")); Serial.print(data.aqi_pm25_china);
//  Serial.print(F("\tPM10  AQI China: ")); Serial.println(data.aqi_pm100_china);
Serial.println(F("---------------------------------------"));


Serial.println(F("---------------------------------------"));
Serial.println(F("Data from BME280 Sensor"));
Serial.println(F("---------------------------------------"));
temperature280 = bme280.readTemperature();
pressurekPa280 = bme280.readPressure();
relativeHumidity280 = bme280.readHumidity();
Serial.print(F("Temprature: ")); Serial.println(temperature280); 
Serial.print(F("Pressure: ")); Serial.println(pressurekPa280); 
Serial.print(F("Humidity: ")); Serial.println(relativeHumidity280);

//Altitude280 = bmp.readAltitude(seaLevelPressure);


Serial.println(F("---------------------------------------"));
Serial.println(F("Data from TVOC Sensor"));
Serial.println(F("---------------------------------------"));
Tvoc_indx = sgp40.measureVocIndex(temperature280, relativeHumidity280);
Serial.print(F("Tvoc_indx")); Serial.println(Tvoc_indx);

// uint16_t co2;
// uint16_t  error;
// char *errorMessage;
// float temperature = 32.85f;
// float humidity = 17.04f;
// error = scd4x.readMeasurement(co2, temperature, humidity);
// if (error) {
//     Serial.print("Error trying to execute readMeasurement(): ");
//     errorToString(error, errorMessage, 256);
//     Serial.println(errorMessage);
// } else if (co2 == 0) {
//     Serial.println("Invalid sample detected, skipping.");
// } else {
//     Serial.print("Co2:");
//     Serial.print(co2);
//     Serial.print("\t");
//     Serial.print("Temperature:");
//     Serial.print(temperature);
//     Serial.print("\t");
//     Serial.print("Humidity:");
//     Serial.println(humidity);
// }

Serial.println();
delay(1000);

o

}


