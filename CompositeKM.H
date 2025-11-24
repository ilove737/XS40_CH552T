
#ifndef __Composite_KM__
#define __Composite_KM__

// extern Ready;
// extern Ep2InKey;
// extern HIDKey;

void USBDeviceInit(void);
// void Enp1IntIn(void);
// void Enp2IntIn(void);
// void HIDValueHandle(void);
// void sendKeyHID(UINT8 *Frames);
void Enp1IntInSend(UINT8 *HIDFrame);
extern UINT8 HIDKey[8];
extern UINT8 HIDMouse[4];

#endif