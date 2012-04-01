/*
 *  ui.c
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
#include <pthread.h>
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "settings.h"
#include "server.h"

extern char ** lines;
extern pthread_mutex_t lines_m;

void redraw(XPLMWindowID inWindowID)
{
    pthread_mutex_lock(&lines_m);
    int space = 13;
    int left, top, right, bottom;
    float	color[] = { 1.0, 1.0, 1.0 }; 	/* RGB White */
    XPLMGetWindowGeometry(inWindowID, &left, &top, &right, &bottom);
    
    for(int i = 0; i < LINECOUNT; i++)
    {
        XPLMDrawString(color,
                       left + 5,
                       top - space,
                       lines[i],
                       NULL, /* no word-wrap */
                       xplmFont_Basic);
        space += OFFSET;
    }
    pthread_mutex_unlock(&lines_m);
}


/* Function for printing to the log */
/* TODO */
/* create separate thread for processing printMsg */
void printMsg(char * text)
{
    pthread_mutex_lock(&lines_m);
    static int counter = 0;
    
    if(counter == LINECOUNT - 1)
    {
        for(int i = 1; i < LINECOUNT; i++)
        {
            strcpy(lines[i-1], lines[i]);
        }
    }
    strcpy(lines[counter], text);
    
    if(counter < (LINECOUNT - 1))
    {
        counter++;
    }
    pthread_mutex_unlock(&lines_m);
}
