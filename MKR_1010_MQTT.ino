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
void initRGB();
void printOLED(int x, int y, String text, int textSize = 1);
void updateOLED(int interval);
void flashWhite(int interval);

void initRGB()
{
  WiFiDrv::pinMode(25, OUTPUT);
  WiFiDrv::pinMode(26, OUTPUT);
  WiFiDrv::pinMode(27, OUTPUT);
}

void initDisplay()
{
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
	{
		Serial.println(F("SSD1306 allocation failed"));
		for(;;); // Loop forever
	}
}

void setup()
{
  Serial.begin(9600);
  dht.begin();
  initDisplay();
  initRGB();
  display.clearDisplay();
  delay(10);

  Serial.println(F("Adafruit MQTT demo"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  ledRed();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  printOLED(0, 0, "Connecting to:", 1);
  printOLED(0, 10, WLAN_SSID, 2);
  display.display();
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    display.setCursor(i, 20);
    display.write(".");
    display.display();
    i++;
  }
  Serial.println();
  display.clearDisplay();
  printOLED(0, 0, "WiFi connected!", 2);
  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
  delay(1000);
  mqtt.subscribe(&text);
}

void printOLED(int x, int y, String text, int textSize)
{
  display.setTextSize(textSize);
  display.setTextColor(WHITE);
  display.setCursor(x, y);
  display.println(text);
}

void updateOLED(int interval)
{
  if ((millis() - delayOLED) > interval)
  {
    delayOLED = millis();
    display.clearDisplay();
    printOLED(0, 0, messageToDisplay, 2);
    display.display();
  }
}

void mqttSub()
{
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription())) {
    if (subscription == &text)
    {
      Serial.print(F("Got: "));
      Serial.println((char *)text.lastread);
      messageToDisplay = (char *)text.lastread;
      Serial.println(messageToDisplay);
      flashWhite(75);
      delayClimate = millis();
    }
  }  
}

void mqttPub(int interval)
{
  if ((millis() - delayPub) > interval)
  {
    mqttPubAll(t, temp);
    mqttPubAll(h, humid);
  }
}

void mqttPubAll(uint32_t value, Adafruit_MQTT_Publish topic)
{
  delayPub = millis();
  if (! topic.publish(value))
  {
    Serial.println(F("Failed"));
  }
}

void getClimate(int interval)
{
  if ((millis() - delayClimate) > interval)
  {
    delayClimate = millis();
    t = dht.readTemperature();
    h = dht.readHumidity();
    messageToDisplay = "";
    messageToDisplay += t;
    messageToDisplay += "C  ";
    messageToDisplay += h;
    messageToDisplay += "%rH";
  }
}

void loop() {
  MQTT_connect();
  mqttSub();
  getClimate(5000);
  mqttPub(5000);
  updateOLED(500);
}

void MQTT_connect()
{
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected())
  {
    ledGreen();
    return;
  }
  ledBlue();
  display.clearDisplay();
  printOLED(0, 0, "Conecting to MQTT ...");
  display.display();  
  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       printOLED(0, 10, "Retrying MQTT conn.");
       display.display();
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  display.clearDisplay();
  printOLED(0, 0, "Conecting to MQTT ...");
  printOLED(0, 10, "MQTT connected!");
  display.display();
  Serial.println("MQTT Connected!");
  ledGreen();
  delay(1234);
}

void flashWhite(int interval)
{
  WiFiDrv::digitalWrite(25, 1);   //GREEN
  WiFiDrv::digitalWrite(26, 1);   //RED
  WiFiDrv::digitalWrite(27, 1);   //BLUE
  delay(interval);
  WiFiDrv::digitalWrite(25, 0);   //GREEN
  WiFiDrv::digitalWrite(26, 0);   //RED
  WiFiDrv::digitalWrite(27, 0);   //BLUE
  delay(interval);
  WiFiDrv::digitalWrite(25, 1);   //GREEN
  WiFiDrv::digitalWrite(26, 1);   //RED
  WiFiDrv::digitalWrite(27, 1);   //BLUE
  delay(interval);
  WiFiDrv::digitalWrite(25, 0);   //GREEN
  WiFiDrv::digitalWrite(26, 0);   //RED
  WiFiDrv::digitalWrite(27, 0);   //BLUE
  delay(interval);
  WiFiDrv::digitalWrite(25, 1);   //GREEN
  WiFiDrv::digitalWrite(26, 1);   //RED
  WiFiDrv::digitalWrite(27, 1);   //BLUE
  delay(interval);
  WiFiDrv::digitalWrite(25, 0);   //GREEN
  WiFiDrv::digitalWrite(26, 0);   //RED
  WiFiDrv::digitalWrite(27, 0);   //BLUE
  delay(interval);
}

void ledRed()
{
  WiFiDrv::digitalWrite(25, 0);   //GREEN
  WiFiDrv::digitalWrite(26, 1);   //RED
  WiFiDrv::digitalWrite(27, 0);   //BLUE
}

void ledGreen()
{
  WiFiDrv::digitalWrite(25, 1);   //GREEN
  WiFiDrv::digitalWrite(26, 0);   //RED
  WiFiDrv::digitalWrite(27, 0);   //BLUE
}

void ledBlue()
{
  WiFiDrv::digitalWrite(25, 0);   //GREEN
  WiFiDrv::digitalWrite(26, 0);   //RED
  WiFiDrv::digitalWrite(27, 1);   //BLUE
}
