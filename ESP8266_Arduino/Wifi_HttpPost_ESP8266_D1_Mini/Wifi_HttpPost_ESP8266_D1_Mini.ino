/*
 * Board: LOLIN(WEMOS) D1 R2 & mini 
 * Required Libraries: BMx280MI
 * 
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#include <ESPDateTime.h>

#include <Wire.h>
#include <BMx280I2C.h>

#include "account.h"
#ifndef WIFI_SSID
#define WIFI_SSID "[Ssid]"
#endif
#ifndef WIFI_PASS
#define WIFI_PASS "[Password]"
#endif

#define DEEP_SLEEP
#define MEASURE_PERIOD_SEC    30

//Your Domain name with URL path or IP address with path
const char* serverName = "http://192.168.178.46:10000/post_temperature_data.php";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;

#ifndef NTP_SERVER
#define NTP_SERVER    "192.168.178.1"
#endif

//create a BMx280I2C object using the I2C interface with I2C Address 0x76
BMx280I2C bmx280(0x76);

void doWork();
void setup_wifi();
void setup_ntp();
bool setup_bmx280();
void pushData(char* timestamp, float temp, double pressure, float humidity);

void printLocalTime()
{
  Serial.println(DateTime.toString());
}

void printBMX280(float temp, double pressure, float humidity)
{
  Serial.print("Temperature: "); Serial.print(temp); Serial.println(" °C"); 
  Serial.print("Pressure (64 bit): "); Serial.print(pressure / 100); Serial.println(" mBar");
  Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %");
}

void setup_wifi()
{
  WiFi.begin( WIFI_SSID, WIFI_PASS );
  WiFi.setOutputPower(0); // this sets wifi to lowest power
  //WiFi.setOutputPower(20.5); // this sets wifi to highest power

  Serial.println();
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("success!");
  Serial.print("IP Address is: ");
  Serial.println(WiFi.localIP());  
}

void setup_ntp()
{
  //init and get the time
  DateTime.setServer( NTP_SERVER );
  DateTime.setTimeZone( -2 );
  DateTime.begin();
  if (!DateTime.isTimeValid()) {
    Serial.println("Failed to get time from server.");
  }
  printLocalTime();  
}

bool setup_bmx280()
{
  Wire.begin();
  
  //begin() checks the Interface, reads the sensor ID (to differentiate between BMP280 and BME280)
  //and reads compensation parameters.
  if (bmx280.begin())
  {
    if (bmx280.isBME280())
      Serial.println("sensor is a BME280");
    else
      Serial.println("sensor is a BMP280");
  
    //reset sensor to default parameters.
    bmx280.resetToDefaults();
  
    //by default sensing is disabled and must be enabled by setting a non-zero
    //oversampling setting.
    //set an oversampling setting for pressure and temperature measurements. 
    bmx280.writeOversamplingPressure(BMx280MI::OSRS_P_x16);
    bmx280.writeOversamplingTemperature(BMx280MI::OSRS_T_x16);
  
    //if sensor is a BME280, set an oversampling setting for humidity measurements.
    if (bmx280.isBME280())
      bmx280.writeOversamplingHumidity(BMx280MI::OSRS_H_x16);

    return true;
  }
  else
  {
    Serial.println("bmx280.begin() failed. check your BMx280 Interface and I2C Address."); 
    return false;   
  }
}

void deepSleep()
{
  Serial.print("Deep sleep for ");
  Serial.print(MEASURE_PERIOD_SEC, DEC);
  Serial.println(" seconds");

  ESP.deepSleep(MEASURE_PERIOD_SEC * 1000 * 1000); // in uSec
}

void setup()
{
  Serial.begin(74880);
  
  if (setup_bmx280())
  {
#ifdef DEEP_SLEEP
    doWork();
    deepSleep();
#endif
  }
  else
  {
#ifdef DEEP_SLEEP
    deepSleep();
#endif    
  }
}

void pushData(char* timestamp, float temp, double pressure, float humidity)
{
  HTTPClient http;
  
  // Your Domain name with URL path or IP address with path
  http.begin(serverName);

  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  // Data to send with HTTP POST
  String httpRequestData = "api_key=temperature_logger&timestamp=";
  httpRequestData += timestamp;
  httpRequestData += "&temperature=" + String(temp);
  httpRequestData += "&pressure=" + String(pressure / 100);
  httpRequestData += "&humidity=" + String(humidity);
  Serial.println(httpRequestData);
      
  // Send HTTP POST request
  int httpResponseCode = http.POST(httpRequestData);
 
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
    
  // Free resources
  http.end();  
}

void doWork()
{
  //start a measurement
  if (bmx280.measure())
  {
    //wait for the measurement to finish
    do
    {
      delay(100);
    } while (!bmx280.hasValue());

    float temp = bmx280.getTemperature();
    double pressure = bmx280.getPressure64();
    float humidity = 0;
    if (bmx280.isBME280())
      humidity = bmx280.getHumidity();
    
    printBMX280(temp, pressure, humidity);
          
    setup_wifi();
    setup_ntp();
  
    //Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED)
    {    
      if (!DateTime.isTimeValid()) 
      {
        Serial.println("Failed to get time from server, retry.");
        DateTime.begin();
      }
      else
      {      
        DateTimeParts p = DateTime.getParts();
        /*
        Serial.printf("%04d/%02d/%02d %02d:%02d:%02d %ld %+05d\n", p.getYear(),
                p.getMonth(), p.getMonthDay(), p.getHours(), p.getMinutes(),
                p.getSeconds(), p.getTime(), p.getTimeZone());
        */      
        char timestampString[20];
        sprintf(timestampString,"%04d-%02d-%02d %02d.%02d.%02d", p.getYear(), p.getMonth(), p.getMonthDay(), p.getHours(), p.getMinutes(), p.getSeconds());
        Serial.println(timestampString);
          
        pushData(timestampString, temp, pressure, humidity);
      }
    }
    else 
    {
      Serial.println("WiFi Disconnected");
    }
  }
  else
  {
      Serial.println("could not start measurement, is a measurement already running?");      
  }
}

void loop() 
{  
#ifndef DEEP_SLEEP
  //Send an HTTP POST request every 10 minutes
  if ((millis() - lastTime) > (MEASURE_PERIOD_SEC * 1000)) // in mSec
  {
    doWork();
    lastTime = millis();
  }
#endif
}
