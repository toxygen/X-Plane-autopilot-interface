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

#define DOWN_LIMIT -5.0F
#define UP_LIMIT 5.0F

#define K_p 1

static int result;
float roll_deg;
float heading;
double elev_meters;


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

/*
 * Allow autopilot to control the plane
 */
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

/*
 * Return control to pilot
 */
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
    static float left  = 0;
    static float right = 0;

    /* turning left */
    if(deg > 0)
    {
        left  = 0.7 * deg;
        right = -deg;
        
        /* enforce limits */
        if(left  > UP_LIMIT)   { left  = UP_LIMIT;   }
        if(right < DOWN_LIMIT) { right = DOWN_LIMIT; }            
    }

    /* turning right */
    else
    {
        left  = deg;
        right = 0.7 * -deg;
        
        /* enforce limits */
        if(left  < DOWN_LIMIT)  { left  = DOWN_LIMIT; }
        if(right > UP_LIMIT)    { right = UP_LIMIT;   }
    }
    
    /* flight model */
    XPLMSetDataf(aileron1_ref, left);
    XPLMSetDataf(aileron2_ref, right);
    
    /* graphical model */
    XPLMSetDataf(aileron3_ref, left);
    XPLMSetDataf(aileron4_ref, right);
}

/*
 * set desired heading
 */
void set_heading(float deg)
{
    heading = deg;
}

/*
 * set desired altitude in meters
 */

void set_elevation(double meters)
{
    elev_meters = meters;
}

/*
 * set desired roll angle 
 */
void set_roll(float deg)
{
    roll_deg = deg;
}

void set_elevator(float degrees)
{
    if(degrees >  5) { degrees =  5; }
    if(degrees < -5) { degrees = -5; }
    XPLMSetDataf(rudder1, degrees);
    XPLMSetDataf(rudder2, degrees);
    XPLMSetDataf(rudder3, degrees);
    XPLMSetDataf(rudder4, degrees);
}

void ap_elev(float meters)
{
    static double error;
    static double last = 0;
    //error = elev_meters - XPLMGetDatad(elevation);
   /* if(XPLMGetDatad(elevation));
    error = last - XPLMGetDatad(elevation);
    if(error > 20) { error = -2; }
    else
    if(error < 0)  { error = 5; }
    else { error = 0; }

    last = XPLMGetDatad(elevation);
    
    set_elevator(error);*/
}

/*
 * Roll at given angle
 */
void ap_roll(float deg)
{
    /* left -, right + */
    static float cur_phi;
    static float result;
    /* TODO make portable for HIL simulation */
    cur_phi = XPLMGetDataf(phi);
    
    result = K_p * (deg - cur_phi);
    set_ailerons(result);
}

void ap_heading(float deg)
{
    static float cur_magpsi;
    static float error = 0;
    cur_magpsi = XPLMGetDataf(magpsi);
    error = heading - cur_magpsi;
    
    ap_roll(error);
}

void * ap_loop()
{
    while(1)
    {
        ap_heading(heading);
        ap_elev(elev_meters);
        usleep(100000);
    }
}