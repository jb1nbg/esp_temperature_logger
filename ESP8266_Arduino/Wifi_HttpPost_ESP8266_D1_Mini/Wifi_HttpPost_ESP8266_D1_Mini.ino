/*
 * Board: LOLIN(WEMOS) D1 R2 & mini 
 * 
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

//Your Domain name with URL path or IP address with path
const char* serverName = "http://192.168.178.46:10000/post_temperature_data.php";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 60 seconds (60000)
unsigned long timerDelay = 60000;

#ifndef NTP_SERVER
#define NTP_SERVER    "192.168.178.1"
#endif

//create a BMx280I2C object using the I2C interface with I2C Address 0x76
BMx280I2C bmx280(0x76);

void printLocalTime()
{
  Serial.println(DateTime.toString());
}

void setup()
{
  delay(1000);
  Serial.begin(115200);
 
  WiFi.begin( WIFI_SSID, WIFI_PASS );

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

  //init and get the time
  DateTime.setServer( NTP_SERVER );
  DateTime.setTimeZone( -2 );
  DateTime.begin();
  if (!DateTime.isTimeValid()) {
    Serial.println("Failed to get time from server.");
  }
  printLocalTime();

  Wire.begin();

  //begin() checks the Interface, reads the sensor ID (to differentiate between BMP280 and BME280)
  //and reads compensation parameters.
  if (!bmx280.begin())
  {
    Serial.println("begin() failed. check your BMx280 Interface and I2C Address.");
    while (1);
  }

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
}

void loop() 
{  
  //Send an HTTP POST request every 10 minutes
  if ((millis() - lastTime) > timerDelay) 
  {
    //Check WiFi connection status
    if (WiFi.status()== WL_CONNECTED)
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
        char logString[20];
        sprintf(logString,"%04d-%02d-%02d %02d.%02d.%02d", p.getYear(), p.getMonth(), p.getMonthDay(), p.getHours(), p.getMinutes(), p.getSeconds());
        Serial.println(logString);

        //start a measurement
        if (bmx280.measure())
        {
          //wait for the measurement to finish
          do
          {
            delay(100);
          } while (!bmx280.hasValue());
        
          Serial.print("Pressure: "); Serial.print(bmx280.getPressure()); Serial.println(" Pa");
          Serial.print("Pressure: "); Serial.print(bmx280.getPressure() / 100); Serial.println(" mBar");
          Serial.print("Pressure (64 bit): "); Serial.print(bmx280.getPressure64()); Serial.println(" Pa");
          Serial.print("Pressure (64 bit): "); Serial.print(bmx280.getPressure64() / 100); Serial.println(" mBar");
          Serial.print("Temperature: "); Serial.print(bmx280.getTemperature()); Serial.println(" °C");
        
          if (bmx280.isBME280())
          {
            Serial.print("Humidity: "); Serial.print(bmx280.getHumidity()); Serial.println(" %");
          }
          
          HTTPClient http;
          
          // Your Domain name with URL path or IP address with path
          http.begin(serverName);
    
          // Specify content-type header
          http.addHeader("Content-Type", "application/x-www-form-urlencoded");
          // Data to send with HTTP POST
          String httpRequestData = "api_key=temperature_logger&timestamp=";
          httpRequestData += logString;
          httpRequestData += "&temperature=" + String(bmx280.getTemperature());
          httpRequestData += "&pressure=" + String(bmx280.getPressure64() / 100);
          httpRequestData += "&humidity=" + String(bmx280.getHumidity());
          Serial.println(httpRequestData);
              
          // Send HTTP POST request
          int httpResponseCode = http.POST(httpRequestData);
         
          Serial.print("HTTP Response code: ");
          Serial.println(httpResponseCode);
            
          // Free resources
          http.end();
        }
        else
        {
            Serial.println("could not start measurement, is a measurement already running?");
            return;        
        }
      }
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}
