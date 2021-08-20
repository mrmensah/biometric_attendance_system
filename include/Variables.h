// Pin declaration
#include <Arduino.h>

#define RED_LED 12
#define GREEN_LED 14
#define BUZZER 0
#define REGISTER 15     // To register
#define AUTHENTICATE 13 // To mark attendance

struct Variables
{
    /* data */
    uint8_t id;
    int wait{200};
    int successBuzz{100};
    int timeOutVal{10000};
    int failureBuzz{1000};
};

String postData ; // post array that will be send to the website
String link = "http://192.168.205.185/biometricattendance/getdata.php"; //computer IP or the server domain
int FingerID = 0;     // The Fingerprint ID from the scanner 
uint8_t id;
