/*
 *  interface.c
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY except by those people which sell it, which
 *  are required to give you total support for your newly bought product;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 *  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMDataAccess.h"
#include "ui.h"
#include "settings.h"
#include "server.h"
#include "control.h"

char **      lines    = NULL;
XPLMWindowID gWindow  = NULL;
int          gClicked = 0;
pthread_t    threads[3];
pthread_mutex_t lines_m;

XPLMDataRef  indicated_airspeed2;   /* float */
XPLMDataRef  alpha;                 /* float */    /* AOA      */
XPLMDataRef  magpsi;                /* float */    /* mag head */
XPLMDataRef  phi;                   /* float */    /* roll     */
XPLMDataRef  theta;                 /* float */    /* pitch    */
XPLMDataRef  elevation;             /* double */   /* altitude */
XPLMDataRef  throttle_ref;          /* float */
XPLMDataRef  paused;                /* int */
XPLMDataRef  engine_count;          /* int */
XPLMDataRef  aileron1_ref;          /* float */
XPLMDataRef  aileron2_ref;          /* float */
XPLMDataRef  aileron3_ref;          /* float */
XPLMDataRef  aileron4_ref;          /* float */
XPLMDataRef  override;              /* int */
XPLMDataRef  rudder1;               /* float */
XPLMDataRef  rudder2;               /* float */
XPLMDataRef  VVI;                   /* float */    /* vertical velocity in feet per second */
XPLMDataRef  rudder4;               /* float */

void cleanup()
{
    for(int i = 0; i < LINECOUNT; i++)
    {
        free(lines[i]);
    }
    free(lines);
    sock_cleanup();
}

void MyDrawWindowCallback(XPLMWindowID inWindowID,
                          void *       inRefcon);

void MyHandleKeyCallback(XPLMWindowID inWindowID,
                         char         inKey,
                         XPLMKeyFlags inFlags,
                         char         inVirtualKey,
                         void *       inRefcon,
                         int          losingFocus);

int MyHandleMouseClickCallback(XPLMWindowID    inWindowID,
                               int             x,
                               int             y,
                               XPLMMouseStatus inMouse,
                               void *          inRefcon);


/*
 * XPluginStart
 */
PLUGIN_API int XPluginStart(
                            char * outName,
                            char * outSig,
                            char * outDesc)
{
    int rc;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    
    
    /* Inform X-Plane about the plugin */
    strcpy(outName, "AP Interface " VERSION);
    strcpy(outSig, COMPANY "." PACKAGE "." PLUGIN);
    strcpy(outDesc, "Interface for autopilot");
    
    /* Create buffer for console */
    lines = (char **) malloc (sizeof(char *) * LINECOUNT);
    
    for(int i = 0; i < LINECOUNT; i++)
    {
        lines[i] = (char *) calloc (256, sizeof(char));
    }
    
    pthread_mutex_init(&lines_m, NULL);
    
    
    /* Create Main Window */
    gWindow = XPLMCreateWindow(
                               50,  /* left    */
                               900, /* top     */
                               300, /* right   */
                               800, /* bottom  */
                               1,   /* visible */
                               MyDrawWindowCallback,       /* draw callback */
                               MyHandleKeyCallback,        /* key handling callback */
                               MyHandleMouseClickCallback, /* mouseclick handling callback */
                               NULL);
    
    indicated_airspeed2 = XPLMFindDataRef("sim/flightmodel/position/indicated_airspeed2");
    if(indicated_airspeed2 != NULL) printMsg("Indicated Airspeed 2 initialized");

    alpha = XPLMFindDataRef("sim/flightmodel/position/alpha");
    if(alpha != NULL) printMsg("AOA initialized");

    phi = XPLMFindDataRef("sim/flightmodel/position/phi");
    if(phi != NULL) printMsg("Roll initialized");
    
    magpsi = XPLMFindDataRef("sim/flightmodel/position/magpsi");
    if(magpsi != NULL) printMsg("Magnetometer initialized");

    
    theta = XPLMFindDataRef("sim/flightmodel/position/theta");
    if(theta != NULL) printMsg("Pitch initialized");
    
    elevation = XPLMFindDataRef("sim/flightmodel/position/elevation");
    if(elevation != NULL) printMsg("Elevation initialized");

    throttle_ref = XPLMFindDataRef("sim/flightmodel/engine/ENGN_thro");
    if(throttle_ref != NULL) printMsg("Throttle initialized");
    
    override = XPLMFindDataRef("sim/operation/override/override_control_surfaces");
    paused = XPLMFindDataRef("sim/time/paused");
    engine_count = XPLMFindDataRef("sim/aircraft/engine/acf_num_engines");

    aileron1_ref = XPLMFindDataRef("sim/flightmodel/controls/wing2l_ail1def");
    if(aileron1_ref != NULL) printMsg("Aileron 1 initialized");

    aileron2_ref = XPLMFindDataRef("sim/flightmodel/controls/wing2r_ail1def");
    if(aileron2_ref != NULL) printMsg("Aileron 2 initialized");

    aileron3_ref = XPLMFindDataRef("sim/flightmodel/controls/wing1l_ail1def");
    aileron4_ref = XPLMFindDataRef("sim/flightmodel/controls/wing1r_ail1def");

    rudder1 = XPLMFindDataRef("sim/flightmodel/controls/hstab1_elv1def");
    rudder2 = XPLMFindDataRef("sim/flightmodel/controls/hstab2_elv1def");
    VVI = XPLMFindDataRef("sim/flightmodel/position/vh_ind_fpm2");
//    rudder4 = XPLMFindDataRef("sim/flightmodel/controls/wing2r_elv1def");
    
    rc = pthread_create(&threads[0], NULL, server, NULL);
    rc = pthread_create(&threads[1], NULL, ap_loop, NULL);
    rc = pthread_create(&threads[2], NULL, ap_log, NULL);
    
    pthread_attr_destroy(&attr);
    return 1;
}

