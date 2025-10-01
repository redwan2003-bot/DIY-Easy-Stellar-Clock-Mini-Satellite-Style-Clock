#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_SHT31.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// ===== WiFi info =====
const char* ssid     = "your_wifi";
const char* password = "pass";

// ===== OLED setup =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
pp
// ===== SHT31 setup =====
Adafruit_SHT31 sht31 = Adafruit_SHT31();

// ===== NTP setup =====
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7 * 3600, 60000); // GMT+7

// ===== Pins  =====
#define I2C_SDA 8
#define I2C_SCL 9
#define LED_PIN 0

// LED
unsigned long lastBlink = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);

  // WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setRotation(1);
  display.display();

  // SHT31 init
  if (!sht31.begin(0x44)) {
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }

  // NTP init
  timeClient.begin();

  // LED init
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  timeClient.update();

  int hours = timeClient.getHours();
  int minutes = timeClient.getMinutes();
  int seconds = timeClient.getSeconds();

  // AM/PM
  bool isPM = false;
  if (hours >= 12) {
    isPM = true;
    if (hours > 12) hours -= 12;
  }
  if (hours == 0) hours = 12;

  // Đọc cảm biến
  float tempC = sht31.readTemperature();
  float hum   = sht31.readHumidity();

  // Đổi sang °F
  float tempF = tempC * 9.0 / 5.0 + 32.0;

  // Format chuỗi
  char hStr[3], mStr[3], sStr[3];
  sprintf(hStr, "%02d", hours);
  sprintf(mStr, "%02d", minutes);
  sprintf(sStr, "%02d", seconds);

  // ===== OLED =====
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // Header
  display.setTextSize(0.5);
  display.setCursor(2, 0);
  display.println("/////");
  display.setCursor(60, 10);
  display.println(isPM ? "PM" : "AM");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

  // hh
  display.setTextSize(2);
  display.setCursor(5, 30);
  display.println(hStr);

  // mm
  display.setTextSize(2);
  display.setCursor(5, 52);
  display.println(mStr);

  // ss
  display.setTextSize(2);
  display.setCursor(5, 72);
  display.println(sStr);

  // --------
  display.drawLine(0, 95, 128, 95, SSD1306_WHITE);

  // (°F) & Độ ẩm
  display.setTextSize(1);
  display.setCursor(5, 105);
  display.print((int)tempF);
  display.print((char)247); // ký hiệu °
  display.print("F ");
  display.setCursor(5, 120);
  display.print((int)hum);
  display.print("%");

  display.display();

  // ===== LED =====
  unsigned long now = millis();
  if (now - lastBlink >= 5000) {  // mỗi 5 giây
    digitalWrite(LED_PIN, HIGH);
    delay(20);                    // sáng 20 ms
    digitalWrite(LED_PIN, LOW);
    lastBlink = now;
  }

  delay(200); // giảm tải CPU
}
