#include <Adafruit_Fingerprint.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>

#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#ifndef PROTOTYPES_H
#define PROTOTYPES_H
#endif

#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#endif

#ifndef VARIABLES_H
#define VARIABLES_H
#endif

#define RED_LED 12
#define GREEN_LED 14
#define BUZZER 0
#define REGISTER 15      // To register
#define AUTHENTICATE 13  // To mark attendance

/*******************************
VARIABLE DECLARATIONS
*******************************/
struct Variables {
  /* data */
  uint8_t id;
  int wait{ 200 };
  int successBuzz{ 100 };
  int timeOutVal{ 10000 };
  int failureBuzz{ 1000 };
};

String postData;                                                   // post array that will be send to the website
String link = "http://192.168.137.1:8080/biometric/authenticate.php";  //computer IP or the server domain
int FingerID = 0;                                                  // The Fingerprint ID from the scanner
uint8_t id;


/*******************************
FUNCTION PROTOTYPES
*******************************/
uint8_t getFingerprintEnroll();
uint8_t readnumber(void);
void numberOfFingers();
void getFingerprintIDRecog();
void successNotify(int, String);
void failNotity(int, String);
void printHex(int num, int precision);
void Reset();
uint8_t getFingerprintID();
int getFingerprintIDez();
void getUserData();

void connectWiFi();
String SendFingerprintID(int finger);
void authenticate(int);
uint8_t getFingerprintID();

/*******************************
FUNCTION DEFINITIONS
*******************************/
//Structs usage
Variables var;
//LCD definition
LiquidCrystal_I2C lcd(0x27, 16, 2);
// Setting up the fingerprint sensor
#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial mySerial(2, 3);
#endif
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// Setting up WiFi for pushing data
const char *ssid = "AKORA-ING-DKB";
const char *password = "deekaybee";
const char *host = "http://192.168.137.1:8080/biometric/getData.php";  // host server/ the php server address
WiFiClient wifiClient;
HTTPClient http;

uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (!Serial.available())
      ;
    num = Serial.parseInt();
  }
  return num;
}

/* Enrolling a new person */
uint8_t getFingerprintEnroll() {
  int p = -1;
  lcd.scrollDisplayLeft();
  lcd.print("Waiting for valid finger to enroll as #");
  lcd.print(var.id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        lcd.clear();
        lcd.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        lcd.clear();
        lcd.println(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        lcd.clear();
        lcd.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        lcd.clear();
        lcd.println("Imaging error");
        break;
      default:
        lcd.clear();
        lcd.println("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      lcd.clear();
      lcd.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      lcd.clear();
      lcd.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      lcd.clear();
      lcd.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      lcd.clear();
      lcd.scrollDisplayLeft();
      lcd.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      lcd.clear();
      lcd.scrollDisplayLeft();
      lcd.println("Could not find fingerprint features");
      return p;
    default:
      lcd.clear();
      lcd.println("Unknown error");
      return p;
  }

  lcd.clear();
  lcd.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID ");
  Serial.println(var.id);
  p = -1;
  lcd.clear();
  lcd.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");
  Serial.println(var.id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID ");
  Serial.println(var.id);
  p = finger.storeModel(var.id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  return true;
}

void successNotify(int buzz, String message) {
  // This is the sound and LED indication when a operation is successful
  for (int i = 3; i > 0; i--) {
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BUZZER, HIGH);
    delay(buzz);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BUZZER, LOW);
    delay(buzz);
  }
  lcd.clear();
  lcd.print(message);
}

void failNotity(int buzz, String message) {
  // This is the sound and LED indication of failure
  for (int i = 3; i > 0; i--) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(BUZZER, HIGH);
    delay(buzz);
    digitalWrite(RED_LED, LOW);
    digitalWrite(BUZZER, LOW);
    delay(buzz);
  }
  lcd.clear();
  lcd.print(message);
}

void Reset() {
  finger.emptyDatabase();
  lcd.print("Reset Complete");
}

void numberOfFingers() {
  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    lcd.clear();
    lcd.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  } else {
    lcd.clear();
    lcd.println("Waiting for valid finger...");
    lcd.clear();
    lcd.print("Sensor contains ");
    lcd.setCursor(0, 1);
    lcd.print(finger.templateCount);
    lcd.println(" templates");
  }
}

