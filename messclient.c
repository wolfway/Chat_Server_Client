#define _XOPEN_SOURCE 500
#define _POSIX_VERSION 199506L


#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include "common.h"
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "libnet.h"

#include <netdb.h>
#include <arpa/inet.h>

/*#define DRIVER  NET_DRIVER_SOCKETS*/
#define DRIVER  NET_DRIVER_SOCKETS
#define CONN_ATTEMPTS 7 

/* Declarations */
int init_client(int,char **);
int b_pthreads_init();

char server_IP_port[256];
struct str_user_data user_data;
struct str_user_data * p_user_data;

int main (int argc, char *argv[]){

 printf("**************************************************\n");
 printf("* Dimitar Jordanov - xjorda01.  PDS project 2007 *\n");
 printf("**************************************************\n");
 if(init_client(argc,argv) == ERROR){
   /* Init Message Client fail */
   return ERROR;
 }/*if*/

 if ( b_pthreads_init() != OK ){
    /* Creating of a thread failed */
     return ERROR;
 }/*if*/


 return OK;
}
/***********************************************/
/* function name :kill_sig_func()              */
/* return falue  :void                         */
/***********************************************/
void v_kill_sig_func(int signal){
  
 printf("\n# Ctrl + C is blocked. Press Enter to exit !\n ");

}

