#include <Adafruit_Fingerprint.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LiquidCrystal_I2C.h>
#include <WiFiClient.h>
#include <Wire.h>


/*******************************
DEFINITIONS
*******************************/
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

String postData;                                                       // post array that will be send to the website
String link = "http://192.168.137.1:8080/biometric/authenticate.php";  //computer IP or the server domain

// Setting up WiFi for pushing data
const char *ssid = "AKORA-ING-DKB";
const char *password = "deekaybee";

Variables var;                       //Structs usage
LiquidCrystal_I2C lcd(0x27, 16, 2);  //LCD definition

#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)  // Setting up the fingerprint sensor
SoftwareSerial mySerial(2, 3);
#endif
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

WiFiClient wifiClient;
HTTPClient http;


/*******************************
FUNCTION PROTOTYPES
*******************************/
void connectWiFi();
uint8_t getFingerprintEnroll();
void registerUser();
void successNotify(int, String);
void failNotity(int, String);
uint8_t getFingerprintID();
void authenticate(int);
void reset();
void promptUser();


/*******************************
FUNCTION DEFINITIONS
*******************************/

// Function to connect the MCU to the Wi-Fi
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

// Function to capture new fingerprint 
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

// Function to save fingerprint and generate finger ID
void registerUser() {
  Serial.println("Ready to enroll a fingerprint!");
  Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
  // var.id = readnumber();
  // readnumber();
  finger.getTemplateCount();
  var.id = finger.templateCount + 1;

  if (var.id == 0) {  // ID #0 not allowed, try again!
    return;
  }
  lcd.clear();
  lcd.print("Enrolling ID #: ");
  lcd.print(var.id);
  delay(2500);
  while (!getFingerprintEnroll())
    ;
  lcd.clear();
  lcd.print("User ");
  lcd.print(var.id);
  lcd.print(" is registered");
  delay(3000);
  successNotify(100, "Registered");
}

// Function to notify of successes
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

// Function to notify of failures
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

// Function to get finger ID of registered users
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return 0;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      lcd.clear();
      lcd.print("Try again");
      return 0;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return 0;
    default:
      Serial.println("Unknown error");
      return 0;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return 0;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      lcd.clear();
      lcd.print("Try again");
      return 0;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return 0;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return 0;
    default:
      Serial.println("Unknown error");
      return 0;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    lcd.clear();
    lcd.print("Try again");
    return 0;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    lcd.clear();
    lcd.print("No match");
    lcd.setCursor(0, 1);
    lcd.print(" found");
    return 0;
  } else {
    Serial.println("Unknown error");
    return 0;
  }

  // found a match!
  Serial.print("Found ID #");
  Serial.print(finger.fingerID);
  Serial.print(" with confidence of ");
  Serial.println(finger.confidence);

  Serial.print("Finger ID: ");
  Serial.println(finger.fingerID);

  return finger.fingerID;
}

// Function to authenticate user with server communication
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

  Serial.println("Post Data: " + postData);             //Post Data
  Serial.println("Reponse Code: " + String(httpCode));  //Print HTTP return code
  Serial.println("Payload :" + payload);                //Print request response payload

  if (httpCode == 200) {
    String user_name = payload.substring(5);
    Serial.print("Welcome ");
    Serial.println(user_name);
    lcd.clear();
    lcd.print("Welcome ");
    lcd.setCursor(4, 1);
    lcd.print(user_name);
  } else {
    lcd.clear();
    lcd.print("No match");
    lcd.setCursor(0, 1);
    lcd.print(" found");
  }

  postData = "";

  delay(250);

  http.end();  //Close connection
}

// Function to clear fingerprint database
void reset() {
  lcd.clear();
  lcd.print("Clearing");
  lcd.setCursor(0, 1);
  lcd.print(" database");

  finger.emptyDatabase();

  delay(1000);
  lcd.clear();
  lcd.print("Reset");
  lcd.setCursor(0, 1);
  lcd.print(" Complete");

  Serial.println("Reset Complete");
}

// Display usage instructions
void promptUser() {
  lcd.clear();
  lcd.print("L: Register");
  lcd.setCursor(0, 1);
  lcd.print("R: Sign In");
}

void setup() {
  // put your setup code here, to run once:
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(REGISTER, INPUT);
  pinMode(AUTHENTICATE, INPUT);

  //initialize the LCD
  lcd.begin();
  lcd.backlight();  // turning on the lcd backlight
  lcd.print("Hello Welcome");

  Serial.begin(9600);
  delay(100);
  Serial.println("\n\nAdafruit Fingerprint sensor enrollment");

  // set the data rate for the sensor serial port
  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (!finger.verifyPassword()) {
      delay(1);
    }
  }

  // Get fingerprint sensor details
  Serial.println("Reading sensor parameters");
  finger.getParameters();
  Serial.print("Status: 0x");
  Serial.println(finger.status_reg, HEX);
  Serial.print("Capacity: ");
  Serial.println(finger.capacity);

  // Initializing WiFi connection
  connectWiFi();

  // Count fingerprints available
  finger.getTemplateCount();
  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data.");
  } else {
    Serial.print("Sensor contains ");
    Serial.print(finger.templateCount);
    Serial.println(" templates");
  }

  delay(100);
}

void loop() {
  // Display operation instructions
  promptUser();
  delay(1000);

  // Read button states
  int reg = digitalRead(REGISTER);
  int auth = digitalRead(AUTHENTICATE);

  // Clear fingerprint database if both are pressed simultaneously
  if (auth == HIGH && reg == HIGH) {
    reset();
  }

  if (reg == HIGH) {
    // Register new member
    registerUser();
  }

  // Authenticate a registered user
  if (auth == HIGH) {
    int thisID = getFingerprintID();
    if (thisID > 0) {
      authenticate(thisID);
      delay(2500);
    }
  }
}
