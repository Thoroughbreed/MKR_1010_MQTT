#include <SPI.h>                  // SPI
#include <WiFiNINA.h>             // Wireless
#include <utility/wifi_drv.h>     // Builtin RGB
#include <DHT.h>                  // DHT
#include <Wire.h>                 // I2C
#include <Adafruit_GFX.h>         // OLED
#include <Adafruit_SSD1306.h>     // OLED
#include "Adafruit_MQTT.h"        // MQTT
#include "Adafruit_MQTT_Client.h" // MQTT
#include <ArduinoJson.h>          // JSON
#include <Servo.h>                // Servo
#include <NTPClient.h>            // NTP client
#include <WiFiUdp.h>              // NTP client

/************************* WiFi Access Point *********************************/
#define WLAN_SSID       "SibirienAP"
#define WLAN_PASS       "Siberia51244"

/************************* NTP cluent *********************************/
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

/************************* Brooker Setup *********************************/
#define AIO_SERVER      "mqtt3.thingspeak.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "Fw0mJzQqJBQ0JyQgACkwJig"
#define AIO_CLIENTID    "Fw0mJzQqJBQ0JyQgACkwJig"
#define AIO_KEY         "JnhZVIxT2gutEAGtCBLSEFMO"

/************************* Servo *********************************/
#define SERVO_PIN       9
Servo servoOne;

/************************* Var & const *********************************/
String messageToDisplay;    // Message for OLED
long delayOLED;
long delayPub;
long delayClimate;
long delayTime;
WiFiClient client;          // Wireless client
//WiFiClientSecure client;  // SSL
int servoPos = 0;           
uint32_t t = 0;             // Temp from DHT11
uint32_t h = 0;             // Humidity from DHT11

/************************* Devices/classes *********************************/
// DHT
#define DHT_PIN 6
#define DHTTYPE DHT11
DHT dht(DHT_PIN, DHTTYPE);

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// MQTT
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_CLIENTID, AIO_USERNAME, AIO_KEY);
/****************************** Feeds ***************************************/
// // PUB
// Adafruit_MQTT_Publish pub = Adafruit_MQTT_Publish(&mqtt, "hackerman/test");
// // Adafruit_MQTT_Publish humid = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/climate/humidity");

// // SUB
// Adafruit_MQTT_Subscribe sub = Adafruit_MQTT_Subscribe(&mqtt, "hackerman/test");

// PUB
Adafruit_MQTT_Publish pub = Adafruit_MQTT_Publish(&mqtt, "channels/1910465/publish");
// Adafruit_MQTT_Publish humid = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/climate/humidity");

// SUB
Adafruit_MQTT_Subscribe sub = Adafruit_MQTT_Subscribe(&mqtt, "channels/1910465/subscribe");

/************************* Func prototyping *********************************/
void MQTT_connect();
void mqttSub();
void mqttPub(int interval);
void mqttPubAll(uint32_t value, Adafruit_MQTT_Publish topic);
void mqqtSubParse(const char*);
void getClimate(int interval);
void getTime();
void runServo(int deg);
void serialLog(const char*);
void initServo();
void initDisplay();
void initWireless();
void initDateTime();
void initRGB();
void printOLED(int x, int y, String text, int textSize = 1);
void updateOLED(int interval);
void flashWhite(int interval);