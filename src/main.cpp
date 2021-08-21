#include "Libraries.h"
#include "Prototypes.h"
#include "Functions.h"

void setup()
{
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(REGISTER, INPUT);
  pinMode(AUTHENTICATE, INPUT);

  //initialize the LCD
  lcd.begin();
  lcd.backlight(); // turning on the lcd backlight
  lcd.print("Hello Welcome");

  //Reading from EEPROM
  // var.id = EEPROM.read(0);

  Serial.begin(9600);
  //Setting timeout for the serial
  Serial.setTimeout(var.timeOutVal);
  delay(100);
  Serial.println("\n\nAdafruit Fingerprint sensor enrollment");

  // set the data rate for the sensor serial port
  finger.begin(57600);

  // Initializing WiFi connection
  connectWiFi();

  if (finger.verifyPassword())
  {
    Serial.println("Found fingerprint sensor!");
  }
  else
  {
    Serial.println("Did not find fingerprint sensor :(");
    while (1)
    {
      delay(1);
    }
  }

  // Get fingerprint sensor details
  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x"));
  Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x"));
  Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: "));
  Serial.println(finger.capacity);
  Serial.print(F("Security level: "));
  Serial.println(finger.security_level);
  Serial.print(F("Device address: "));
  Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: "));
  Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: "));
  Serial.println(finger.baud_rate);

  // count fingerprints available
  finger.getTemplateCount();

  if (finger.templateCount == 0)
  {
    Serial.print("Sensor doesn't contain any fingerprint data.");
  }
  else
  {
    Serial.println("Waiting for valid finger...");
    Serial.print("Sensor contains ");
    Serial.print(finger.templateCount);
    Serial.println(" templates");
  }
}

void loop()
{
  // Enroll new User
  // Buttons setup
  int reg =  digitalRead(REGISTER);
  int auth = digitalRead(AUTHENTICATE);

  // Registering new members
  if (reg == HIGH)
  {
    Serial.println("Ready to enroll a fingerprint!");
    // Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
    // var.id = readnumber();
    // readnumber();
    finger.getTemplateCount();
    var.id = finger.templateCount + 1; // Assigning the ID automatically

    if (var.id == 0) // ID #0 not allowed, try again!
    { 
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

  
  // Authenticating the user 
  if (auth == HIGH)
  {
    getUserData();
  }

  
  if (auth == HIGH && reg == HIGH)
  {
    Reset();
  }
}