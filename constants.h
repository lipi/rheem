
#ifndef _CONSTANTS_H
#define _CONSTANTS_H

#include <OneWire.h>
#include <Dallastemperature.h>

#define REVISION 1

// heat exchanger input and output temperature sensors
const DeviceAddress HeatExInAddr = { 0x28, 0xFF, 0xA7, 0x67, 0x70, 0x17, 0x03, 0xB3 }; // A1-2
const DeviceAddress HeatExOutAddr = { 0x28, 0xFF, 0x8C, 0x25, 0x71, 0x17, 0x03, 0x77 }; // A1-4

// hot-water cylinder top and bottom temperature sensors
const DeviceAddress HWC_TopAddr = { 0x28, 0xFF, 0x56, 0xE9, 0x70, 0x17, 0x03, 0x14 }; // A1-1
const DeviceAddress HWC_BottomAddr = { 0x28, 0xFF, 0x70, 0x20, 0x71, 0x17, 0x03, 0x71 }; // A1-3

// outputs
const int heaterPin = 3;      // relay
const int compressorPin = 10; // relay
const int pumpPin = 13;       // solid-state relay

#endif
