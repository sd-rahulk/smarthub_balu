#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN D4
#define DHTTYPE DHT11
#define GASPIN A0

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27,16,2);

ESP8266WebServer server(80);

const char* ssid = "SmartHub";
const char* password = "12345678";

float temperature;
float humidity;
int gasValue;

void handleRoot()
{
  File file = LittleFS.open("/index.html", "r");
  server.streamFile(file, "text/html");
  file.close();
}

void handleData()
{
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  gasValue = analogRead(GASPIN);

  String json = "{";
  json += "\"temp\":" + String(temperature) + ",";
  json += "\"hum\":" + String(humidity) + ",";
  json += "\"gas\":" + String(gasValue);
  json += "}";

  server.send(200, "application/json", json);
}

void setup()
{
  Serial.begin(115200);

  dht.begin();

  lcd.init();
  lcd.backlight();

  WiFi.softAP(ssid, password);

  Serial.println("Access Point Started");
  Serial.println(WiFi.softAPIP());

  LittleFS.begin();

  server.on("/", handleRoot);
  server.on("/data", handleData);

  server.begin();
}

void loop()
{
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  gasValue = analogRead(GASPIN);

  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("T:");
  lcd.print(temperature);
  lcd.print("C");

  lcd.setCursor(8,0);
  lcd.print("H:");
  lcd.print(humidity);

  lcd.setCursor(0,1);
  lcd.print("Gas:");
  lcd.print(gasValue);

  server.handleClient();

  delay(2000);
}