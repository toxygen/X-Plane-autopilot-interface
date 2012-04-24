//
//  control.c
//  X-Plane interface
//
//  Created by Marian Such on 24/4/12.
//  Copyright 2012 FIT CVUT. All rights reserved.
//

#include <string.h>
#include <math.h>
#include <float.h>
#include <unistd.h>
#include "control.h"
#include "ui.h"

#define DOWN_LIMIT -15
#define UP_LIMIT 15

#define K_p 1

static int result;
float roll_deg;


void parse_command(char * input)
{
    if(!strcmp(input, "client: ap\n"))
    {
        ap_on();
    }
    if(!strcmp(input, "client: an\n"))
    {
        ap_off();
    }
}

int ap_on()
{
    XPLMSetDatai(override, 1);
    result = XPLMGetDatai(override);
    if(result)
    {
        printMsg("Autopilot turned on");
        return 1;
    }
    return 0;
}


int ap_off()
{
    XPLMSetDatai(override, 0);
    if(result)
    {
        printMsg("Autopilot turned off");    
        return 1;
    }
    return 0;    
}

/*
 *  Deflect ailerons, use differential
 *  positive turn right
 */
void set_ailerons(float deg)
{
    /* flight model */
    XPLMSetDataf(aileron1_ref, ( deg > 0) ?  0.7 * deg :  deg);  
    XPLMSetDataf(aileron2_ref, (-deg > 0) ? -0.7 * deg : -deg);
    
    /* graphical model */
    XPLMSetDataf(aileron3_ref, ( deg > 0) ?  0.7 * deg :  deg);  
    XPLMSetDataf(aileron4_ref, (-deg > 0) ? -0.7 * deg : -deg);
}


void set_roll(float deg)
{
    roll_deg = deg;
}

/*
 *  Roll at given angle
 */
void ap_roll(float deg)
{
    /* left -, right + */
    static float cur_phi;
    /* TODO make portable for HIL simulation */
    cur_phi = XPLMGetDataf(phi);
    
    float result = K_p * (deg - cur_phi);
    set_ailerons(result);
    /*
    if(cur_phi < deg)
    {
        set_ailerons(2);
    }
    if(cur_phi > deg)
    {
        set_ailerons(-2);
    }
    if(fabs(deg - cur_phi) < FLT_EPSILON)
    {
        set_ailerons(0);
    }*/
}


void * ap_loop()
{
    while(1)
    {
        ap_roll(roll_deg);
        usleep(100000);
    }
}