
#ifndef _CONSTANTS_H
#define _CONSTANTS_H

#include <OneWire.h>
#include <Dallastemperature.h>

//
// These constants are not meant to be changed, unless the harddware or wiring changes
//

#define REVISION 1

// uncomment following line if using different set of sensors fortesting
//#define TESTING

#ifdef TESTING
const DeviceAddress HeatExInAddr = { 0x28, 0xFF, 0xD2, 0x6C, 0xC1, 0x16, 0x04, 0xC8 };
const DeviceAddress HeatExOutAddr = { 0x28, 0xFF, 0x6A, 0x93, 0xB5, 0x16, 0x03, 0xA0 };
const DeviceAddress HWC_TopAddr = { 0x28, 0xFF, 0xE6, 0x6C, 0xC1, 0x16, 0x04, 0x53 };
const DeviceAddress HWC_BottomAddr = { 0x28, 0xFF, 0x49, 0x23, 0xB5, 0x16, 0x05, 0x5A };

#else
// heat exchanger input and output temperature sensors
const DeviceAddress HeatExInAddr = { 0x28, 0xFF, 0xA7, 0x67, 0x70, 0x17, 0x03, 0xB3 }; // A1-2
const DeviceAddress HeatExOutAddr = { 0x28, 0xFF, 0x8C, 0x25, 0x71, 0x17, 0x03, 0x77 }; // A1-4

// hot-water cylinder top and bottom temperature sensors
const DeviceAddress HWC_TopAddr = { 0x28, 0xFF, 0x56, 0xE9, 0x70, 0x17, 0x03, 0x14 }; // A1-1
const DeviceAddress HWC_BottomAddr = { 0x28, 0xFF, 0x70, 0x20, 0x71, 0x17, 0x03, 0x71 }; // A1-3
#endif

// outputs
const int heaterPin = 3;      // relay
const int compressorPin = 10; // relay
const int pumpPin = 13;       // solid-state relay

const float dutyMinimum = 0.2; // keep water flowing so temperature can be measured
const float dutyMaximum = 0.99; // avoid 1.0 as it makes relay logic misbehave
const unsigned int periodMsec = 1000;

#endif
