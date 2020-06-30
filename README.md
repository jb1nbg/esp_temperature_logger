# esp_temperature_logger
BME280 (Temp., Pressure, Humitidy) logger with an ESP8266 D1 Mini to a SQL database

# What you need:
- Webserver with enabled PHP
- MySQL database
- ESP8266 NodeMCU D1 Mini
- BME280 sensor module
- Arduino IDE v1.8.13 (or newer)

# Installation steps
## Arduino IDE
- install Arduino IDE
- install ESP8266 toolchain 

## Database
- create a new database "temperature" on the MySQL database
- create in "temperature" database a table with the SQL script `SQL/create_table.sql`
- add a MySQL databse user for push and request data
- set "INSERT", "SELECT" rights for this user

## Webpage
- copy all files and subfolders from `Webpage/` to your Webserver
- enable PHP on your WebServer
- create a acoount.php file and set $username and $password to your database user

example:

```PHP
<!php

$username = "user";
$password = "password"

?>
```

## ESP8266 NodeMCU D1 Mini
- connect the BME280 sensor module to the NodeMCU via i2c
- create a file account.h and setup your WiFi username,password and NTP (NetworkTimeServer IP

example:

```C
#define NTP_SERVER    "192.168.178.1"

#define WIFI_SSID     "wlan ssid"
#define WIFI_PASS     "password"
```

- program the skretch
