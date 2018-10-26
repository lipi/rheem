
#ifndef _PARAMETERS_H
#define _PARAMETERS_H

//
// values here are subject to tuning
//

const float dutyMinimum = 0.2; // keep water flowing so temperature can be measured
const float dutyMaximum = 0.99; // avoid 1.0 as it makes relay logic misbehave
const unsigned int periodMsec = 1000;

#endif
