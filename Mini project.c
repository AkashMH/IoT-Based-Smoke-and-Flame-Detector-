#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// LCD setup: address 0x27, 16 columns, 2 rows
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Constants for EEPROM storage
const int EEPROM_SIZE = 512; // EEPROM size for Arduino Nano
const int MAX_PHONE_LENGTH = 15; // Max phone number length
const int PHONE_COUNT = 2; // Number of phone numbers to store

// Pin definitions for smoke and flame detection
const int redLed = 4;
const int greenLed = 6;
const int buzzer = 5;
const int smokeA0 = A0;  // Smoke sensor analog pin
const int flame = 2;     // Flame sensor digital pin

// GSM module pins (adjust these based on your wiring)
const int gsmTX = 7;
const int gsmRX = 8;
SoftwareSerial gsmSerial(gsmTX, gsmRX); // Create a SoftwareSerial object for GSM communication

// Thresholds
int sensorThres = 200;  // Smoke sensor threshold for triggering alarm

// EEPROM storage offsets
int offsets[PHONE_COUNT];

void setup() {
  Serial.begin(9600);  // Start serial communication for debugging

  // Initialize LCD
  lcd.init();  // Use init() instead of begin()
  lcd.backlight();

  // Initialize pins for smoke and flame detection
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(flame, INPUT);
  
  // Initialize offsets for phone numbers in EEPROM
  initializeOffsets();

  // Store phone numbers in EEPROM
  writeToEEPROM(offsets[0], "+917022869350");
  writeToEEPROM(offsets[1], "+917411249153");

  // Confirm storage of phone numbers
  Serial.println("Phone numbers stored in EEPROM!");

  // Initialize GSM module communication
  gsmSerial.begin(9600);
  delay(1000); // Allow time for GSM module to initialize
  
  // Send a test message to confirm GSM communication
  sendSMS("Test message from system!");

  // Display a welcome message on the LCD
  lcd.setCursor(0, 0);
  lcd.print("System Initializing");
  delay(1000);  // Wait for a second
  lcd.clear();
}

void loop() {
  // Read the incoming number (for testing)
  String incomingNumber = "+917022869350";
  Serial.println("Incoming Number: " + incomingNumber);

  bool isMatched = false;

  // Compare the incoming number with stored numbers
  for (int i = 0; i < PHONE_COUNT; i++) {
    String storedNumber = readFromEEPROM(offsets[i], MAX_PHONE_LENGTH);
    if (compareWithoutCountryCode(storedNumber, incomingNumber)) {
      isMatched = true;
      break;
    }
  }

  // Display matching result on LCD
  lcd.setCursor(0, 0);
  lcd.print("Incoming Number: ");
  lcd.print(incomingNumber);

  if (isMatched) {
    lcd.setCursor(0, 1);
    lcd.print("Number matched!");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("Number not matched!");
  }

  // Read smoke sensor value
  int smokeLevel = analogRead(smokeA0);

  // Read flame sensor value (HIGH means no flame detected)
  int flameDetected = digitalRead(flame);

  // Display smoke level and flame status on LCD
  lcd.setCursor(0, 0);
  lcd.print("Smoke: ");
  lcd.print(smokeLevel);

  lcd.setCursor(0, 1);
  lcd.print("Flame: ");
  if (flameDetected == HIGH) {
    lcd.print("No Flame");
  } else {
    lcd.print("Flame Detected");
  }

  // If smoke level exceeds threshold or flame detected, trigger alarm and send SMS or make call
  if (smokeLevel > sensorThres || flameDetected == LOW) {
    digitalWrite(redLed, HIGH);  // Turn on red LED
    digitalWrite(greenLed, LOW); // Turn off green LED
    digitalWrite(buzzer, HIGH);  // Activate buzzer

    // Send SMS or make a call if a fire or smoke is detected
    sendSMS("Fire or Smoke Detected! Please take action.");
    makeCall("+917022869350");  // Example number, replace with actual phone numbers
  } else {
    digitalWrite(redLed, LOW);   // Turn off red LED
    digitalWrite(greenLed, HIGH); // Turn on green LED
    digitalWrite(buzzer, LOW);   // Deactivate buzzer
  }

  delay(5000);  // Delay for readability and sensor updates
}

// Write data to EEPROM
void writeToEEPROM(int address, const String &data) {
  for (int i = 0; i < data.length(); i++) {
    EEPROM.write(address + i, data[i]); // Write each character to EEPROM
  }
  EEPROM.write(address + data.length(), '\0'); // Write null terminator
}

// Read data from EEPROM
String readFromEEPROM(int address, int length) {
  char data[length]; // Buffer to hold the data
  for (int i = 0; i < length; i++) {
    data[i] = EEPROM.read(address + i); // Read each byte from EEPROM
    if (data[i] == '\0') break; // Stop at null terminator
  }
  return String(data); // Convert the buffer to a String and return it
}

// Compare two numbers without considering the country code
bool compareWithoutCountryCode(String storedNumber, String incomingNumber) {
  // Remove country code (assumes country code is the first three characters)
  String storedWithoutCode = storedNumber.substring(3);
  String incomingWithoutCode = incomingNumber.substring(3);

  return storedWithoutCode == incomingWithoutCode; // Return true if they match
}

// Initialize offsets for EEPROM storage
void initializeOffsets() {
  int currentOffset = 0; // Start at the beginning of EEPROM
  for (int i = 0; i < PHONE_COUNT; i++) {
    offsets[i] = currentOffset; // Set the offset for each phone number
    currentOffset += MAX_PHONE_LENGTH + 1; // Move to the next storage location
  }
}

// Function to send an SMS
void sendSMS(String message) {
  gsmSerial.println("AT+CMGF=1"); // Set SMS to text mode
  delay(1000);
  gsmSerial.println("AT+CMGS=\"+917022869350\""); // Replace with the actual phone number
  delay(1000);
  gsmSerial.println(message); // Send the message
  delay(1000);
  gsmSerial.println((char)26);  // ASCII code for Ctrl+Z to send the SMS
  delay(1000);
}

// Function to make a call
void makeCall(String phoneNumber) {
  gsmSerial.println("ATD" + phoneNumber + ";"); // Dial the phone number
  delay(1000);
}