#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <time.h>

/***********Variables***********/
String wifi_ssid      = "CHANGE TO YOUR WIFI SSID";
String wifi_password  = "CHANGE TO YOUR WIFI PASSWORD";
char timezone[10]     = "IST-2IDT";
char timeserver[20]   = "il.pool.ntp.org";
/*******************************/

// LED Matrix Pins (NodeMCU):
// Vcc -> 3v
// Gnd -> G
// DIN -> D7
// CS  -> D4
// CLK -> D5

int pinCS = D4; 
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays   = 1;
char time_value[20];

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

int wait = 70; // In milliseconds
int spacer = 1;
int width  = 5 + spacer; // The font width is 5 pixels

int m;
String t, h;

#define DHTPIN 0          // D3
#define DHTTYPE DHT11     // DHT 11

void setup() {
  Serial.begin(9600);
  WiFi.begin(wifi_ssid, wifi_password);
  
  configTime(0 * 3600, 0, timeserver, "time.nist.gov");
  setenv("TZ", timezone, 1);
  
  matrix.setIntensity(10); // Use a value between 0 and 15 for brightness
  matrix.setRotation(0, 1); // The first display is position upside down
  matrix.setRotation(1, 1); // The first display is position upside down
  matrix.setRotation(2, 1); // The first display is position upside down
  matrix.setRotation(3, 1); // The first display is position upside down
  matrix.fillScreen(LOW);
  matrix.write();
  
  while (WiFi.status() != WL_CONNECTED) {
    display_message(wifi_message());
    delay(3000);
  } 
}

String wifi_message() {
  switch (WiFi.status()){
    case WL_NO_SSID_AVAIL:
      return "SSID cannot be reached";
    case WL_CONNECTED:
      return "Connection established";
    case WL_CONNECT_FAILED:
      return "Connection failed - Wrong password?";
    case WL_DISCONNECTED:
      return "Connecting...";
  }
}

void loop() {
  m = map(analogRead(0),0,1024,0,12);
  matrix.fillScreen(LOW);
  time_t now = time(nullptr);
  String time = String(ctime(&now));
  time.trim();
  time.substring(11,19).toCharArray(time_value, 10); 
  matrix.drawChar(2,0, time_value[0], HIGH,LOW,1); // H
  matrix.drawChar(8,0, time_value[1], HIGH,LOW,1); // HH  
  matrix.drawChar(14,0,time_value[2], HIGH,LOW,1); // HH:
  matrix.drawChar(20,0,time_value[3], HIGH,LOW,1); // HH:M
  matrix.drawChar(26,0,time_value[4], HIGH,LOW,1); // HH:MM
  matrix.write();
  delay(10000);
}

void display_message(String message){
   for ( int i = 0 ; i < width * message.length() + matrix.width() - spacer; i++ ) {
    //matrix.fillScreen(LOW);
    int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    int y = (matrix.height() - 8) / 2; // center the text vertically
    while ( x + width - spacer >= 0 && letter >= 0 ) {
      if ( letter < message.length() ) {
        matrix.drawChar(x, y, message[letter], HIGH, LOW, 1); // HIGH LOW means foreground ON, background off, reverse to invert the image
      }
      letter--;
      x -= width;
    }
    matrix.write(); // Send bitmap to display
    delay(wait/2);
  }
}
