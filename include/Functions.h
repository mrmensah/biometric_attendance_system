#include "Libraries.h"
#include "Variables.h"
#include "Prototypes.h"

//Structs usage
Variables var;
//LCD definition
LiquidCrystal_I2C lcd(0x27, 16, 2);

#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial mySerial(2, 3);
#endif

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

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

uint8_t downloadFingerprintTemplate(uint16_t id)
{
    Serial.println("------------------------------------");
    Serial.print("Attempting to load #");
    Serial.println(id);
    uint8_t p = finger.loadModel(id);
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.print("Template ");
        Serial.print(id);
        Serial.println(" loaded");
        break;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        return p;
    default:
        Serial.print("Unknown error ");
        Serial.println(p);
        return p;
    }

    // OK success!

    Serial.print("Attempting to get #");
    Serial.println(id);
    p = finger.getModel();
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.print("Template ");
        Serial.print(id);
        Serial.println(" transferring:");
        break;
    default:
        Serial.print("Unknown error ");
        Serial.println(p);
        return p;
    }

    // one data packet is 267 bytes. in one data packet, 11 bytes are 'usesless' :D
    uint8_t bytesReceived[534]; // 2 data packets
    memset(bytesReceived, 0xff, 534);

    uint32_t starttime = millis();
    int i = 0;
    while (i < 534 && (millis() - starttime) < 20000)
    {
        if (mySerial.available())
        {
            bytesReceived[i++] = mySerial.read();
        }
    }
    Serial.print(i);
    Serial.println(" bytes read.");
    Serial.println("Decoding packet...");

    uint8_t fingerTemplate[512]; // the real template
    memset(fingerTemplate, 0xff, 512);

    // filtering only the data packets
    int uindx = 9;
    int index = 0;
    while (index < 534)
    {
        while (index < uindx)
            ++index;
        uindx += 256;
        while (index < uindx)
        {
            fingerTemplate[index++] = bytesReceived[index];
        }
        uindx += 2;
        while (index < uindx)
            ++index;
        uindx = index + 9;
    }
    for (int i = 0; i < 512; ++i)
    {
        //Serial.print("0x");
        printHex(fingerTemplate[i], 2);
        //Serial.print(", ");
    }
    Serial.println("\ndone.");

    /*
  uint8_t templateBuffer[256];
  memset(templateBuffer, 0xff, 256);  //zero out template buffer
  int index=0;
  uint32_t starttime = millis();
  while ((index < 256) && ((millis() - starttime) < 1000))
  {
    if (mySerial.available())
    {
      templateBuffer[index] = mySerial.read();
      index++;
    }
  }

  Serial.print(index); Serial.println(" bytes read");

  //dump entire templateBuffer.  This prints out 16 lines of 16 bytes
  for (int count= 0; count < 16; count++)
  {
    for (int i = 0; i < 16; i++)
    {
      Serial.print("0x");
      Serial.print(templateBuffer[count*16+i], HEX);
      Serial.print(", ");
    }
    Serial.println();
  }*/
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

void printHex(int num, int precision)
{
    char tmp[16];
    char format[128];

    sprintf(format, "%%.%dX", precision);

    sprintf(tmp, format, num);
    Serial.print(tmp);
}
