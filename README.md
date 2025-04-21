Fingerprint Attendance System
A robust and reliable commercial fingerprint attendance system designed for colleges, featuring enrollment, attendance tracking, and data management. The system operates for approximately 10 hours daily and integrates with Google Sheets for data storage and a web dashboard for data retrieval.
Features
Enrollment Phase

Register student fingerprints with a unique ID (roll number).
Assign subject codes for specific classes.

Attendance Phase

Scan and match fingerprints against a stored database.
Mark attendance with a timestamp and subject code.
Store data locally in CSV format and send to Google Sheets.

Data Management

Retrieve attendance data via WiFi.
Download data as a CSV file or view on a web dashboard.

Hardware Requirements

ESP32 microcontroller
Adafruit Fingerprint Sensor
SSD1306 OLED Display (128x64)
4x4 Keypad
WiFi module (integrated with ESP32)

Software Requirements

Arduino IDE
Libraries: Adafruit_Fingerprint, Adafruit_GFX, Adafruit_SSD1306, Adafruit_Keypad, WiFi, WebServer, HTTPClient, SPIFFS
Google Apps Script for Google Sheets integration

Setup Instructions

Hardware Setup:
Connect the fingerprint sensor to ESP32 (RX: GPIO16, TX: GPIO17).
Connect the OLED display via I2C (SDA, SCL).
Connect the keypad to specified GPIO pins (26, 27, 14, 13, 12, 32, 33, 25).


Software Setup:
Install required Arduino libraries.
Update main.ino with your WiFi credentials and Google Script ID.
Deploy apps-script.gs in Google Apps Script and note the deployment URL.


Upload Code:
Upload main.ino to the ESP32 using Arduino IDE.


Google Sheets:
Ensure the Google Sheet is accessible and linked to the Apps Script.



Usage

Enrollment: Press '*' on the keypad and follow the OLED prompts to enroll a fingerprint.
Attendance: Enter a subject code using the keypad and press '#'. Place a finger on the sensor to mark attendance.
Data Retrieval: Access the web dashboard at the ESP32's IP address or download the CSV file from /download.

Repository Structure
fingerprint-attendance-system/
├── src/
│   └── main.ino              # Arduino code for ESP32
├── scripts/
│   └── apps-script.gs        # Google Apps Script for Google Sheets
├── .gitignore                # Git ignore file
├── README.md                 # Project documentation
└── LICENSE                   # MIT License

License
This project is licensed under the MIT License. See the LICENSE file for details.
