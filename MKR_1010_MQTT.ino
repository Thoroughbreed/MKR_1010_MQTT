#include "shared.h"

#pragma region Initialization

void initServo()
{
	servoOne.attach(SERVO_PIN);
	runServo(0);
}

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

void initWireless()
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
  printOLED(0, 0, "WiFi connected!");
  printOLED(0, 30, timeClient.getFormattedTime());
  display.display();
  delay(1000);
}

void setup()
{
  initRGB();
  ledRed();
  initServo();
  Serial.begin(9600);
  dht.begin();          
  initDisplay();
  display.clearDisplay();
  delay(10);
  timeClient.begin();
  timeClient.setTimeOffset(7200);
  initWireless();
  getTime(0);
  mqtt.subscribe(&sub);
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
    printOLED(0, 55, timeClient.getFormattedTime());
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
  printOLED(0, 30, timeClient.getFormattedTime());
  display.display();  
  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) // connect will return 0 for connected
  { 
    display.clearDisplay();
    printOLED(0, 0, "Conecting to MQTT ...");
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    printOLED(0, 10, "Retrying MQTT conn.");
    printOLED(0, 30, timeClient.getFormattedTime());
    display.display();
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0)
    {
      // basically die and wait for WDT to reset me
      while (1)
      {
        display.clearDisplay();
        printOLED(0, 0, "Conecting to MQTT ...");
        printOLED(0, 10, "Retrying MQTT conn.");
        printOLED(0, 20, "MQTT conn. dead");
        printOLED(0, 30, timeClient.getFormattedTime());
        printOLED(0, 40, "Check settings!");
        display.display();
        delay(999);
      };
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
  while ((subscription = mqtt.readSubscription()))
  {
    if (subscription == &sub)
    {
      mqqtSubParse((char *)sub.lastread);
    }
  }  
}

void mqttPub(int interval)
{
  String publishText = "";
  if ((millis() - delayPub) > interval)
  {
    publishText += "field1=";
    publishText += t;
    publishText += "&field2=";
    publishText += h;
    mqttPubAll(publishText, pub);
  }
}

void mqttPubAll(String payload, Adafruit_MQTT_Publish topic)
{
  delayPub = millis();
  if (! topic.publish(payload.c_str()))
  {
    Serial.println(F("Failed"));
  }
}
#pragma endregion

#pragma region Misc functions

void mqqtSubParse(const char * payload)
{
  StaticJsonDocument<500> doc;

  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  const char* text = doc["field3"];
  const char* rpc = doc["field4"];

  if (text)
  {
    serialLog("Print text on oled ...");
    flashWhite(75);
    messageToDisplay = text;
    delayClimate = millis();
  }
  if (rpc)
  {
    serialLog("RPC call ...");
    serialLog(rpc);
    flashWhite(25);  
    String degree = rpc; 
    runServo(degree.toInt());
  }
}

void serialLog(const char* txt)
{
  Serial.println(txt);
}

void runServo(int deg)
{
  String txt = "Servo: ";
  txt += deg;
  serialLog(txt.c_str());
  // printOLED(15, 0, txt, 2);
  display.display();
  servoOne.write(deg);
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

void getTime(int interval)
{
  if ((millis() - delayTime) > interval)
  {
    delayTime = millis();
    timeClient.update();
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

void loop()
{
  getClimate(5000);
  updateOLED(500);
  getTime(60000);
  MQTT_connect();
  mqttSub();
  mqttPub(25000);
}