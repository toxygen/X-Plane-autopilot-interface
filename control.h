//
//  control.h
//  X-Plane interface
//
//  Created by Marian Such on 24/4/12.
//  Copyright 2012 FIT CVUT. All rights reserved.
//

#include "XPLMDataAccess.h"

extern XPLMDataRef override;
extern XPLMDataRef aileron1_ref;
extern XPLMDataRef aileron2_ref;
extern XPLMDataRef aileron3_ref;
extern XPLMDataRef aileron4_ref;
extern XPLMDataRef phi;
extern XPLMDataRef magpsi;
extern XPLMDataRef elevation;
extern XPLMDataRef rudder1;
extern XPLMDataRef rudder2;
extern XPLMDataRef VVI;
extern XPLMDataRef rudder4;

void parse_command(char *);
int  ap_on();
int  ap_off();
void ap_roll(float);
void set_ailerons(float);
void * ap_loop();
void set_roll(float);
void set_heading(float);
void set_elevation(double);