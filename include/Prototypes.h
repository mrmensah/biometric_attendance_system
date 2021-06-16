#include "Libraries.h"

uint8_t getFingerprintEnroll();
uint8_t downloadFingerprintTemplate(uint16_t id);
void getFingerprintIDRecog();
void successNotify(int, String);
void failNotity(int, String);
void printHex(int num, int precision);