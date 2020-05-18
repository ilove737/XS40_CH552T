
#ifndef Composite_KM
#define Composite_KM

// extern Ready;
// extern Ep2InKey;
// extern HIDKey;

void USBDeviceInit();
void Enp1IntIn();
void Enp2IntIn();
void HIDValueHandle();
void sendKeyHID(UINT8 *Frames);

#endif