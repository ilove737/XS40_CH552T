
#ifndef __Composite_KM__
#define __Composite_KM__

// extern Ready;
// extern Ep2InKey;
// extern HIDKey;

void USBDeviceInit();
// void Enp1IntIn();
// void Enp2IntIn();
// void HIDValueHandle();
// void sendKeyHID(UINT8 *Frames);
void Enp1IntInSend(UINT8 *HIDFrame);

#endif