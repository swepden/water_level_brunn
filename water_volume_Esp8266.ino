/*************************************************************
  Stabil version: HC-SR04 + volymberäkning + Blynk
  - Mäter avstånd kontinuerligt
  - Skickar värden till Blynk varje minut
 *************************************************************/

#define BLYNK_TEMPLATE_ID           "TMPL4BSLrZ5gx"
#define BLYNK_TEMPLATE_NAME         "Quickstart Template"
#define BLYNK_AUTH_TOKEN            "NiuQL6Uxp3nElBqRk7tyggQohGWAYuwr"

#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// HC-SR04 pins
#define TRIG_PIN D5  // GPIO14
#define ECHO_PIN D6  // GPIO12

// WiFi-info
char ssid[] = "DeGgObOt_m";
char pass[] = "12345678";

// Tankdimensioner (justera efter din tunna)
#define TANK_HEIGHT_CM 80       // Totalhöjd på tunnan
#define TANK_RADIUS_CM 25       // Radie på tunnan

// Timer och lagring
BlynkTimer timer;
long lastDistance = -1;
float lastVolume = 0;

// Funktion: Mät avstånd i cm
long readDistanceCM()
{
  delay(50);  // Stabilisering
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(5);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);  // Timeout: 30ms

  if (duration == 0) {
    return -1;
  }

  return duration * 0.034 / 2;
}

// Funktion: Skicka till Blynk varje minut
void myTimerEvent()
{
  if (lastDistance >= 0) {
    Serial.print("Avstånd: ");
    Serial.print(lastDistance);
    Serial.println(" cm");

    Serial.print("Volym: ");
    Serial.print(lastVolume);
    Serial.println(" liter");

    Blynk.virtualWrite(V4, lastDistance);  // Avstånd i cm
    Blynk.virtualWrite(V5, lastVolume);    // Volym i liter
  } else {
    Serial.println("Ingen giltig mätning sparad");
    Blynk.virtualWrite(V4, 0);
    Blynk.virtualWrite(V5, 0);
  }
}

BLYNK_CONNECTED()
{
  Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(V3, "onImageUrl",  "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V3, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
}

void setup()
{
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.println("Startar upp...");
  long test = readDistanceCM();
  Serial.print("Första mätning: ");
  Serial.println(test);

  timer.setInterval(60000L, myTimerEvent);  // Kör varje minut
}

void loop()
{
  Blynk.run();
  timer.run();

  // Mät avstånd kontinuerligt
  long distance = readDistanceCM();
  if (distance >= 0 && distance <= TANK_HEIGHT_CM) {
    lastDistance = distance;
    long waterHeight = TANK_HEIGHT_CM - distance;
    lastVolume = 3.1416 * TANK_RADIUS_CM * TANK_RADIUS_CM * waterHeight / 1000.0;

    Serial.print("Loop-mätning: ");
    Serial.print(distance);
    Serial.print(" cm → ");
    Serial.print(lastVolume);
    Serial.println(" liter");
  }

  delay(1000);  // En mätning per sekund
}
