#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <Timezone.h>

/***********Variables***********/
String wifi_ssid      = "CHANGE TO YOUR WIFI SSID";
String wifi_password  = "CHANGE TO YOUR WIFI PASSWORD";
char timeserver[20]   = "il.pool.ntp.org";
/*******************************/

TimeChangeRule myIDT = {"IDT", Last, Fri, Mar, 2, 180};    // Israel Daylight time = UTC - 3 hours
TimeChangeRule myIST = {"IST", Last, Sun, Oct, 2, 120};    // Israel standard time = UTC + 2  hours
Timezone myTZ(myIDT, myIST);

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

void setup() {
  WiFi.begin(wifi_ssid, wifi_password);
  
  configTime(0 * 3600, 0, timeserver, "time.nist.gov");
  setTime(myTZ.toUTC(compileTime()));
  
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
  return "None";
}

void loop() { 
  time_t local = myTZ.toLocal(now());
  display_time(local);
  delay(10000);
}

void display_time(time_t unix_time){
  String time = String(ctime(&unix_time));
  time.trim();
  time.substring(11,19).toCharArray(time_value, 10); 

  matrix.fillScreen(LOW);
  matrix.drawChar(2,0, time_value[0], HIGH,LOW,1); // H
  matrix.drawChar(8,0, time_value[1], HIGH,LOW,1); // HH  
  matrix.drawChar(14,0,time_value[2], HIGH,LOW,1); // HH:
  matrix.drawChar(20,0,time_value[3], HIGH,LOW,1); // HH:M
  matrix.drawChar(26,0,time_value[4], HIGH,LOW,1); // HH:MM
  matrix.write();
}

void display_message(String message){
  int wait = 70; // In milliseconds
  int spacer = 1;
  int width  = 5 + spacer; // The font width is 5 pixels

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

// Function to return the compile date and time as a time_t value
time_t compileTime()
{
    const time_t FUDGE(10);     // fudge factor to allow for compile time (seconds, YMMV)
    const char *compDate = __DATE__, *compTime = __TIME__, *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
    char chMon[4], *m;
    tmElements_t tm;

    strncpy(chMon, compDate, 3);
    chMon[3] = '\0';
    m = strstr(months, chMon);
    tm.Month = ((m - months) / 3 + 1);

    tm.Day = atoi(compDate + 4);
    tm.Year = atoi(compDate + 7) - 1970;
    tm.Hour = atoi(compTime);
    tm.Minute = atoi(compTime + 3);
    tm.Second = atoi(compTime + 6);
    time_t t = makeTime(tm);
    return t + FUDGE;           // add fudge factor to allow for compile time
}