/***********************************************/
/* function name : i_child_sig_func()          */
/* return falue  :                             */
/***********************************************/
void init_SIGCHLD_hook(){

struct sigaction sa_kill;
sigset_t sb;

sa_kill.sa_handler=v_kill_sig_func;
sigaddset(&sa_kill.sa_mask,SIGINT);
sa_kill.sa_flags=0;


   sigemptyset(&sb);
   sigaddset(&sb,SIGQUIT);
   sigaddset(&sb,SIGTERM);
   sigaddset(&sb,SIGTSTP);
   sigprocmask(SIG_BLOCK, &sb, NULL);

  if(sigaction(SIGINT,&sa_kill,NULL) == -1){
    perror(" init SIGINT ");
    exit(1);
  }
}
/***********************************************/
/* function name : b_parser                    */
/* return falue  : int                         */
/***********************************************/
int b_parser(int argc, char *argv[]){

 if ( argc != 6 || strncmp(argv[2],"-tcp",4) != OK || strncmp(argv[4],"-udp",4) != OK ){
  return ERROR;
 }
  /* Copy the IP address*/
 strncpy(server_IP_port,argv[1],strlen(argv[1]));
 /* Add ':' */
 server_IP_port[strlen(argv[1])] = ':'; 
 /* Copy the Server's port + '\0' */
 strncpy(&server_IP_port[strlen(argv[1])+1],argv[3],strlen(argv[3])+1);

 while(1){
      printf("Please enter your IP address : ");
      fflush(stdout);
      if(  read(0,user_data.local_IP,IP_LEN+1) > IP_LEN ){
        printf("# Incorect IP address ! \n ");
        /* TODO  Take the clear function from pos_3 threads.c if this stays */
      }/*if*/
      else
        break;
 }/*while*/

 /* remove the new line */
 user_data.local_IP[strlen(user_data.local_IP)-1]='\0';

 user_data.pid = (int)getpid(); 
 strncpy( user_data.local_IP_udp_port , user_data.local_IP , strlen(user_data.local_IP) );
 user_data.local_IP_udp_port[strlen(user_data.local_IP)]=':';
 strncpy(&user_data.local_IP_udp_port[strlen(user_data.local_IP)+1],argv[5],strlen(argv[5])+1);
 
 return OK;
}
/***********************************************/
/* function name : pv_messclient_send          */
/* return falue  : void *                      */
/***********************************************/
int init_client(int argc, char *argv[]){

 net_init ();
 /*net_loadconfig (NULL);*/
 net_detectdrivers (net_drivers_all); 
 net_initdrivers (net_drivers_all);

 /* Set up Proc Mask + SININT hook */
  init_SIGCHLD_hook();

 /* Parse the input */
 if ( b_parser(argc,argv) == ERROR ){
  printf("# Bad parameters! \n");
  return ERROR;
 }/*if*/
 
  return OK;
}
/***********************************************/
/* function name : pv_messclient_send          */
/* return falue  : void *                      */
/***********************************************/
void *pv_messclient_send(){

NET_CONN *conn = NULL;
char message[MAX_MESS_LEN + 2];
int status = 0;
int message_len = 0;
int conn_attempts = 0;
/* Send Sign in Massage*/

  conn = net_openconn (DRIVER,NULL);

    if (!conn) {
       fprintf (stderr,"Error opening conn.\n");
       exit(ERROR);
    }

    if (net_connect (conn,server_IP_port) != 0) {
       fprintf (stderr,"Error initiating connection.\n");
       exit(ERROR);
    }
    
    /* Try to connect */
    for(conn_attempts=0;conn_attempts < CONN_ATTEMPTS; conn_attempts++ ){
       status = net_poll_connect(conn);
       sleep(1);
       if(status > 0){
           break;
         }/*if*/
    }/*for*/  
     /* if fail to coonnect */
    if(conn_attempts == CONN_ATTEMPTS ){
      printf("# Server unreachable !!!\n");
      printf("Good bye !!!\n");
      exit(0); 
    }/*if*/

 
    if (status < 0) {
      fprintf(stderr,"Error connecting.");
      exit(ERROR);
   }

    /* Prepare Message */
     message[0]='S'; /*Sign in*/
     memcpy(message+1,&user_data,sizeof(user_data));
    /* Send a message */
       net_send_rdm (conn,message,sizeof(user_data) + 1);

    /* Close the conn.  */
    net_closeconn (conn);
  
    printf("Enter text to send.  Press  Enter  to quit.\n");
    /* Print Uset ID */
    printf("# IP : %s PID :%d :", user_data.local_IP, user_data.pid );
    fflush(stdout);

while(1){
     
    /* Let user enter something.  */
    message_len = read(0,message+1+sizeof(user_data),MAX_MESS_LEN + 1);
    
    if( message_len == -1){
      fprintf(stderr,"# Fatal Error!\n");
      exit(ERROR); 
    }/*if*/
    
    if( message_len > MAX_MESS_LEN ){
      printf("# Too long message ! Message is discarded !\n");
      continue;
    }/*if*/
    

    conn = net_openconn (DRIVER, NULL);

    if (!conn) {
       fprintf (stderr,"Error opening conn.\n");
       exit(ERROR);
    } 

    if (net_connect (conn,server_IP_port) != 0) {
       fprintf (stderr,"Error initiating connection.\n");
       exit(ERROR);
    }
    /* Try to connect */
    for(conn_attempts=0;conn_attempts < CONN_ATTEMPTS; conn_attempts++ ){
       status = net_poll_connect(conn);
       sleep(1);
       if(status > 0){
           break;
         }/*if*/
    }/*for*/
     /* if fail to coonnect */
    if(conn_attempts == CONN_ATTEMPTS ){
      printf("# Server unreachable !!!\n");
      printf("Good bye !!!\n");
      exit(0);
    }/*if*/

   /*
    do status = net_poll_connect (conn);
    while (status == 0);
    */
   if (status < 0) {
      fprintf (stderr,"Error connecting.");
      exit(ERROR);
   }

     /*  Put the Client ID in the message */  
      memcpy(message+1,&user_data,sizeof(user_data));
    /* Logout */
    if( message_len == 1){
      message[0]='L'; /* Logout */
    /* Send a message */
      net_send_rdm (conn,message, sizeof(user_data) + 1);
      printf("# Good Bye !!!\n");
      exit(0);
    }
    else{
      message[0]='M'; /* Message */
    /* Send a message */
      message[message_len + 1 + sizeof(user_data)] = '\0'; 

      net_send_rdm (conn, message, message_len + 1 + sizeof(user_data) + 1);
    }

    /* Close the conn.  */
    net_closeconn (conn);
    
 } /* while */
return OK;
}
/***********************************************/
/* function name : pv_messclient_receive       */
/* return falue  : void *                      */
/***********************************************/
void *pv_messclient_receive(){

NET_CHANNEL *chan = NULL;
message_buf  rbuf;
struct str_user_data * p_user_data;

  chan = net_openchannel (DRIVER,user_data.local_IP_udp_port);

   
    if (!chan) {
        fprintf (stderr,"Error opening channel.\n");
        exit(ERROR);
    }


 while(1){
   while (net_query (chan)){
       /* If so, receive them and print them out.  */

         if (net_receive (chan,&rbuf,sizeof(rbuf), NULL) > 0){
              p_user_data = (struct str_user_data *)( rbuf.mtext + 1 );
              /* Print the message */
              printf("\n# Received from IP : %s PID : %d : %s\n", p_user_data->local_IP,p_user_data->pid,rbuf.mtext+1+sizeof(struct str_user_data));  
              /*Write the init massage again */ 
              printf("# IP : %s PID :%d :", user_data.local_IP, user_data.pid );
              fflush(stdout);
        }/*if*/
    }/*while*/ 
 }/*while (1)*/

   net_closechannel (chan);

return OK;
}
/***********************************************/
/* function name : b_pthreads_init()           */
/* return falue  : ERROR / OK                  */
/***********************************************/
int  b_pthreads_init(){

int i=0,result=0;
void *pv_ret_value_send = NULL;
void *pv_ret_value_receive = NULL;
pthread_attr_t attr[2];
pthread_t pt[2];


 /*  init thread's attributs */
  for(i=0;i<2;i++){
   if( ( result = pthread_attr_init(&attr[i])) != 0 ){
   fprintf(stderr," pthread_attr_init() err %d \n",result);
   return ERROR;
   }
  } /* for */

  /*  set thread's attributs */
  for(i=0;i<2;i++){
   if( (result = pthread_attr_setdetachstate(&attr[i],PTHREAD_CREATE_JOINABLE)) != 0 ){
   fprintf(stderr," pthread_attr_setdetachstate () err %d \n",result);
   return ERROR;
   }
  } /* for */

  /*  create and start  Command Line Interface thread */

  result = pthread_create(pt,attr,pv_messclient_send,NULL);
  if(result){
    fprintf(stderr," pthread_create() err %d\n",result);
    return ERROR;
  }
 /*  create and start Shell_Exec  thread */
   result = pthread_create(&pt[1],&attr[1],pv_messclient_receive,NULL);
  if(result){
    fprintf(stderr," pthread_create() err %d\n",result);
    return ERROR;
  }
   /* wait to finish first thread */
  if( (result = pthread_join(pt[0],&pv_ret_value_send)) != 0 ){
    fprintf(stderr,"pthread_join() err %d\n",result);
    return ERROR;
  }

   /* wait to finish second thread */
  if( (result = pthread_join(pt[1],&pv_ret_value_receive)) != 0 ){
    fprintf(stderr,"pthread_join() err %d\n",result);
    return ERROR;
  }

  return OK;

} /* end : int  b_pthreads_init() */

