#include <SPI.h>
#include <Wire.h>
#include <WiFiClient.h> 
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_Fingerprint.h>  //https://github.com/adafruit/Adafruit-Fingerprint-Sensor-Library
#include <LiquidCrystal_I2C.h>
//************************************************************************
//Fingerprint scanner Pins
#define Finger_Rx 2 //
#define Finger_Tx 3 //
//************************************************************************
SoftwareSerial mySerial(Finger_Rx, Finger_Tx);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
//************************************************************************
//LCD init
LiquidCrystal_I2C lcd(0x27, 16, 2);
//************************************************************************
/* Set these to your desired credentials. */
const char *ssid = "Mensah's Nokia";  //ENTER YOUR WIFI SETTINGS
const char *password = "lucille1";
//************************************************************************
String postData ; // post array that will be send to the website
String link = "http://192.168.205.185/biometricattendance/getdata.php"; //computer IP or the server domain
int FingerID = 0;     // The Fingerprint ID from the scanner 
uint8_t id;

WiFiClient wiFiClient;

//Function Prototypes
uint8_t getFingerprintEnroll();
void connectToWiFi();
void DisplayFingerprintID();
void SendFingerprintID( int finger );
int getFingerprintID();
void ChecktoDeleteID();
uint8_t deleteFingerprint( int id);
void confirmAdding();
void connectToWiFi();

