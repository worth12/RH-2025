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
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiUDP.h>
#include "ArduinoJson.h"



// If your PM2.5 is UART only, for UNO and others (without hardware serial) 
// we must use software serial...
// pin #2 is IN from sensor (TX pin on sensor), leave pin #3 disconnected
// comment these two lines if using hardware serial
//#include <SoftwareSerial.h>
//SoftwareSerial pmSerial(2, 3);

Adafruit_PM25AQI aqi = Adafruit_PM25AQI();
// SensirionI2CScd4x scd4x;
Adafruit_SGP40 sgp40;
Adafruit_BME280 bme280;

double temperature280;
double relativeHumidity280;
double pressurekPa280;
double Altitude280;
double seaLevelPressure;
double Tvoc_indx;

int interval = 5; // seconds for MQTT publish interval
#define Post_MILLIS (interval * 1000)
unsigned long lastPost = 0; ///seting 0 on init will post on first boot

WiFiClient espClient;
PubSubClient client(espClient);

const char* MQTT_USER = "";
const char* MQTT_PASS = "";
// const char* mqtt_server = "34.201.118.174";
const char* mqtt_server = "10.151.99.211";

// I2C address of the Mega (slave)
#define MEGA_SLAVE_ADDR 0x10

///send to arduino over i2c
void sendToMotor(String data){
  if (client.connected())
  {
    client.publish("rover/tele", data.c_str());
  }

  // Read a line (until newline or timeout)
  // String input = Serial.readStringUntil('\n');
  // input.trim(); // Remove trailing \r or whitespace

  // Convert that string to a float (distance in meters)
  float distanceM = data.toFloat();

  Serial.print("Sending distance (m): ");
  Serial.println(distanceM);

  // Send the float over I2C (4 bytes on most MCUs)
  Wire.beginTransmission(MEGA_SLAVE_ADDR);
  Wire.write((const uint8_t*)&distanceM, sizeof(distanceM));
  Wire.endTransmission();

  if (client.connected())
  {
    client.publish("rover/tele", "sent");
  }
}


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

void makeJson(String data){
  Serial.println(data);
}

