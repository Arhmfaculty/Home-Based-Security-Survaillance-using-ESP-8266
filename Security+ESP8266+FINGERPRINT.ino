//Most of the print to the serial monitor was commented because it was interfering with the database. We left it however, to show that 
//those information can be uncommented and printed to the serial monitor in the absence of an lcd.

//The following codes were written using the Adafruit Fingerprint Sensor Library as foundation
// This code can be run to verify fingerprints.

#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <ESP_Mail_Client.h>

#define WIFI_SSID "Farm.io"
#define WIFI_PASSWORD "farm.@io"

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

#define AUTHOR_EMAIL "farm.ioproject@gmail.com"
#define AUTHOR_PASSWORD "lyqq vgwv rxzl liwb"

#define RECIPIENT1 "kelvin.obiorah@ashesi.edu.gh"
#define RECIPIENT2 "0033mab@gmail.com"
#define RECIPIENT3 "majorine.kyei@ashesi.edu.gh"
#define RECIPIENT4 "ewura.sam@ashesi.edu.gh"
#define RECIPIENT5 "jesse.donkor@ashesi.edu.gh"

SMTPSession smtp;

#if (defined(_AVR) || defined(ESP8266)) && !defined(AVR_ATmega2560_)
// For UNO and others without hardware serial, we must use software serial...
// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)
// Set up the serial port to use softwareserial..
SoftwareSerial mySerial(13, 15);
 
#else
// On Leonardo/M0/etc, others with hardware serial, use hardware serial!
// #0 is green wire, #1 is white
#define mySerial Serial1

#endif

//This is used to store students' name and their corresponding fingerIDs so that the students' names can be matched and displayed on the lcd.
int studentID[50] = {1, 2, 3, 4, 5, 6, 7, 8};

const char* studentName[50] = {"Ewura Ama", "Ewura Ama", "Kelvin", "Kelvin", "Jesse", "Jesse", "Majorine", "Majorine"};

#define buzzer 5 //Buzzer Pin 
#define echoPin 14 // attach pin D2 Arduino to pin Echo of HC-SR04
#define trigPin 12 //attach pin D3 Arduino to pin Trig of HC-SR04

bool buzzerState = false; // Initially buzzer is off

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup()
{
  //Serial.begin(9600);
  Serial.begin(115200);
  Serial.println();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(buzzer, OUTPUT); // Sets the buzzer as an Output
  while (!Serial);  
  delay(100);
  Serial.println("\n\nAdafruit finger detect test");

  // set the data rate for the sensor serial port
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  }
  else {
    Serial.println("Waiting for valid finger...");
      Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }
}

void Ultrasonic(){
   // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
 
  // Reads the echoPin, returns the sound wave travel time in microseconds
  long duration = pulseIn(echoPin, HIGH);

  // Calculating the distance
  float distance= duration*0.034/2;

  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  // Serial.println(" cm");
  
  // Check if the sensor detects a motion in about 5cm
  if (distance <= 100.0) {
    digitalWrite(buzzer, HIGH);
    delay(2000);
    sendEmail(RECIPIENT2, "Intrusion Alert ‼️", "An intrusion is detected on your farm. The necessary action needs to be taken  - BY FARM.IO ❤️");
  }
  else {
    digitalWrite(buzzer, buzzerState ? HIGH : LOW); // Turn on/off buzzer based on buzzerState
    delay(1000);
  }
}

void loop() {
  getFingerprintID();
  delay(50); // Delaying the time between two consecutive runs
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
//      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
     Serial.println("No finger detected");
      Ultrasonic();
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
     Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
//      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
//      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
//      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
//      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
//      Serial.println("Could not find fingerprint features");
      return p;
    default:
//      Serial.println("Unknown error");
      return p;    
  }

  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    buzzerState = false; // Turn off the buzzer
    digitalWrite(buzzer, LOW); // Turn off the buzzer
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
//    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
      Serial.println("Did not find a match");
      delay(1000);
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  bool found = false;
  for (byte n = 0; n < 50; n++) {
    if (finger.fingerID == studentID[n]) {
      Serial.print(studentName[n]);
      Serial.print(" ");
      buzzerState = true; // Turn on the buzzer
      digitalWrite(buzzer, HIGH); // Turn on the buzzer
      delay(5000);
      found = true;
    }
  }

  return finger.fingerID;
}

int getFingerprintIDez() {
  uint8_t p = finger.getImage();
   
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

   bool found = false;
  for (byte n = 0; n < 50; n++) {
    if (finger.fingerID == studentID[n]) {
      Serial.print(studentName[n]);
      found = true;
    }
  }

  if (!found)
    Serial.println("Fingerprint owner unknown");
    
  return finger.fingerID;
}

void sendEmail(const char* recipient, const char* subject, const char* message) {
  SMTP_Message smtpMessage;
  smtpMessage.sender.name = F("Farm.io");
  smtpMessage.sender.email = AUTHOR_EMAIL;
  smtpMessage.subject = subject;
  smtpMessage.addRecipient("", recipient);
  smtpMessage.text.content = message;
  smtpMessage.text.charSet = "us-ascii";
  smtpMessage.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  smtpMessage.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  smtpMessage.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

  // Create a Session_Config object
  Session_Config config;
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;
  config.login.user_domain = "";

  // Connect to the SMTP server
  if (!smtp.connect(&config)){
    Serial.println("Connection error");
    return;
  }

  // Send the email
  if (!MailClient.sendMail(&smtp, &smtpMessage)) {
    Serial.println("Error sending email");
    return;
  }

  Serial.println("Email sent successfully");
}
