#include "Libraries.h"
#include "Variables.h"
#include "Prototypes.h"

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
const char *ssid = "Mensah's Nokia";
const char *password = "lucille1";
const char *host = ""; // host server/ the php server address

WiFiClient wiFiClient;

uint8_t readnumber(void)
{
    uint8_t num = 0;

    while (num == 0)
    {
        while (!Serial.available())
            ;
        num = Serial.parseInt();
    }
    return num;
}

/* Enrolling a new person */
uint8_t getFingerprintEnroll()
{
    int p = -1;
    lcd.scrollDisplayLeft();
    lcd.print("Waiting for valid finger to enroll as #");
    lcd.print(var.id);
    while (p != FINGERPRINT_OK)
    {
        p = finger.getImage();
        switch (p)
        {
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
    switch (p)
    {
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
    while (p != FINGERPRINT_NOFINGER)
    {
        p = finger.getImage();
    }
    Serial.print("ID ");
    Serial.println(var.id);
    p = -1;
    lcd.clear();
    lcd.println("Place same finger again");
    while (p != FINGERPRINT_OK)
    {
        p = finger.getImage();
        switch (p)
        {
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
    switch (p)
    {
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
    if (p == FINGERPRINT_OK)
    {
        Serial.println("Prints matched!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial.println("Communication error");
        return p;
    }
    else if (p == FINGERPRINT_ENROLLMISMATCH)
    {
        Serial.println("Fingerprints did not match");
        return p;
    }
    else
    {
        Serial.println("Unknown error");
        return p;
    }

    Serial.print("ID ");
    Serial.println(var.id);
    p = finger.storeModel(var.id);
    if (p == FINGERPRINT_OK)
    {
        Serial.println("Stored!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial.println("Communication error");
        return p;
    }
    else if (p == FINGERPRINT_BADLOCATION)
    {
        Serial.println("Could not store in that location");
        return p;
    }
    else if (p == FINGERPRINT_FLASHERR)
    {
        Serial.println("Error writing to flash");
        return p;
    }
    else
    {
        Serial.println("Unknown error");
        return p;
    }

    return true;
}

void successNotify(int buzz, String message)
{
    // This is the sound and LED indication when a operation is successful
    for (int i = 3; i > 0; i--)
    {
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

void failNotity(int buzz, String message)
{
    // This is the sound and LED indication of failure
    for (int i = 3; i > 0; i--)
    {
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

void Reset()
{
    finger.emptyDatabase();
    lcd.print("Reset Complete");
}

void numberOfFingers()
{
    finger.getTemplateCount();

    if (finger.templateCount == 0)
    {
        lcd.clear();
        lcd.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
    }
    else
    {
        lcd.clear();
        lcd.println("Waiting for valid finger...");
        lcd.clear();
        lcd.print("Sensor contains ");
        lcd.setCursor(0, 1);
        lcd.print(finger.templateCount);
        lcd.println(" templates");
    }
}

int getFingerprintIDez()
{
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

void getUserData()
{
    int x = 0;

    do
    {
        // take the fingerprint
        int userID = getFingerprintID();
        // delay(2000);
        lcd.clear();
        lcd.print("You're user #: ");
        lcd.print(userID);
        SendFingerprintID(userID);
        if (userID != 0)
        {
            x = 1; // Take this out incase of emergency :)
        }
    } while (x == 0);

    // send a get request to the server for anyone with that ID and get all the data in the ID row.
}

// Connecting to WiFi
void connectWiFi()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    Serial.print("Connecting to WiFi...       ");
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
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
uint8_t getFingerprintID()
{
    uint8_t p = finger.getImage();
    switch (p)
    {
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
    switch (p)
    {
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
    if (p == FINGERPRINT_OK)
    {
        Serial.println("Found a print match!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial.println("Communication error");
        return p;
    }
    else if (p == FINGERPRINT_NOTFOUND)
    {
        Serial.println("Did not find a match");
        return p;
    }
    else
    {
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

void ChecktoAddID()
{

    HTTPClient http; //Declare object of class HTTPClient
    //Post Data
    postData = "Get_Fingerid=get_id"; // Add the Fingerprint ID to the Post array in order to send it
    // Post methode

    http.begin(wiFiClient, link);                                        //initiate HTTP request, put your Website URL or Your Computer IP
    http.addHeader("Content-Type", "application/x-www-form-urlencoded"); //Specify content-type header

    int httpCode = http.POST(postData); //Send the request
    String payload = http.getString();  //Get the response payload

    if (payload.substring(0, 6) == "add-id")
    {
        String add_id = payload.substring(6);
        Serial.println(add_id);
        id = add_id.toInt();
        getFingerprintEnroll();
    }
    http.end(); //Close connection
}

void SendFingerprintID(int finger)
{

    HTTPClient http; //Declare object of class HTTPClient
    //Post Data
    postData = "FingerID=" + String(finger); // Add the Fingerprint ID to the Post array in order to send it
    // Post methode

    http.begin(wiFiClient, link);                                        //initiate HTTP request, put your Website URL or Your Computer IP
    http.addHeader("Content-Type", "application/x-www-form-urlencoded"); //Specify content-type header

    int httpCode = http.POST(postData); //Send the request
    String payload = http.getString();  //Get the response payload

    Serial.println(httpCode); //Print HTTP return code
    Serial.println(payload);  //Print request response payload
    Serial.println(postData); //Post Data
    Serial.println(finger);   //Print fingerprint ID

    if (payload.substring(0, 5) == "login")
    {
        String user_name = payload.substring(5);
        Serial.print("Welcome ");
        Serial.println(user_name);
        lcd.clear();
        lcd.print("Welcome ");
        lcd.print(user_name);
    }
    else if (payload.substring(0, 6) == "logout")
    {
        String user_name = payload.substring(6);
        Serial.print("Goodbye ");
        Serial.println(user_name);
        lcd.clear();
        lcd.print("Good Bye");
        lcd.setCursor(0, 1);
        lcd.print(user_name);
    }
    delay(1000);

    postData = "";
    http.end(); //Close connection
}

void DisplayFingerprintID()
{
    //Fingerprint has been detected
    if (FingerID > 0)
    {
        SendFingerprintID(FingerID); // Send the Fingerprint ID to the website.
    }
    //---------------------------------------------
    //No finger detected
    else if (FingerID == 0)
    {
        lcd.clear();
        lcd.print("No Finger Detected");
        Serial.print("No Finger Detected");
    }
    //---------------------------------------------
    //Didn't find a match
    else if (FingerID == -1)
    {
        lcd.clear();
        lcd.print("Unrecognized User");
        Serial.print("Unrecognized User");
    }
    //---------------------------------------------
    //Didn't find the scanner or there an error
    else if (FingerID == -2)
    {
        lcd.clear();
        lcd.print("No scanner found");
        Serial.print("No scanner found");
    }
}

void ChecktoDeleteID()
{

    HTTPClient http; //Declare object of class HTTPClient
    //Post Data
    postData = "DeleteID=check"; // Add the Fingerprint ID to the Post array in order to send it
    // Post methode

    http.begin(wiFiClient, link);                                        //initiate HTTP request, put your Website URL or Your Computer IP
    http.addHeader("Content-Type", "application/x-www-form-urlencoded"); //Specify content-type header

    int httpCode = http.POST(postData); //Send the request
    String payload = http.getString();  //Get the response payload

    if (payload.substring(0, 6) == "del-id")
    {
        String del_id = payload.substring(6);
        Serial.println(del_id);
        deleteFingerprint(del_id.toInt());
    }

    http.end(); //Close connection
}

uint8_t deleteFingerprint(int id)
{
    uint8_t p = -1;

    p = finger.deleteModel(id);

    if (p == FINGERPRINT_OK)
    {
        Serial.println("Deleted!");
        lcd.clear();
        lcd.print("Deleted!");
        return p;
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial.println("Communication error");
        lcd.clear();
        lcd.print("Communication error!\n");
        return p;
    }
    else if (p == FINGERPRINT_BADLOCATION)
    {
        Serial.println("Could not delete in that location");
        lcd.clear();
        lcd.print(F("Could not delete in that location!\n"));
        return p;
    }
    else if (p == FINGERPRINT_FLASHERR)
    {
        Serial.println("Error writing to flash");
        lcd.clear();
        lcd.print("Error writing to flash!\n");
        return p;
    }
    else
    {
        Serial.print("Unknown error: 0x");
        Serial.println(p, HEX);
        lcd.clear();
        lcd.print("Unknown error:\n");
        return p;
    }
}

void confirmAdding()
{

    HTTPClient http; //Declare object of class HTTPClient
    //Post Data
    postData = "confirm_id=" + String(id); // Add the Fingerprint ID to the Post array in order to send it
    // Post methode

    http.begin(wiFiClient, link);                                        //initiate HTTP request, put your Website URL or Your Computer IP
    http.addHeader("Content-Type", "application/x-www-form-urlencoded"); //Specify content-type header

    int httpCode = http.POST(postData); //Send the request
    String payload = http.getString();  //Get the response payload

    Serial.println(payload);

    http.end(); //Close connection
}