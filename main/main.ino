// some code shamelessly pulled from https://randomnerdtutorials.com/esp32-tft-lvgl-digital-clock/ and from https://github.com/espressif/arduino-esp32/blob/master/libraries/HTTPClient/examples/BasicHttpClient/BasicHttpClient.ino

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WifiMulti.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1
// most ssd1306 with a screen height of 64 will use address 0x3D instead of 0x3C. i have a botched one, if yours doesn't work, try changing this to 0x3D.
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define USE_SERIAL Serial

WiFiMulti wifiMulti;

// replace with your own network's ssid and password
const char* ssid = "ssid";
const char* password = "password";

// replace with your own timezone from https://worldtimeapi.org/api/timezone
const char* timezone = "America/New_York";

String current_date;
String current_time;

String final_time_str;

static int32_t hour;
static int32_t minute;
static int32_t second;
bool sync_time_date = false;

// adds a leading zero if the input time is less than 10
String format_time(int time) {
  return (time < 10) ? "0" + String(time) : String(time);
}

void worldtime_sync() {
  // wait for wifi connection
  if ((wifiMulti.run() == WL_CONNECTED)) {
    HTTPClient http;

    USE_SERIAL.print("[HTTP] begin...\n");
    http.begin(String("http://worldtimeapi.org/api/timezone/") + timezone);

    USE_SERIAL.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        USE_SERIAL.println(payload);
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
        if (!error) {
          const char* datetime = doc["datetime"];
          String datetime_str = String(datetime);
          int splitIndex = datetime_str.indexOf('T');
          current_date = datetime_str.substring(0, splitIndex);
          current_time = datetime_str.substring(splitIndex + 1, splitIndex + 9);
          hour = current_time.substring(0, 2).toInt();
          minute = current_time.substring(3, 5).toInt();
          second = current_time.substring(6, 8).toInt();
          // Serial.println(current_time.substring(0, 2));
          // Serial.println(current_time.substring(3, 5));
          // Serial.println(current_time.substring(6, 8));
          
          // comment out all instances of this if statement if you want 24hr time
          if (hour > 12) {
            hour = hour % 12;
          }
          sync_time_date = true;
        }
        else {
          Serial.print("deserializationJson() failed: ");
          Serial.println(error.c_str());
        }
      }
    }
    else {
      USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
}

void get_initial_sync() {
  while (!sync_time_date) {
    Serial.println("Initial syncing...");
    display.clearDisplay();
    display.setCursor(12, 26);
    display.println("Initial syncing...");
    display.display();
    worldtime_sync();
    delay(1000);
  }
  Serial.println("Initial Sync Complete.");
  display.clearDisplay();
  display.setCursor(23, 26);
  display.println("Sync complete!");
  display.display();
  delay(1000);
}

static void timer_cb() {
  second++;
  if (second > 59) {
    second = 0;
    minute++;
    if (minute > 59) {
      minute = 0;
      hour++;
      sync_time_date = false;
      hourly_resync();
        if (hour > 12) {
          hour = hour % 12;
        }
      }
    }
  
  // uncomment this line if you want a leading zero for hours with one digit. you'll need to re-adjust all instances of setCursor (that deal with time) to (20, 26).
  // String hour_time_f = format_time(hour);
  String hour_time_f = String(hour);
  String minute_time_f = format_time(minute);
  String second_time_f = format_time(second);

  final_time_str = String(hour_time_f) + ":" + String(minute_time_f);

  display.clearDisplay();
  if (hour < 10) {
    display.setCursor(31, 26);
  }
  else {
    display.setCursor(20, 26);
  }
  display.setTextSize(3);
  display.printf(final_time_str.c_str());
  display.setCursor(36, 10);
  display.setTextSize(1);
  display.printf(current_date.c_str());
  display.display();
}

void hourly_resync() {
  while (!sync_time_date) {
    timer_cb();
    Serial.println("Resyncing...");
    display.clearDisplay();
    if (hour < 10) {
      display.setCursor(31, 26);
    }
    else {
      display.setCursor(20, 26);
    }
    display.setTextSize(3);
    display.println(final_time_str.c_str());
    display.setCursor(36, 10);
    display.setTextSize(1);
    display.printf(current_date.c_str());
    display.setCursor(30, 53);
    display.println("Resyncing...");
    display.display();
    worldtime_sync();
    delay(1000);
  }
  Serial.println("Resync complete.");
}

void setup() {
  USE_SERIAL.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed!"));
    for (;;)
      ; // Don't proceed, loop forever
  }

  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  for (uint8_t t = 4; t > 0; t--){
    USE_SERIAL.printf("[SETUP]WAIT %d...\n", t);
    USE_SERIAL.flush();
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(20, 23);
    display.println("Garrett's Great");
    display.setCursor(30, 36);
    display.println("ESP32 Clock");
    display.display();
    delay(1000);
  }

  wifiMulti.addAP(ssid, password);

  get_initial_sync();

  display.clearDisplay();
  
}

void loop() {
  timer_cb();
  delay(1000);
}