void setup() {
 
  Serial.begin(115200);
  //---------------------------------------------
  
  connectToWiFi();
  
  //---------------------------------------------

  lcd.begin();
  //---------------------------------------------
  
  // set the data rate for the sensor serial port
  finger.begin(57600);
  Serial.println("\n\nAdafruit finger detect test");
 
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }
  //---------------------------------------------
  
  finger.getTemplateCount();
  Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  Serial.println("Waiting for valid finger...");
  
  //------------*test the connection*------------
  
  //SendFingerprintID( FingerID );
  
}
//************************************************************************
void loop() {
 
  //check if there's a connection to WiFi or not
  if(WiFi.status() != WL_CONNECTED){
    connectToWiFi();
  }
  //---------------------------------------------
  //If there no fingerprint has been scanned return -1 or -2 if there an error or 0 if there nothing, The ID start form 1 to 127
  FingerID = getFingerprintID();  // Get the Fingerprint ID from the Scanner
  delay(50);            //don't need to run this at full speed.
  
  //---------------------------------------------
  
  DisplayFingerprintID();
  
  //---------------------------------------------
 
  ChecktoAddID();
 
  //---------------------------------------------
  
  ChecktoDeleteID();
 
  //---------------------------------------------
}
//************Display the fingerprint ID state on the OLED*************
void DisplayFingerprintID(){
  //Fingerprint has been detected 
  if (FingerID > 0){
    SendFingerprintID( FingerID ); // Send the Fingerprint ID to the website.
  }
  //---------------------------------------------
  //No finger detected
  else if (FingerID == 0){
    lcd.clear();
    lcd.print("No Finger Detected");
    Serial.print("No Finger Detected");
  }
  //---------------------------------------------
  //Didn't find a match
  else if (FingerID == -1){
    lcd.clear();
   lcd.print("Unrecognized User");
   Serial.print("Unrecognized User");
  }
  //---------------------------------------------
  //Didn't find the scanner or there an error
  else if (FingerID == -2){
    lcd.clear();
    lcd.print("No scanner found");
    Serial.print("No scanner found");
  }
}
//************send the fingerprint ID to the website*************
void SendFingerprintID( int finger ){
  
  HTTPClient http;    //Declare object of class HTTPClient
  //Post Data
  postData = "FingerID=" + String(finger); // Add the Fingerprint ID to the Post array in order to send it
  // Post methode
 
  http.begin(wiFiClient, link); //initiate HTTP request, put your Website URL or Your Computer IP 
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");    //Specify content-type header
  
  int httpCode = http.POST(postData);   //Send the request
  String payload = http.getString();    //Get the response payload
  
  Serial.println(httpCode);   //Print HTTP return code
  Serial.println(payload);    //Print request response payload
  Serial.println(postData);   //Post Data
  Serial.println(finger);     //Print fingerprint ID
 
  if (payload.substring(0, 5) == "login") {
    String user_name = payload.substring(5);
    Serial.print("Welcome ");
    Serial.println(user_name);  
    lcd.clear();        
    lcd.print("Welcome ");
    lcd.print(user_name);
    
  }
  else if (payload.substring(0, 6) == "logout") {
    String user_name = payload.substring(6);
    Serial.print("Goodbye ");
    Serial.println(user_name);
    lcd.clear();
    lcd.print("Good Bye");
    lcd.setCursor(0,1);
    lcd.print(user_name);
  }
  delay(1000);
  
  postData = "";
  http.end();  //Close connection
}
//********************Get the Fingerprint ID******************
int getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      //Serial.println("No finger detected");
      return 0;
    case FINGERPRINT_PACKETRECIEVEERR:
      //Serial.println("Communication error");
      return -2;
    case FINGERPRINT_IMAGEFAIL:
      //Serial.println("Imaging error");
      return -2;
    default:
      //Serial.println("Unknown error");
      return -2;
  }
  // OK success!
  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      //Serial.println("Image too messy");
      return -1;
    case FINGERPRINT_PACKETRECIEVEERR:
      //Serial.println("Communication error");
      return -2;
    case FINGERPRINT_FEATUREFAIL:
      //Serial.println("Could not find fingerprint features");
      return -2;
    case FINGERPRINT_INVALIDIMAGE:
      //Serial.println("Could not find fingerprint features");
      return -2;
    default:
      //Serial.println("Unknown error");
      return -2;
  }
  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    //Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    //Serial.println("Communication error");
    return -2;
  } else if (p == FINGERPRINT_NOTFOUND) {
    //Serial.println("Did not find a match");
    return -1;
  } else {
    //Serial.println("Unknown error");
    return -2;
  }   
  // found a match!
  //Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  //Serial.print(" with confidence of "); Serial.println(finger.confidence); 
 
  return finger.fingerID;
}
//******************Check if there a Fingerprint ID to delete******************
void ChecktoDeleteID(){
 
  HTTPClient http;    //Declare object of class HTTPClient
  //Post Data
  postData = "DeleteID=check"; // Add the Fingerprint ID to the Post array in order to send it
  // Post methode
 
  http.begin(wiFiClient, link); //initiate HTTP request, put your Website URL or Your Computer IP 
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");    //Specify content-type header
  
  int httpCode = http.POST(postData);   //Send the request
  String payload = http.getString();    //Get the response payload
 
  if (payload.substring(0, 6) == "del-id") {
    String del_id = payload.substring(6);
    Serial.println(del_id);
    deleteFingerprint( del_id.toInt() );
  }
  
  http.end();  //Close connection
}
//******************Delete Finpgerprint ID*****************
uint8_t deleteFingerprint( int id) {
  uint8_t p = -1;
  
  p = finger.deleteModel(id);
 
  if (p == FINGERPRINT_OK) {
    Serial.println("Deleted!");
    lcd.clear();
    lcd.print("Deleted!\n");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    lcd.clear();
    lcd.print("Communication error!\n");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not delete in that location");
    lcd.clear();
    lcd.print(F("Could not delete in that location!\n"));
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    lcd.clear();
    lcd.print("Error writing to flash!\n");
    return p;
  } else {
    Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
    lcd.clear();
    lcd.print("Unknown error:\n");
    return p;
  }   
}
//******************Check if there a Fingerprint ID to add******************
void ChecktoAddID(){
 
  HTTPClient http;    //Declare object of class HTTPClient
  //Post Data
  postData = "Get_Fingerid=get_id"; // Add the Fingerprint ID to the Post array in order to send it
  // Post methode
 
  http.begin(wiFiClient, link); //initiate HTTP request, put your Website URL or Your Computer IP 
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");    //Specify content-type header
  
  int httpCode = http.POST(postData);   //Send the request
  String payload = http.getString();    //Get the response payload
 
  if (payload.substring(0, 6) == "add-id") {
    String add_id = payload.substring(6);
    Serial.println(add_id);
    id = add_id.toInt();
    getFingerprintEnroll();
  }
  http.end();  //Close connection
}
//******************Enroll a Finpgerprint ID*****************
uint8_t getFingerprintEnroll() {
 
  int p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      lcd.clear();
      lcd.print("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
//      lcd.clear();
//      lcd.print(".");
//      lcd.print("scanning");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
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
 
  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      lcd.clear();
      lcd.println("Image converted");
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      lcd.clear();
       lcd.println("Image too messy");
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
  lcd.clear();
  lcd.print("Remove finger");
  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  lcd.print("ID "); lcd.println(id);
  p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      lcd.clear();
      lcd.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      Serial.print("scanning");
//      lcd.clear();
//      lcd.println(".");
//      lcd.print("scanning");
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
  Serial.print("Creating model for #");  Serial.println(id);
  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
    lcd.println("Prints matched!");
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
  
  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    lcd.clear();
    lcd.println("Stored!");
    confirmAdding();
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
}
//******************Check if there a Fingerprint ID to add******************
void confirmAdding(){
 
  HTTPClient http;    //Declare object of class HTTPClient
  //Post Data
  postData = "confirm_id=" + String(id); // Add the Fingerprint ID to the Post array in order to send it
  // Post methode
 
  http.begin(wiFiClient, link); //initiate HTTP request, put your Website URL or Your Computer IP 
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");    //Specify content-type header
  
  int httpCode = http.POST(postData);   //Send the request
  String payload = http.getString();    //Get the response payload
 
  Serial.println(payload);
  
  http.end();  //Close connection
}
//********************connect to the WiFi******************
void connectToWiFi(){
    WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
    delay(1000);
    WiFi.mode(WIFI_STA);
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
 
    lcd.clear();
    lcd.print("Connecting to \n");
    lcd.print(ssid);
    
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("Connected");

    lcd.clear();
    lcd.print("Connected \n");
    
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());  //IP address assigned to your ESP

    lcd.clear();
    lcd.print("IP address: ");
    lcd.println(WiFi.localIP());
 
}