void connectToWiFi(const char *ssid, const char *pwd)
{

  Serial.println("Connecting to WiFi network: " + String(ssid));
  WiFi.begin(ssid, pwd); //connecting to Wi-Fi using network name and password from cred.h
  if (WiFi.status() == WL_CONNECT_FAILED){
    Serial.println("WiFi CONNECT_FAILED!");
  }
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println(WiFi.status());
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
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

void sendDistance(String distance){
  //send distance to rover
  Serial.println("Send Distance:" + distance);
  if (distance.length() > 0)
  {
    sendToMotor(distance);
  }



}

void sendStick(String stick){
  //send stick to rover
  Serial.println("Send Stick:" + stick);
  if (stick.length() > 0)
  {
    sendToMotor("S " + stick);
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{ //the topic, message, and length of message recieved
    Serial.print("Message arrived [");
    Serial.println(topic);
    Serial.print("] ");
    if (strcmp(topic, "rover/distance") == 0)
    {
      sendDistance(convert_Payload(payload, length));
    }
    else if (strcmp(topic, "rover/stick") == 0)
    {
      sendStick(convert_Payload(payload, length));
      
    }
    else
    {
      Serial.println("Unknown Topic");
    }
    Serial.println();
}




/* Reconnect to MQTT broker */
void reconnectMQTT()
{
   // Keep trying until we're reconnected
   while (!client.connected())
   {
      Serial.print("Attempting MQTT connection...");
      // Create a random client ID
      String clientId = "BOTESP";
      
      // clientId += WiFi.macAddress();
      // String clientId = WiFi.macAddress();

      String LWT_TOPIC = "BOTESP/LWT/"+clientId;
      // Attempt to connect
      //if (client.connect( "TEST",MQTT_USER, MQTT_PASS,LWT_TOPIC.c_str(),0,true,"Offline")) //
      //
      if (client.connect(clientId.c_str(),MQTT_USER, MQTT_PASS)) //clientId.c_str(),
      {
        Serial.println("connected");
         // Once connected, publish an announcement...
         String teletopic = "tele";
         teletopic += "/BOTESP/";
         //client.publish("tele/SHoPSense/LWT", "Online");
         client.publish(LWT_TOPIC.c_str(), "Online",false);
        //... and resubscribe
        client.setCallback(callback);
        client.subscribe("rover/distance");
        client.subscribe("rover/stick");
        

      }
      else
      {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(5000);
      }
   }
}




void setup() {
  // Wait for serial monitor to open
  
  Serial.begin(115200);
  // while (!Serial) delay(10);

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
    
  }
  Serial.println("PM25 found!");

  // getSCD40Serial();
  sgp40.begin();
  bme280.begin();

  Wire.begin();

  uint16_t error;
  char errorMessage[256];

  // scd4x.begin(Wire);

  // // stop potentially previously started measurement
  // error = scd4x.stopPeriodicMeasurement();
  // if (error) {
  //     Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
  //     errorToString(error, errorMessage, 256);
  //     Serial.println(errorMessage);
  // }

  // uint16_t serial0;
  // uint16_t serial1;
  // uint16_t serial2;
  // error = scd4x.getSerialNumber(serial0, serial1, serial2);
  // if (error) {
  //     Serial.print("Error trying to execute getSerialNumber(): ");
  //     errorToString(error, errorMessage, 256);
  //     Serial.println(errorMessage);
  // } else {
  //     printSerialNumber(serial0, serial1, serial2);
  // }

  // // Start Measurement
  // error = scd4x.startPeriodicMeasurement();
  // if (error) {
  //     Serial.print("Error trying to execute startPeriodicMeasurement(): ");
  //     errorToString(error, errorMessage, 256);
  //     Serial.println(errorMessage);
  // }

  Serial.println("Waiting for first measurement... (5 sec)");
  connectToWiFi("FIVE GUYS", "Airplanes");
  client.setServer(mqtt_server, 1883);
  uint16_t mqttbuffersize = 1000;
  client.setBufferSize(mqttbuffersize);  
}

void loop() {
  if (!client.connected())
  {
    reconnectMQTT();
    //might get stuck in a loop if we can't connect to internet or a broker
  }
  client.loop();
  char * combined_data;
  PM25_AQI_Data data;

  // if (! aqi.read(&data)) {
  //   Serial.println("Could not read from AQI");
    
  //   delay(500);  // try again in a bit!
  //   return;
  // }

  // Serial.println(F("---------------------------------------"));
  // Serial.println("Data from PM Sensor");
  // Serial.println(F("---------------------------------------"));

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
  // Serial.println(F("AQI"));
  // Serial.print(F("PM2.5 AQI US: ")); Serial.print(data.aqi_pm25_us);
  // Serial.print(F("\tPM10  AQI US: ")); Serial.println(data.aqi_pm100_us);
  // //  Serial.print(F("PM2.5 AQI China: ")); Serial.print(data.aqi_pm25_china);
  // //  Serial.print(F("\tPM10  AQI China: ")); Serial.println(data.aqi_pm100_china);
  // Serial.println(F("---------------------------------------"));


  // Serial.println(F("---------------------------------------"));
  // Serial.println(F("Data from BME280 Sensor"));
  // Serial.println(F("---------------------------------------"));
  temperature280 = bme280.readTemperature();
  pressurekPa280 = bme280.readPressure();
  relativeHumidity280 = bme280.readHumidity();
  //Serial.print(F("Temprature: ")); Serial.println(temperature280); 
  //Serial.print(F("Pressure: ")); Serial.println(pressurekPa280); 
  //Serial.print(F("Humidity: ")); Serial.println(relativeHumidity280);

  //Altitude280 = bmp.readAltitude(seaLevelPressure);


  // Serial.println(F("---------------------------------------"));
  // Serial.println(F("Data from TVOC Sensor"));
  // Serial.println(F("---------------------------------------"));
  Tvoc_indx = sgp40.measureVocIndex(temperature280, relativeHumidity280);
  // Serial.print(F("Tvoc_indx")); 
  // Serial.println(Tvoc_indx);
  char sensor[1000];
  const size_t capacity2 = JSON_OBJECT_SIZE(50);
  DynamicJsonDocument json(capacity2);

  json["temperature"] = temperature280;
  json["humidity"] = relativeHumidity280;
  json["pressure"] = pressurekPa280;
  json["pm1_0"] = data.particles_10um;
  json["pm2_5"] = data.particles_25um;
  json["pm1_0Status"] = "Fine";
  json["pm2_5Status"] = "Fine";
  json["Tvoc_indx"] = Tvoc_indx;
  serializeJson(json, sensor);
     if ((millis() - lastPost) >= Post_MILLIS)
   {
      // char sensetopic[80];
      // strcpy (sensetopic,"SHoPSense/");
      // strcpy (sensetopic,"SHoPSense_Gamma");
      // strcpy (sensetopic,"/sensor");
      if (client.connected())
      {
         if (client.publish("all_sensor", sensor))
         {
            Serial.print("MQTT Publish success");
            Serial.println(sensor);
            String devicetopic = "all_sensor"; 
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