/*
 * XPluginStop
 */
PLUGIN_API void	XPluginStop(void)
{
    XPLMDestroyWindow(gWindow);
    cleanup();
}

/*
 * XPluginDisable
 */
PLUGIN_API void XPluginDisable(void)
{
    pthread_cancel(threads[0]);
    pthread_cancel(threads[1]);
    pthread_cancel(threads[2]);
    pthread_mutex_destroy(&lines_m);
}

/*
 * XPluginEnable.
 */
PLUGIN_API int XPluginEnable(void)
{
    return 1;
}

/*
 * XPluginReceiveMessage
 */
PLUGIN_API void XPluginReceiveMessage(
                                      XPLMPluginID inFromWho,
                                      long	   inMessage,
                                      void *	   inParam)
{
}

/*
 * MyDrawingWindowCallback
 */
void MyDrawWindowCallback(
                          XPLMWindowID inWindowID,
                          void *       inRefcon)
{
    /* transfer to pointers, probably leaking memory */
    int left, top, right, bottom;
    char speed_IAS[100];
    char altitude[100];
    char AOA[100];
    char roll[100]; 
    char pause[100];
    char heading[100];
    char thrust[100];
    char pitch[100];
    int  char_count = 0;
    float throttle[8];
    
    XPLMGetDatavf(throttle_ref, throttle, 0, 8);
    
    
    set_heading(240.0f);
    set_elevation(1000);
    
    /* get the size of window */
    XPLMGetWindowGeometry(inWindowID, &left, &top, &right, &bottom);
    
    /* draw dark shade in window */
    XPLMDrawTranslucentDarkBox(left, top, right, bottom);

    char_count += sprintf(speed_IAS,  "%.2f", XPLMGetDataf(indicated_airspeed2));
    char_count += sprintf(altitude,  ",%.2f", XPLMGetDatad(elevation));
    char_count += sprintf(AOA,       ",%.2f", XPLMGetDataf(alpha));    
    char_count += sprintf(roll,      ",%.2f", XPLMGetDataf(phi));
    char_count += sprintf(pause,     ",%.2d", XPLMGetDatai(paused));
    char_count += sprintf(heading,   ",%.2f", XPLMGetDataf(magpsi));
    char_count += sprintf(thrust,    ",%.2f", throttle[0]);
    char_count += sprintf(pitch,     ",%.2f", XPLMGetDataf(theta));
    
    strcat(speed_IAS, altitude);
    strcat(speed_IAS, AOA);
    strcat(speed_IAS, roll);
    strcat(speed_IAS, pause);
    strcat(speed_IAS, heading);
    strcat(speed_IAS, thrust);
    strcat(speed_IAS, pitch);
    
    send_nmsg(s2, speed_IAS, char_count);
    redraw(inWindowID);
    
}

/*
 * MyHandleKeyCallback
 */
void MyHandleKeyCallback(
                         XPLMWindowID inWindowID,
                         char         inKey,
                         XPLMKeyFlags inFlags,
                         char         inVirtualKey,
                         void *       inRefcon,
                         int          losingFocus)
{
}

/*
 * MyHandleMouseClickCallback
 */
int MyHandleMouseClickCallback(
                               XPLMWindowID    inWindowID,
                               int             x,
                               int             y,
                               XPLMMouseStatus inMouse,
                               void *          inRefcon)
{
    /* toggle up and down mouse state */
    if ((inMouse == xplm_MouseDown) || (inMouse == xplm_MouseUp))
        gClicked = 1 - gClicked;
    if(gClicked)
        printMsg("test1");
    else
        printMsg("test2");
    return 1;
}
