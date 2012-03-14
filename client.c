/*
 *  client.c
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
#include <pwd.h>

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
 * main()
 *
 * return home of current user
 */
int main()
{
  int s, len;
  struct sockaddr_un remote;
  char str[100];
  char path[109];

  strcpy(path, user_home());
  strncat(path, "/ap.socket", 108 - strlen(path));

  if((s = socket(PF_LOCAL, SOCK_STREAM, 0)) == -1)
    {
      perror("socket");
      exit(1);
    }

  printf("Trying to connect...\n");

  remote.sun_family = AF_UNIX;
  strcpy(remote.sun_path, path);
  len = strlen(remote.sun_path) + sizeof(remote.sun_family) + 1;
  if(connect(s, (struct sockaddr *) &remote, len) == -1)
    {
      perror("connect");
      exit(1);
    }

  printf("Connected.\n");
  int h = 5;
  while(h--)
    {
      strcpy(str, "aoeu\n");
      printf("sending %s", str);
      if(send(s, str, strlen(str) + 1, 0) == -1)
	{
	  perror("send");
	  exit(1);
	}
      if(recvfrom(s, str, 100, 0, NULL, NULL))
	{
	  printf("prijate: %s", str);
	}
      sleep(2);
    }

  close(s);

  return 0;
}
