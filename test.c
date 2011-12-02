#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

#define OK 0
#define ERROR -1

int kill(int,int);

int main(int argc,char *argv[])
{
int pid ;
int pid_cl_1;
int pid_cl_2;

int fd_cl_1_conf;
int fd_cl_2_conf;

int fd_cl_1_out;
int fd_cl_2_out;

int fd_ser_conf; 

pid =(int)fork();
 
  /* Start Server */ 
  if(pid == 0){ /*child*/
   
   /* Open Server's configuration file */
   fd_ser_conf = open("server.conf",O_RDONLY,NULL);
   /*Redirect stdin */
   dup2(fd_ser_conf,0); 
   /* Strat the Server */   
    execl("./messserver","messserver","-tcp","1234","-udp","555",0); 
    printf("# Exec failed  \n");
    exit(1);
  }


  pid_cl_1 = (int) fork();  
  /* Start Client 1 */
  if(pid_cl_1 == 0){ /*child*/

   /* Create output file */
   fd_cl_1_out = creat("client_1.out",S_IRWXU); 
   /* Open Client's configuration file */
   fd_cl_1_conf = open("client_1.conf",O_RDONLY,NULL);
   /*Redirect stdin */
   dup2(fd_cl_1_conf,0); 
   /* Redirect stdout*/
   dup2(fd_cl_1_out,1); 
   /* Strat the Server */   
    execl("./messclient","messclient","127.0.0.1","-tcp","1234","-udp","555",0); 
    printf("# Exec failed  \n");
    exit(1);
  }
  

  pid_cl_2 = (int) fork();  
  /* Start Client 2 */
  if(pid_cl_2 == 0){ /*child*/
 
   /* Create output file */
   fd_cl_2_out = creat("client_2.out",S_IRWXU); 
   /* Open Client's configuration file */
   fd_cl_2_conf = open("client_2.conf",O_RDONLY,NULL);
   /*Redirect stdin */
   dup2(fd_cl_2_conf,0); 
   /* Redirect stdout*/
   dup2(fd_cl_2_out,1); 

   /* Strat the Server */   
    execl("../client","client","127.0.0.1","-tcp","1234","-udp","333",0); 
    printf("# Exec failed  \n");
    exit(1);
  }
  
 sleep(10);
 kill(pid_cl_1,9);
 kill(pid_cl_2,9);
 kill(pid,9);

 printf("Test complete successfully ! \n ");

 return OK;
}
