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
#include <pthread.h>

#define DATA_OUT "/Users/toxygen/Sites/uav2/data.out"
#define DATA_IN  "/Users/toxygen/Sites/uav2/data.in"


pthread_t       threads[3];
pthread_mutex_t console_m;

/*
 * user_home()
 *
 * return home of current user
 *
 */
char * user_home()
{
    struct passwd * pw;
    uid_t uid;
    uid = getuid();
    pw  = getpwuid(uid);
    return pw->pw_dir;
}

// read input from file
void * read_datain()
{
    FILE * fp;
    char str_new[100];
    char str_old[100];
    while(1)
    {
        fp = fopen(DATA_IN, "r");
        fgets(str_new, 100, fp);
        fclose(fp);
        pthread_mutex_lock(&console_m);
//        printf("precitane %s\n", str_new);
        pthread_mutex_unlock(&console_m);
        if(strcmp(str_new, str_old) != 0)
        {
            strcpy(str_old, str_new);
            pthread_mutex_lock(&console_m);
            printf("autopilot %s\n", str_old);
            pthread_mutex_unlock(&console_m);
        }
        usleep(500000);
    }
}

/*
 * listen(socket)
 *
 * prints received data from socket s
 *
 */
void * listen_console(void * s)
{
    FILE * fp;
    fp = fopen(DATA_OUT,"w+");
    int stat;
    char str_in[100];
    int sock;
    sock = *((int*) s);
    while(1)
    {
        stat = recvfrom(sock, str_in, 100, 0, NULL, NULL);
        if (stat < 1)
        {
            fclose(fp);
#ifdef DEBUG
            pthread_mutex_lock(&console_m);
            printf("value je %d\n", stat);
            perror("recvfrom -1");
            pthread_mutex_unlock(&console_m);
#endif
            exit(1);
        }
        else
        {
#ifdef DEBUG
            pthread_mutex_lock(&console_m);
            printf("prijate: %s\n", str_in);
            printf("pocet: %d\n", stat);
            fflush(stdout);
#endif
            fseek(fp, 0, SEEK_SET);
            fwrite(str_in, sizeof(char), stat, fp);
            fflush(fp);
            pthread_mutex_unlock(&console_m);
        }
    }

}


/*
 * main()
 *
 */
int main()
{
    int s, len, rc;
    struct sockaddr_un remote;
    char str[100];
    char path[109];
    
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    
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
   
    pthread_mutex_init(&console_m, NULL);
    rc = pthread_create(&threads[0], NULL, listen_console, (void *) &s);
    rc = pthread_create(&threads[1], NULL, read_datain, NULL);
    pthread_attr_destroy(&attr);
    
    printf("Connected.\n");
    while(1)
    {
        scanf("%s", str);
        strcat(str, "\n");
        pthread_mutex_lock(&console_m);
        printf("sending %s", str);
        pthread_mutex_unlock(&console_m);
        if(send(s, str, strlen(str) + 1, 0) < 1)
        {  
            pthread_mutex_lock(&console_m);
            perror("send");
            pthread_mutex_unlock(&console_m);
            exit(1);
        }
    }
    pthread_cancel(threads[0]);
    pthread_cancel(threads[1]);
    pthread_mutex_destroy(&console_m);
    close(s);
    
    return 0;
}
