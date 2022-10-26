#include "shared.h"

#pragma region Initialization

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

void initWireless();
{
  printOLED(0, 0, "Connecting to:", 1);
  printOLED(0, 10, WLAN_SSID, 2);
  display.display();
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    display.setCursor(i, 20);
    display.write(".");
    display.display();
    i++;
  }
  display.clearDisplay();
  printOLED(0, 0, "WiFi connected!", 2);
  delay(1000);
}

void setup()
{
  initRGB();
  ledRed();
  Serial.begin(9600);
  dht.begin();          
  initDisplay();
  display.clearDisplay();
  delay(10);
  initWireless();
  mqtt.subscribe(&text);
}
#pragma endregion

#pragma region OLED
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
#pragma endregion

#pragma region MQTT pub/sub
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
#pragma endregion

#pragma region Misc functions
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
#pragma endregion

#pragma region LED Control
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
#pragma endregion

void loop() {
  MQTT_connect();
  mqttSub();
  getClimate(5000);
  mqttPub(5000);
  updateOLED(500);
}