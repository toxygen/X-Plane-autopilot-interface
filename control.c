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


// ----
#include <sys/time.h>  

struct timeval t1, t2;
double elapsedTime;
// -----

#define DOWN_LIMIT -5.0f
#define UP_LIMIT    5.0f
#define DIFF_RATIO  0.7

#define MAX_ELEVATOR  24
#define MIN_ELEVATOR -24

#define CLIMB_LIMIT     800
#define DESCEND_LIMIT  -800
#define MAX_ROLL 20

#define FREQ_TIME 100000 // microseconds = 0.1s = 10Hz
#define FREQ      (1000000.0f / FREQ_TIME) 
#define DT        (FREQ_TIME / 100000.0f)

#define OUT_COEF_ELEV 0.24f
#define OUT_COEF_AIL  0.05f

static int result;
float  roll_deg;
float  heading;
double elev_meters;

int    ap = 0;


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
        ap = 1;
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
        ap = 0;
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
    static float left  = 0.0f;
    static float right = 0.0f;

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

    left  = (left  / UP_LIMIT) * 100.0f * OUT_COEF_AIL;
    right = (right / UP_LIMIT) * 100.0f * OUT_COEF_AIL;
    
    /* flight model */
    XPLMSetDataf(aileron1_ref, left);
    XPLMSetDataf(aileron2_ref, right);
    
    /* graphical model */
    XPLMSetDataf(aileron3_ref, left);
    XPLMSetDataf(aileron4_ref, right);
}

/*
 * Return current heading
 */
float get_heading()
{
    float cur_psi;
    cur_psi = XPLMGetDataf(magpsi);
    if(cur_psi > 360.0f) { cur_psi = fmod(cur_psi, 360.0f); }
    if(cur_psi < 0.0f)   { cur_psi = fmod(cur_psi, 360.0f) + 360.0f; }
    return cur_psi;
}

/*
 * Set desired heading
 */
void set_heading(float deg)
{
    if(deg > 360.0f) { deg = fmod(deg, 360.0f); }
    if(deg < 0.0f)   { deg = 360.0f + fmod(deg, 360.0f); }
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
    degrees = (degrees / MAX_ELEVATOR) * 100 * OUT_COEF_ELEV;
    
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
    
    if(feet > CLIMB_LIMIT)   { feet = CLIMB_LIMIT;   }
    if(feet < DESCEND_LIMIT) { feet = DESCEND_LIMIT; }
    
    rate  = XPLMGetDataf(VVI);
    error = rate - feet;
    
    // climbing limits
    if(error > CLIMB_LIMIT)   { error = CLIMB_LIMIT;   }
    if(error < DESCEND_LIMIT) { error = DESCEND_LIMIT; }
    
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
    float roll;
    roll = XPLMGetDataf(phi);
    if(roll > 180.0f) 
    { 
        roll  = fmod(roll, 180.0f);
        roll -= 180.0f; 
    }
    if(roll < -180.0f) 
    { 
        roll  = fmod(roll, 180.0f);
        roll += 180.0f; 
    }
    return roll;
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
    static int count = 0;
    
    if(deg >  MAX_ROLL) { deg =  MAX_ROLL; }
    if(deg < -MAX_ROLL) { deg = -MAX_ROLL; }
    
    cur_phi = get_roll();
    error   = deg - cur_phi;
    signal  = Kp * error;
    
    if(cur_phi >  20.0F) { result = -5; }
    if(cur_phi < -20.0F) { result =  5; }

    set_ailerons(signal);
    if(!ap) { count = 0; }
//    printf("%d %f\n", count, cur_phi);
    count++;
}

void ap_heading(float deg)
{
    static float cur_magpsi;
    static float error;
    static float signal;
    static float derivative;
    static float integral = 0;
    static float last = 0;
    static float Kp = 1.0f;
    static float Kd = 10.0f;
    static float Ki = 0.01f;
    static int   count = 0;
    
    cur_magpsi = get_heading();
    error      = deg - cur_magpsi;
    
    derivative = (cur_magpsi - last) / DT;
    last = cur_magpsi;
    
    // integrate only if autopilot turned on
    if(!ap) 
    {
        integral = 0;
        count = 0;
    }
    else
    {
        integral += DT * fmod(error, 2);
    }
 
    
//    printf("%d %f %f\n", count, integral, cur_magpsi);
    count++;
    
    signal = Kp * error + Kd * derivative + Ki * integral;
//    printf("%d %f\n", count, integral);    
//    printf("integral %f heading %f error %f magpsi %f\n", integral, heading, error, cur_magpsi);
    
    ap_roll(signal);
}

/*
 * Main autopilot loop
 */
void * ap_loop()
{
    while(1)
    {
        gettimeofday(&t1, NULL);
        ap_heading(heading);
        ap_elev(elev_meters);
        gettimeofday(&t2, NULL);
        elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
        elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
        printf("%f ms\n", elapsedTime);
        usleep(FREQ_TIME);
    }
}