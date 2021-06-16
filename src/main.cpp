#include "Libraries.h"
#include "Prototypes.h"
// #include "Variables.h"
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
  lcd.print("Hello Meow");

  //Reading from EEPROM
  var.id = EEPROM.read(0);

  Serial.begin(9600);
  // while (!Serial);  // For Yun/Leo/Micro/Zero/...
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

  // Try to get the templates for fingers 1 through 10
  // for (int finger = 1; finger < 10; finger++) {
  //   downloadFingerprintTemplate(finger);
  // }

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
}

void loop()
{
  int reg = 0;
  reg = digitalRead(REGISTER);
  if (reg == HIGH)
  {
    // This funtion is responsible for registering new members
    Serial.println("Ready to enroll a fingerprint!");
    Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
    // id = readnumber();
    // readnumber();
    var.id += 1;

    if (var.id == 0)
    { // ID #0 not allowed, try again!
      return;
    }
    lcd.clear();
    lcd.print("Enrolling ID #: ");
    lcd.print(var.id);

    while (!getFingerprintEnroll())
      ;
    EEPROM.write(0, var.id);
    lcd.clear();
    lcd.print("User ");
    lcd.print(var.id);
    lcd.print(" is registered");
    delay(3000);
    successNotify(100, "Registered");
  }
  // testing failure
  int auth = digitalRead(AUTHENTICATE);
  if (auth == HIGH)
  {
    for (int finger = 1; finger < 10; finger++)
    {
      downloadFingerprintTemplate(finger);
    }
    failNotity(100, "Failed to register");
  }
}