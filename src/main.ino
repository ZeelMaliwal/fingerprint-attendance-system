#include <Adafruit_Fingerprint.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Keypad.h>
#include <SPIFFS.h>
#include <FS.h>
#include <WiFi.h>
#include <WebServer.h>
#include <vector>
#include <SoftwareSerial.h>
#include <HTTPClient.h>

// WiFi credentials
const char* ssid = "SSID";
const char* password = "PASSWORD";

const char* GOOGLE_SCRIPT_ID = "SCRIPT_ID"; // Replace with your script ID
const char* googleSheetURL = "GOOGLE_SHEET_URL";

// Web Server
WebServer server(80);

// OLED Display Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Keypad Configuration
#define KEYPAD_PID 419
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {26, 27, 14, 13}; // GPIOs connected to row pins
byte colPins[COLS] = {12, 32, 33, 25}; // GPIOs connected to column pins
Adafruit_Keypad customKeypad = Adafruit_Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Fingerprint Sensor (using hardware Serial2 for better reliability)
#define FINGERPRINT_RX 16
#define FINGERPRINT_TX 17
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial2);

// Global Variables
String currentSubjectCode = "";
String inputBuffer = "";
File attendanceFile;
std::vector<String> subjectCodes;
int lastEnrolledRoll = 0;

bool sendToGoogleSheets(String action, String data1, String data2, String data3 = "") {
  HTTPClient http;
  String url = String(googleSheetURL);
  url += "?action=" + action;
  url += "&data1=" + urlEncode(data1);
  url += "&data2=" + urlEncode(data2);
  if (data3 != "") {
    url += "&data3=" + urlEncode(data3);
  }

  http.begin(url);
  int httpCode = http.GET();
  bool success = (httpCode == 200);
  http.end();
  return success;
}

// URL encoding function
String urlEncode(String str) {
  String encodedString = "";
  char c;
  char code0;
  char code1;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (c == ' ') {
      encodedString += '+';
    } else if (isalnum(c)) {
      encodedString += c;
    } else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9) {
        code0 = c - 10 + 'A';
      }
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
    }
  }
  return encodedString;
}

// HTML Template for Web Dashboard
const char* htmlTemplate = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Attendance Dashboard</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        .container { max-width: 800px; margin: 0 auto; }
        table { width: 100%; border-collapse: collapse; margin-top: 20px; }
        th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
        th { background-color: #f2f2f2; }
        .download-btn { 
            background-color: #4CAF50;
            color: white;
            padding: 10px 20px;
            text-decoration: none;
            border-radius: 4px;
            display: inline-block;
            margin: 10px 0;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Attendance Dashboard</h1>
        <h2>Available Subject Codes:</h2>
        %SUBJECT_LIST%
    </div>
</body>
</html>
)";

void handleRoot() {
  String subjectList = "<ul>";
  for (String code : subjectCodes) {
    subjectList += "<li>" + code + "</li>";
  }
  subjectList += "</ul>";
  String html = String(htmlTemplate);
  html.replace("%SUBJECT_LIST%", subjectList);
  server.send(200, "text/html", html);
}

void handleDownload() {
  String csvContent = "Timestamp,FingerprintID,SubjectCode\n";
  if (SPIFFS.exists("/attendance.csv")) {
    File file = SPIFFS.open("/attendance.csv", "r");
    while (file.available()) {
      csvContent += file.readStringUntil('\n') + "\n";
    }
    file.close();
  }
  server.send(200, "text/csv", csvContent);
}

void setup() {
  Serial.begin(115200);

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Initialize OLED Display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Initializing...");
  display.display();

  // Initialize Keypad
  customKeypad.begin();

  // Initialize Fingerprint Sensor
  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  // Initialize WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  // Initialize Web Server
  server.on("/", handleRoot);
  server.on("/download", handleDownload);
  server.begin();
  Serial.println("HTTP server started");

  // Load subject codes (example)
  subjectCodes.push_back("CS101");
  subjectCodes.push_back("MATH201");

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("System Ready");
  display.display();
}

void loop() {
  server.handleClient();
  customKeypad.tick();

  // Handle Keypad Input
  while (customKeypad.available()) {
    keypadEvent e = customKeypad.read();
    if (e.bit.EVENT == KEY_JUST_PRESSED) {
      char key = (char)e.bit.KEY;
      if (key == '#') {
        if (inputBuffer.length() > 0) {
          currentSubjectCode = inputBuffer;
          inputBuffer = "";
          display.clearDisplay();
          display.setCursor(0, 0);
          display.println("Subject: " + currentSubjectCode);
          display.display();
        }
      } else if (key == '*') {
        enrollFingerprint();
      } else {
        inputBuffer += key;
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Input: " + inputBuffer);
        display.display();
      }
    }
  }

  // Handle Fingerprint Scanning
  int fingerprintID = getFingerprintID();
  if (fingerprintID > 0) {
    String timestamp = String(millis());
    String data = timestamp + "," + String(fingerprintID) + "," + currentSubjectCode + "\n";
    
    // Save to local file
    attendanceFile = SPIFFS.open("/attendance.csv", FILE_APPEND);
    if (attendanceFile) {
      attendanceFile.print(data);
      attendanceFile.close();
    }

    // Send to Google Sheets
    sendToGoogleSheets("attendance", String(fingerprintID), currentSubjectCode, timestamp);

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Attendance Marked");
    display.println("ID: " + String(fingerprintID));
    display.display();
    delay(2000);
  }
}

int getFingerprintID() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) return -1;

  return finger.fingerID;
}

void enrollFingerprint() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Place finger...");
  display.display();

  int id = lastEnrolledRoll + 1;
  uint8_t p = -1;

  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        break;
      case FINGERPRINT_NOFINGER:
        break;
      default:
        display.clearDisplay();
        display.println("Error capturing");
        display.display();
        delay(2000);
        return;
    }
  }

  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) {
    display.clearDisplay();
    display.println("Error processing");
    display.display();
    delay(2000);
    return;
  }

  display.clearDisplay();
  display.println("Remove finger");
  display.display();
  delay(2000);

  p = finger.getImage();
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }

  display.clearDisplay();
  display.println("Place same finger");
  display.display();

  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        break;
      default:
        display.clearDisplay();
        display.println("Error capturing");
        display.display();
        delay(2000);
        return;
    }
  }

  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) {
    display.clearDisplay();
    display.println("Error processing");
    display.display();
    delay(2000);
    return;
  }

  p = finger.createModel();
  if (p != FINGERPRINT_OK) {
    display.clearDisplay();
    display.println("Fingerprints don't match");
    display.display();
    delay(2000);
    return;
  }

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    lastEnrolledRoll = id;
    sendToGoogleSheets("enroll", String(id), "");
    display.clearDisplay();
    display.println("Enrolled ID: " + String(id));
    display.display();
    delay(2000);
  } else {
    display.clearDisplay();
    display.println("Error storing");
    display.display();
    delay(2000);
  }
}