int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)
    return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)
    return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)
    return -1;

  // found a match!
  Serial.print("Found ID #");
  Serial.print(finger.fingerID);
  Serial.print(" with confidence of ");
  Serial.println(finger.confidence);
  return finger.fingerID;
}

void getUserData() {
  int x = 0;
  while (x == 0) {
    // take the fingerprint
    int userID = getFingerprintIDez();
    x = userID;
    // delay(2000);
    lcd.clear();
    lcd.print("You're user #: ");
    lcd.print(userID);
  }

  // send a get request to the server for anyone with that ID and get all the data in the ID row.
  String user_name = SendFingerprintID(x);
}

// Connecting to WiFi
void connectWiFi() {
  lcd.clear();
  lcd.setCursor(0, 0);
  Serial.print("Connecting to WiFi...       ");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  lcd.setCursor(0, 0);
  lcd.print("WiFi connected          ");
  lcd.setCursor(0, 1);
  lcd.print("IP:");
  lcd.setCursor(4, 1);
  lcd.print(WiFi.localIP());

  delay(1500);
}

// getfingerprint id function
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
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

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // found a match!
  Serial.print("Found ID #");
  Serial.print(finger.fingerID);
  Serial.print(" with confidence of ");
  Serial.println(finger.confidence);

  return finger.fingerID;
}

void ChecktoAddID() {

  HTTPClient http;  //Declare object of class HTTPClient
  //Post Data
  postData = "Get_Fingerid=get_id";  // Add the Fingerprint ID to the Post array in order to send it
  // Post methode

  http.begin(wifiClient, link);                                         //initiate HTTP request, put your Website URL or Your Computer IP
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");  //Specify content-type header

  int httpCode = http.POST(postData);  //Send the request
  String payload = http.getString();   //Get the response payload

  if (payload.substring(0, 6) == "add-id") {
    String add_id = payload.substring(6);
    Serial.println(add_id);
    id = add_id.toInt();
    getFingerprintEnroll();
  }
  http.end();  //Close connection
}

void authenticate(int fingerID) {
  Serial.print("\nAuthenticating FingerID: ");
  Serial.println(fingerID);

  HTTPClient http;  //Declare object of class HTTPClient
  //Post Data
  postData = "auth=" + String(fingerID);  // Add the Fingerprint ID to the Post array in order to send it
  // Post methode

  http.begin(wifiClient, link);                                         //initiate HTTP request, put your Website URL or Your Computer IP
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");  //Specify content-type header

  int httpCode = http.POST(postData);  //Send the request
  String payload = http.getString();   //Get the response payload

  Serial.println("Reponse Code: " + String(httpCode));  //Print HTTP return code
  Serial.println("Payload :" + payload);   //Print request response payload
  Serial.println("Post Data: " + postData);  //Post Data

  String user_name = payload.substring(5);
  Serial.print("Welcome ");
  Serial.println(user_name);
  lcd.print("Welcome ");
  lcd.print(user_name);
  
  delay(750);

  postData = "";
  http.end();  //Close connection
}

String SendFingerprintID(int finger) {
  Serial.print("\nSending FingerID: ");
  Serial.println(finger);

  HTTPClient http;  //Declare object of class HTTPClient
  //Post Data
  postData = "FingerID=" + String(finger);  // Add the Fingerprint ID to the Post array in order to send it
  // Post methode

  http.begin(wifiClient, link);                                         //initiate HTTP request, put your Website URL or Your Computer IP
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");  //Specify content-type header

  int httpCode = http.POST(postData);  //Send the request
  String payload = http.getString();   //Get the response payload

  Serial.println("Reponse Code: " + String(httpCode));  //Print HTTP return code
  Serial.println("Payload :" + payload);   //Print request response payload
  Serial.println("Post Data: " + postData);  //Post Data

  String user_name = "";

  if (payload.substring(0, 5) == "login") {
    user_name = payload.substring(5);
    Serial.print("Welcome ");
    Serial.println(user_name);
    lcd.print("Welcome ");
    lcd.print(user_name);

  } else if (payload.substring(0, 6) == "logout") {
    user_name = payload.substring(6);
    Serial.print("Goodbye ");
    Serial.println(user_name);
    // display.print("Good Bye");
    // display.print(user_name);
  }
  delay(1000);

  postData = "";
  http.end();  //Close connection

  return user_name;
}

