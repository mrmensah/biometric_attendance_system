#include "Libraries.h"
#include "Prototypes.h"
#include "Functions.h"

void setup()
{
  // put your setup code here, to run once:
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

  // Initializing WiFi connection
  connectWiFi();

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
    // var.id = finger.templateCount;
  }
}

void loop()
{
  // Enroll new User
  int reg = 0;
  reg = digitalRead(REGISTER);
  if (reg == HIGH)
  {
    // This function is responsible for registering new members
    Serial.println("Ready to enroll a fingerprint!");
    Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
    // var.id = readnumber();
    // readnumber();
    finger.getTemplateCount();
    var.id = finger.templateCount + 1;

    if (var.id == 0)
    { // ID #0 not allowed, try again!
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

  // Get fingerprint ID

  // Display fingerprint ID

  // Add fingerprint ID to the Database

  // Delete fingerprint ID from the Database

  // getting user data
  int auth = digitalRead(AUTHENTICATE);
  if (auth == HIGH)
  {
    getUserData();
    // finger.emptyDatabase();
    // finger.getTemplateCount();
   
  }

  if (auth == HIGH && reg == HIGH)
  {
    Reset();
  }
}