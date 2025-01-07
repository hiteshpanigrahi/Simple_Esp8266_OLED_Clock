#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

// Define OLED display parameters
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_I2C_ADDRESS 0x3C  // I2C address of OLED

// Set up WiFi details
const char* ssid = "Tathastu-2.4G";  // Replace with your WiFi network name (SSID)
const char* password = "tathastu@099";  // Replace with your WiFi password

// Initialize the OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Set up NTP client
WiFiUDP udp;
NTPClient timeClient(udp, "pool.ntp.org", 0, 3600000);  // UTC offset of 0, update every hour

// IST Offset (Indian Standard Time is UTC +5:30, so it's 5 hours and 30 minutes ahead)
const long IST_OFFSET = 5 * 3600 + 30 * 60;  // 5 hours 30 minutes in seconds

// Array of month names
const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

// Array of days of the week
const char* days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

void setup() {
  // Start serial communication for debugging
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Stop if OLED initialization fails
  }

  // Initialize the NTP client
  timeClient.begin();
  timeClient.update();

  // Clear display
  display.clearDisplay();
}

void loop() {
  // Update the time from NTP
  timeClient.update();

  // Get the raw time in seconds since epoch (UTC)
  unsigned long epochTime = timeClient.getEpochTime();

  // Adjust time to IST by adding the IST_OFFSET (5 hours 30 minutes)
  epochTime += IST_OFFSET;

  // Convert to time format
  int hours = (epochTime % 86400L) / 3600;
  int minutes = (epochTime % 3600) / 60;
  int seconds = epochTime % 60;

  // Add leading zero for single-digit hours, minutes, and seconds
  String strHours = (hours < 10) ? "0" + String(hours) : String(hours);
  String strMinutes = (minutes < 10) ? "0" + String(minutes) : String(minutes);
  String strSeconds = (seconds < 10) ? "0" + String(seconds) : String(seconds);

  // Format time as HH:MM:SS
  String currentTime = strHours + ":" + strMinutes + ":" + strSeconds;

  // Determine AM/PM
  String ampm = (hours >= 12) ? "PM" : "AM";
  if (hours > 12) hours -= 12;  // Convert to 12-hour format

  // Convert epoch time to a struct tm for date calculation
  time_t rawTime = epochTime;  // Cast to time_t
  struct tm *timeInfo;
  timeInfo = localtime(&rawTime);

  // Get the date (DD/MM/YY)
  int day = timeInfo->tm_mday;  // Day of the month (1-31)
  int month = timeInfo->tm_mon;  // Month (0-11)
  int year = timeInfo->tm_year + 1900;  // Year since 1900, so add 1900

  // Get the day of the week (0-6, Sunday-Saturday)
  int dayOfWeek = timeInfo->tm_wday;  // Day of the week (0 = Sunday, 1 = Monday, ..., 6 = Saturday)

  String currentDate = String(day) + " " + months[month] + " " + String(year);  // Format as DD Month YYYY

  // Clear the display
  display.clearDisplay();

  // Display AM/PM at the top right corner
  display.setTextSize(1);  // Smaller text for AM/PM
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(SCREEN_WIDTH - 35, 0);  // Position at the top-right corner
  display.print(ampm);

  // Display day of the week at the top left corner
  display.setCursor(0, 0);  // Position at the top-left corner
  display.print(days[dayOfWeek]);  // Print the day of the week (e.g., "Mon", "Tue")

  // Set text size for the clock (larger text for hours and minutes)
  display.setTextSize(3);     // Larger text for the clock (bold and pixelated)
  display.setTextColor(SSD1306_WHITE);

  // Calculate the width of the current time string
  int textWidth = currentTime.length() * 12;  // Each character width is approximately 12 pixels (size 3)

  // Calculate the horizontal position to center the text
  int x = (SCREEN_WIDTH - textWidth) / 5;

  // Calculate the vertical position for centering the clock
  int textHeight = 3 * 8;  // 3 times the font height (font size 3 gives 24 pixels)
  int y = (SCREEN_HEIGHT - textHeight) / 2;  // Center vertically, adjust to move a bit up

  // Display the clock at the center of the screen
  display.setCursor(x, y); // Set cursor at the center of the screen
  display.print(strHours); 
  display.print(":");
  display.print(strMinutes);

  // Set text size for the seconds (smaller text for seconds)
  display.setTextSize(2);  // Smaller text for the seconds
  display.setCursor(SCREEN_WIDTH - 40,y + 7);  // Position seconds on the right side
  display.print(" ");
  display.print(strSeconds);

  // Set text size for the date (smaller text for the date)
  display.setTextSize(1);  // Smaller text for the date

  // Calculate vertical position for the date (place it near the bottom of the screen)
  int dateTextHeight = 8;  // Height of the date text (smaller text size)
  int dateY = SCREEN_HEIGHT - dateTextHeight - 2;  // 2 pixels from the bottom
  int dateWidth = currentDate.length() * 6;  // Estimate width of the date text

  // Calculate the horizontal position to center the date
  int dateX = (SCREEN_WIDTH - dateWidth) / 2;

  // Display the date at the bottom of the screen (formatted as DD Month YYYY)
  display.setCursor(dateX, dateY);
  display.print(currentDate);

  display.display();  // Update the display

  delay(1000);  // Update every second
}
