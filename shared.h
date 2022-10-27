#include <SPI.h>                  // SPI
#include <WiFiNINA.h>             // Wireless
#include <utility/wifi_drv.h>     // Builtin RGB
#include <DHT.h>                  // DHT
#include <Wire.h>                 // I2C
#include <Adafruit_GFX.h>         // OLED
#include <Adafruit_SSD1306.h>     // OLED
#include "Adafruit_MQTT.h"        // MQTT
#include "Adafruit_MQTT_Client.h" // MQTT

/************************* WiFi Access Point *********************************/
#define WLAN_SSID       "SibirienAP"
#define WLAN_PASS       "Siberia51244"

/************************* Brooker Setup *********************************/
#define AIO_SERVER      "hackerman.cloud.shiftr.io"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "hackerman"
#define AIO_KEY         "NKV20rskFJWP8b7A"

/************************* Var & const *********************************/
String messageToDisplay;    // Message for OLED
long delayOLED;
long delayPub;
long delayClimate;
WiFiClient client;          // Wireless client
//WiFiClientSecure client;  // SSL
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
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
/****************************** Feeds ***************************************/
// PUB
Adafruit_MQTT_Publish temp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/climate/temperature");
Adafruit_MQTT_Publish humid = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/climate/humidity");

// SUB
Adafruit_MQTT_Subscribe text = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/text/oled");

/************************* Func prototyping *********************************/
void MQTT_connect();
void mqttSub();
void mqttPub(int interval);
void mqttPubAll(uint32_t value, Adafruit_MQTT_Publish topic);
void getClimate(int interval);
void initDisplay();
void initWireless();
void initRGB();
void printOLED(int x, int y, String text, int textSize = 1);
void updateOLED(int interval);
void flashWhite(int interval);