#include "Libraries.h"

uint8_t getFingerprintEnroll();
uint8_t readnumber(void);
uint8_t deleteFingerprint(uint8_t id);
void numberOfFingers();
void getFingerprintIDRecog();
void successNotify(int, String);
void failNotity(int, String);
void printHex(int num, int precision);
void Reset();

int getFingerprintIDez();
void getUserData();

void connectWiFi();