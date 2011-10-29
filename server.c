/*
 *  server.c
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
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <pwd.h>
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "ui.h"

int s1, s2, len;
struct sockaddr_un local, remote;

/*
 * user_home()
 *
 * return home of current user
 */
char * user_home()
{
  struct passwd * pw;
  uid_t uid;
  uid = getuid();
  pw  = getpwuid(uid);
  return pw->pw_dir;
}

/*
 * sock_cleanup()
 *
 * close both connections and unlinks socket
 */
void sock_cleanup()
{
  close(s2);
  close(s1);
  unlink(local.sun_path);
}

/*
 * server()
 *
 * start server which creates socket and listens
 */

void * server()
{
  socklen_t t;
  char str[100];
  char msg[100];
  char path[109];

  strcpy(path, user_home());
  strncat(path, "/ap.socket", 108 - strlen(path));

  printMsg("server: Started socket server\n");
  printMsg(path);
  if((s1 = socket(PF_LOCAL, SOCK_STREAM, 0)) == -1)
    {
      printMsg("socket error");
      pthread_exit(NULL);
    }
  local.sun_family = PF_LOCAL;
  strcpy(local.sun_path, path);
  unlink(local.sun_path);
  len = strlen(local.sun_path) + sizeof(local.sun_family) + 1;
  if(bind(s1, (struct sockaddr *) &local, len) == -1)
    {
      printMsg("bind");
      pthread_exit(NULL);
    }

  if (listen(s1, 5) == -1)
    {
      printMsg("listen");
      pthread_exit(NULL);
    }

  while(1)
    {
      int done, n;
      printMsg("server: Waiting for a connection...\n");
      t = sizeof(remote);
      if((s2 = accept(s1, (struct sockaddr *) &remote, &t)) == -1)
	{
	  printMsg("accept");
	  pthread_exit(NULL);
	}
      printMsg("server: Client connected.\n");

      done = 0;
      while(1)
	{
	  n = recvfrom(s2, str, 100, 0, NULL, NULL);
	  if(n < 0)
	    {
	      if(n < 0) printMsg("recv");
	      break;
	    }
	  if(n == 0)
	    {
	      printMsg("server: client disconnected.\n");
	      break;
	    }
	  else
	    {
	      strcpy(msg, "client: ");
	      strcat(msg, str);
	      printMsg(msg);
	      send(s2, str, 100, 0);
	    }
	}
    }
}
