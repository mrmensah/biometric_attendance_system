#include "Libraries.h"

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
void SendFingerprintID( int finger );
uint8_t getFingerprintID();
uint8_t deleteFingerprint( int id);
void ChecktoDeleteID();
void confirmAdding();
void DisplayFingerprintID();