void setup() {
  // put your setup code here, to run once:
  // pinMode(RED_LED, OUTPUT);
  // pinMode(GREEN_LED, OUTPUT);
  // pinMode(BUZZER, OUTPUT);
  // pinMode(REGISTER, INPUT);
  // pinMode(AUTHENTICATE, INPUT);

  //initialize the LCD
  // lcd.begin();
  // lcd.backlight(); // turning on the lcd backlight
  // lcd.print("Hello Welcome");

  //Reading from EEPROM
  // var.id = EEPROM.read(0);

  Serial.begin(9600);
  //Setting timeout for the serial
  // Serial.setTimeout(var.timeOutVal);
  // delay(100);
  // Serial.println("\n\nAdafruit Fingerprint sensor enrollment");

  // set the data rate for the sensor serial port
  // finger.begin(57600);

  // if (finger.verifyPassword())
  // {
  //   Serial.println("Found fingerprint sensor!");
  // }
  // else
  // {
  //   Serial.println("Did not find fingerprint sensor :(");
  //   while (1)
  //   {
  //     delay(1);
  //   }
  // }

  // Get fingerprint sensor details
  // Serial.println(F("Reading sensor parameters"));
  // finger.getParameters();
  // Serial.print(F("Status: 0x"));
  // Serial.println(finger.status_reg, HEX);
  // Serial.print(F("Sys ID: 0x"));
  // Serial.println(finger.system_id, HEX);
  // Serial.print(F("Capacity: "));
  // Serial.println(finger.capacity);
  // Serial.print(F("Security level: "));
  // Serial.println(finger.security_level);
  // Serial.print(F("Device address: "));
  // Serial.println(finger.device_addr, HEX);
  // Serial.print(F("Packet len: "));
  // Serial.println(finger.packet_len);
  // Serial.print(F("Baud rate: "));
  // Serial.println(finger.baud_rate);

  // Initializing WiFi connection
  connectWiFi();

  // count fingerprints available
  // finger.getTemplateCount();

  // if (finger.templateCount == 0)
  // {
  //   Serial.print("Sensor doesn't contain any fingerprint data.");
  // }
  // else
  // {
  //   Serial.println("Waiting for valid finger...");
  //   Serial.print("Sensor contains ");
  //   Serial.print(finger.templateCount);
  //   Serial.println(" templates");
  //   // var.id = finger.templateCount;
  // }
}

void loop() {
  // Enroll new User
  // int reg = 0;
  // reg = digitalRead(REGISTER);
  String option = "";
  if (Serial.available() > 0) {
    option = Serial.readString();

    String id = option.substring(1, option.length()-1); // Extract finger ID from entered string
    option = option[0]; // Get option r->register; a->authenticate

    if (option.equals("R") || option.equals("r")) {
      // This function is responsible for registering new members
      Serial.println("Ready to enroll a fingerprint!");
      Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
      // var.id = readnumber();
      // readnumber();
      // finger.getTemplateCount();
      // var.id = finger.templateCount + 1;

      // if (var.id == 0) {  // ID #0 not allowed, try again!
      //   return;
      // }
      // lcd.clear();
      // lcd.print("Enrolling ID #: ");
      // lcd.print(var.id);
      // delay(2500);
      // while (!getFingerprintEnroll());
      // lcd.clear();
      // lcd.print("User ");
      // lcd.print(var.id);
      // lcd.print(" is registered");
      // delay(3000);
      // successNotify(100, "Registered");
    } else {
      // Get fingerprint ID

    // Display fingerprint ID

    // Add fingerprint ID to the Database

    // Delete fingerprint ID from the Database

    // getting user data
    // int auth = digitalRead(AUTHENTICATE);
    if (option.equals("A") || option.equals("a")) {
      
      // getUserData();
      // SendFingerprintID(100);
      authenticate(id.toInt());
      // finger.emptyDatabase();
      // finger.getTemplateCount();
    }

    // if (auth == HIGH && reg == HIGH)
    // {
    //   Reset();
    // }
    }
  }
}
