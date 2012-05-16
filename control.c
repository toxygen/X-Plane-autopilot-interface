//
//  control.c
//  X-Plane interface
//
//  Created by Marian Such on 24/4/12.
//  Copyright 2012 FIT CVUT. All rights reserved.
//

#include <stdio.h>

#include <string.h>
#include <math.h>
#include <float.h>
#include <unistd.h>
#include "control.h"
#include "ui.h"

#define DOWN_LIMIT -5.0F
#define UP_LIMIT    5.0F
#define DIFF_RATIO  0.7

#define MAX_ELEVATOR  24
#define MIN_ELEVATOR -24

#define CLIMB_LIMIT     800
#define DESCEND_LIMIT  -800


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
        left  = DIFF_RATIO * deg;
        right = -deg;
        
        /* enforce limits */
        if(left  > UP_LIMIT)   { left  = UP_LIMIT;   }
        if(right < DOWN_LIMIT) { right = DOWN_LIMIT; }            
    }

    /* turning right */
    else
    {
        left  = deg;
        right = DIFF_RATIO * -deg;
        
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

float get_heading()
{
    return XPLMGetDataf(magpsi);
}

/*
 * Set desired heading
 */
void set_heading(float deg)
{
    heading = deg;
}

double get_elevation()
{
    return XPLMGetDatad(elevation);
}

/*
 * Set desired altitude in meters
 */

void set_elevation(double meters)
{
    elev_meters = meters;
}

/*
 * Set desired roll angle 
 */
void set_roll(float deg)
{
    roll_deg = deg;
}

void set_elevator(float degrees)
{
    // 
    if(degrees > MAX_ELEVATOR) { degrees =  MAX_ELEVATOR; }
    if(degrees < MIN_ELEVATOR) { degrees =  MIN_ELEVATOR; }
    XPLMSetDataf(rudder1, degrees);
    XPLMSetDataf(rudder2, degrees);
//    XPLMSetDataf(rudder3, degrees);
//    XPLMSetDataf(rudder4, degrees);
}

/*
 * Control climb rate
 */
void ap_climb(float feet)
{
    static float Kp = 0.005f;
    static float error;
    static float signal;
    static float rate;
    
    if(feet >  CLIMB_LIMIT)   { feet = CLIMB_LIMIT;   }
    if(feet <  DESCEND_LIMIT) { feet = DESCEND_LIMIT; }
    
    rate  = XPLMGetDataf(VVI);
    error = rate - feet;
    
    // climbing limits
    if(error >  CLIMB_LIMIT)   { error = CLIMB_LIMIT;   }
    if(error <  DESCEND_LIMIT) { error = DESCEND_LIMIT; }
    
    signal = Kp * error;
    
    set_elevator(signal);
}

/*
 * Control elevator to reach altitude
 */
void ap_elev(float meters)
{
    static double Kp = 50.0;
    static double error;
    static double signal;
    static double cur_elev;
    
    cur_elev = XPLMGetDatad(elevation);
    error    = cur_elev - meters;
    signal   = -Kp * error;
    
    ap_climb(signal);
}

/*
 * Return current roll
 */
float get_roll()
{
    return XPLMGetDataf(phi);
}

/*
 * Roll at given angle
 */
void ap_roll(float deg)
{
    /* left -, right + */
    static float cur_phi;
    static float signal;
    static float error;
    static float Kp = 1.0f;
    
    cur_phi = get_roll();
    error   = deg - cur_phi;
    signal  = Kp * error;
    
    if(cur_phi >  20.0F) { result = -5; }
    if(cur_phi < -20.0F) { result =  5; }

    set_ailerons(signal);
}

void ap_heading(float deg)
{
    static float cur_magpsi;
    static float error;
    static float signal;
    static float Kp = 1.0f;

    cur_magpsi = get_heading();
    error      = heading - cur_magpsi;
    signal     = Kp * error;
    
    //    printf("heading %f error %f magpsi %f\n", heading, error, cur_magpsi);
    
    ap_roll(signal);
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