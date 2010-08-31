#include <stdio.h>
#include <string.h>
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "settings.h"


void winPrintf(
	       XPLMWindowID inWindowID,
	       char * text)
{
  int left, top, right, bottom;
  float	color[] = { 1.0, 1.0, 1.0 }; 	/* RGB White */

  XPLMGetWindowGeometry(inWindowID, &left, &top, &right, &bottom);
  XPLMDrawString(
		 color,
		 left + 5,
		 top - 20,
		 text,
		 NULL, /* no word-wrap */
		 xplmFont_Basic);
}
