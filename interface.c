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
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "settings.h"

XPLMWindowID gWindow  = NULL;
int          gClicked = 0;

void MyDrawWindowCallback(
			  XPLMWindowID inWindowID,    
			  void *       inRefcon);    

void MyHandleKeyCallback(
			 XPLMWindowID inWindowID,    
			 char         inKey,    
			 XPLMKeyFlags inFlags,    
			 char         inVirtualKey,    
			 void *       inRefcon,    
			 int          losingFocus);    

int MyHandleMouseClickCallback(
			       XPLMWindowID    inWindowID,    
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
	/* First we must fill in the passed in buffers to describe our
	 * plugin to the plugin-system. */

	strcpy(outName, "AP Interface " VERSION);
	strcpy(outSig, COMPANY "." PACKAGE "." PLUGIN);
	strcpy(outDesc, "Interface for autopilot");

	gWindow = XPLMCreateWindow(
				   50,  /* left    */
				   900,  /* top     */
				   300, /* right   */
				   800, /* bottom  */
				   1,   /* visible */
				   MyDrawWindowCallback, /* draw callback */
				   MyHandleKeyCallback,  /* key handling callback */
				   MyHandleMouseClickCallback, /* mouseclick handling callback */
				   NULL);		
	return 1;
}

/*
 * XPluginStop
 */
PLUGIN_API void	XPluginStop(void)
{
	XPLMDestroyWindow(gWindow);
}

/*
 * XPluginDisable
 */
PLUGIN_API void XPluginDisable(void)
{
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
  int left, top, right, bottom;
  float	color[] = { 1.0, 1.0, 1.0 }; 	/* RGB White */

  /* get the size of window */
  XPLMGetWindowGeometry(inWindowID, &left, &top, &right, &bottom);

  /* draw dark shade in window */
  XPLMDrawTranslucentDarkBox(left, top, right, bottom); 
  winPrintf(inWindowID, "bubu");
		
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
	
  return 1;
}                                      
