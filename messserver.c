#define _XOPEN_SOURCE 500
#define _POSIX_VERSION 199506L
	
#include <signal.h>
#include <stdlib.h>
#include "common.h"
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "libnet.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DRIVER  NET_DRIVER_SOCKETS

typedef struct user_ID{
char local_IP_udp_port[IP_PORT];
char local_IP[IP_LEN];
int  pid;
struct user_ID *prev;
struct user_ID *next;
}user_ID_t;

user_ID_t *head = NULL;
user_ID_t *tail = NULL;
char server_IP_TCP[IP_PORT];
char tcp_port[6];

NET_CONN *listen;
NET_CONN *conn;

void get_server_IP(char * tcp_port);
void init_SIGCHLD_hook();
int b_pthreads_init();
int b_parser(int argc, char *argv[]);
/****************************** MAIN ***********************************/
int main (int argc, char *argv[]){
    printf("**************************************************\n");
    printf("* Dimitar Jordanov - xjorda01.  PDS project 2007 *\n");
    printf("**************************************************\n");

   /* Set up procmask + SIGINT hook*/
    init_SIGCHLD_hook();
   /* Parse the inpust params */
   if(b_parser(argc,argv) == ERROR ){
      fprintf(stderr,"# Bad parameters! \n");
      return ERROR;
   }/*if*/

    /* Get Server IP */
     get_server_IP(argv[2]);

   if ( b_pthreads_init() != OK ){
    /* Creating of a thread failed */
     return ERROR;
   }/*if*/

 return OK;
}/*end MAIN*/
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
/* function name : add_db                      */
/* return falue  : int                         */
/***********************************************/
int add_db(char * local_IP_udp_port, char * local_IP, int pid){

user_ID_t * temp = NULL;

 temp = malloc(sizeof(user_ID_t));

 if(temp == NULL){
  fprintf(stderr,"#Fatal Error !\n");
  exit(ERROR);
 }

 /* Fill the User info */
 strncpy(temp->local_IP_udp_port,local_IP_udp_port,IP_PORT);
 temp->pid = pid;
 strncpy(temp->local_IP,local_IP,IP_LEN);
 temp->next = NULL;

 if( tail != NULL){
   temp->prev=tail;
   tail->next = temp;
   tail=temp;
 }/*if*/
 else{
   temp->prev = NULL;
   tail=temp;
   head=temp;
 }/*else*/

 /* Information message*/
  printf("# New user added -  IP :%s  PID : %d \n",local_IP,pid);

return OK;
}
/***********************************************/
/* function name : remove_db                   */
/* return falue  : int                         */
/***********************************************/
int remove_db(char * local_IP, int pid){

user_ID_t * temp = NULL;

  temp = head;

  while(temp){
   if( ( strncmp(temp->local_IP,local_IP,IP_LEN) == OK )  && ( temp->pid == pid ) )
     break;
   else
     temp=temp->next;
  }/*while*/

  
 /* Information message*/
  printf("# User logout  -  IP :%s  PID : %d  \n",local_IP,pid);


  if(temp){
   /*if first and last*/
    if( (temp==head) && (temp == tail) ){
         head = NULL;
         tail = NULL;
         free(temp);
         return OK;
    }/*if*/
    /* if first but not last*/
    if( temp == head ){
      head = head->next;
      head->prev = NULL;
      free(temp);
      return OK;
    }/*if*/
   /* if last */
   if( temp == tail){
     tail=tail->prev;
     tail->next=NULL;
     free(temp);
     return OK;
   }/*if*/
   else{ /* middle element */
     temp->prev->next = temp->next;
     temp->next->prev = temp->prev;
     free(temp);
     return OK;
   }/*else*/

 }/*if(temp)*/
return ERROR;
}

/***********************************************/
/* function name : server_destructor           */
/* return falue  : void                        */
/***********************************************/
void server_destructor(){
user_ID_t * temp = head;
/* Free the memory allocated for the user database*/
 while(head){
   temp=temp->next;
   free(head);
   head = temp; 
 }/*while*/
}
/***********************************************/
/* function name : print_db                    */
/* return falue  : void                        */
/***********************************************/
void print_db(){

user_ID_t * temp = head;
int i =1;
 
  printf("*********************************************\n");
  printf("*               User's List                 *\n");
  printf("*********************************************\n");
 while(temp){
   printf("User : %d\n",i);
   printf("IP_Port  : %s\n",temp->local_IP_udp_port);
   printf("Local_IP : %s\n",temp->local_IP);
   printf("IP_PID   : %d\n",temp->pid);
   temp=temp->next;
   i++;
 }/*while*/
}
/***********************************************/
/* function name : b_send_udp_all              */
/* return falue  : int                         */
/***********************************************/
int b_send_udp_all(void * message, int mess_len){

NET_CHANNEL *chan;

user_ID_t * temp = head;

  /* Create a Channel*/
   chan = net_openchannel (DRIVER,"");

   if (!chan) {
        fprintf(stderr,"Error opening channel.\n");
        return 1;
   }

   while(temp != NULL) {

      /* Assign a target */
      if (net_assigntarget (chan,temp->local_IP_udp_port) != 0) {
        fprintf(stderr,"Could not use that address;\n");
         return 1;
      } /*if*/
      /* Send a message */
      net_send (chan, message,mess_len);

     temp=temp->next;
  } /*while*/
    /* Close the channel.  */
    net_closechannel (chan);

 return OK;
                                                                                                     
}
/***********************************************/
/* function name : pv_messserver_receive()     */
/* return falue  : ERROR / OK                  */
/***********************************************/
void *pv_messserver_receive(){

int msqid = 0;
key_t key = 1234;
message_buf  rbuf;
int mess_len = 0;
struct str_user_data * p_user_data;

  /* Get the Message Queue ID */
  if ((msqid = msgget(key, 0666)) < 0) {
      perror("msgget");
      exit(1);
   }/*if*/

  while(1){
   
     /* Receive an answer of message type 1. */
     if ( (  mess_len =  msgrcv(msqid,&rbuf,sizeof(rbuf), 1, 0) ) < 0 ) {
         perror("msgrcv");
         exit(1);
     }
     if( rbuf.mtext[0] =='S'){
        p_user_data = (struct str_user_data *) (rbuf.mtext + 1); 
        add_db(p_user_data->local_IP_udp_port,p_user_data->local_IP,p_user_data->pid);
        continue;
     }/*if*/

     if( rbuf.mtext[0] == 'L'){
        p_user_data = (struct str_user_data *) (rbuf.mtext + 1); 
        remove_db(p_user_data->local_IP,p_user_data->pid);
        continue;
     }/*if*/

     if( rbuf.mtext[0] == 'M'){
        p_user_data = (struct str_user_data *) (rbuf.mtext + 1); 
        printf("# Message from IP : %s PID : %d : %s",p_user_data->local_IP, p_user_data->pid,rbuf.mtext + 1 + sizeof(struct str_user_data));
        
        if( b_send_udp_all((void *) &rbuf,mess_len) != OK )
          fprintf(stderr,"#Error - udp communication failed !\n");
     }/*if*/

}/*while*/
 return OK;
}
/***********************************************/
/* function name : b_parser                    */
/* return falue  : int                         */
/***********************************************/
int b_parser(int argc, char *argv[]){

 if ( argc != 5 || strncmp(argv[1],"-tcp",4) != OK || strncmp(argv[3],"-udp",4) != OK ){
  return ERROR;
 }
  /* Copy the IP address*/
/* strncpy(server_IP_port,argv[1],strlen(argv[1]));*/
 return OK;
}
/***********************************************/
/* function name : v_block_unblock_stdin()      */
/* return falue  :                             */
/***********************************************/
void v_block_unblock_stdin(int block_flag)
{
  ioctl(0, FIONBIO, &block_flag);

 /* val = fcntl(sock, F_GETFL, 0); */
 /* fcntl(sock, F_SETFL, val|O_NONBLOCK); */
}
/***********************************************/
/* function name : print_help()                */
/* return falue  : ERROR / OK                  */
/***********************************************/
void print_help(){

   printf("*************************************\n");
   printf("*      HELP - COMMAND'S LIST        *\n");
   printf("*************************************\n");
   printf("print - Print the User's database.   \n");
   printf("Enter - Quit \n");
   printf("?     - Help \n");

}
/***********************************************/
/* function name : get_server_IP               */
/* return falue  : void                        */
/***********************************************/
void get_server_IP(char * tcp_port){
    /* Get the Server IP + TCP port */
int len = 5;
    while(1){
      printf("Please enter your IP address : ");
      fflush(stdout);
      /*if( (  fread(server_IP_TCP,1,IP_LEN+1,stdin) > IP_LEN ) ){*/
      if( ( len =  read(0,server_IP_TCP,IP_LEN+1) ) > IP_LEN  ){
          fprintf(stderr,"# Incorect IP address ! \n ");
      }/*if*/
      else
        break;
    }/*while*/
   server_IP_TCP[strlen(server_IP_TCP)-1]=':'; /* remove the \n */
   strncpy(&server_IP_TCP[strlen(server_IP_TCP)],tcp_port,strlen(tcp_port)+1);
   
}
/***********************************************/
/* function name : pv_messserver_commands()    */
/* return falue  : void *                      */
/***********************************************/
void *pv_messserver_commands(){
 
char in_buff[10]; /* buffer for input from console */
char q_mark[1] = "?";


   /*print welcome mesage */
    print_help();

    printf ("Awaiting connection...\n");

 while (1){

      /* Press enter - Quit  */ 

       switch (  read(0,in_buff,10)  ){
          
           case 1: {   server_destructor();
                       printf("# Good Bye !\n");
                       exit(OK);
                   }

           case 6: { if( strncmp(in_buff,"print",5) == OK ) /* Print User Database*/
                        print_db();
                     else
                       printf("# Unknown command !\n");
                     break;
                   }
          case 2:  { if(strncmp(in_buff,q_mark,1) == OK){  /* HELP*/
                      print_help(); 
                      break;
                     }/*if*/
                     else{
                       printf("# Unknown command !\n");
                        break; 
                     }/*else*/
                    } 
          case  0: {
                    break;
                  }  
           default :{
                     printf("# Unknown command !\n");
                     break;
                   } 
       }

 }/*while(1)*/
}
/***********************************************/
/* function name : pv_messserver_send          */
/* return falue  : void                        */
/***********************************************/
void *pv_messserver_send(){

int msqid;
int msgflg = IPC_CREAT | 0666;
key_t key=1234;
size_t buf_length;
message_buf  sbuf;
     /*Init Libnet */
     net_init ();
 
     net_detectdrivers (net_drivers_all);
     net_initdrivers (net_drivers_all);

     /* Create Message */
     if ((msqid = msgget(key, msgflg )) < 0) {
        perror("msgget");
        exit(1);
     }/*if*/


     /*the server, open a listening conn and wait ans for  clients */
     listen = net_openconn (DRIVER, server_IP_TCP);
     if(!listen) {
       fprintf(stderr,"Error opening conn.\n");
       exit(ERROR);
     }/*if*/

     if(net_listen (listen) != 0) {
       fprintf (stderr,"Error making conn listen.\n");
        exit(ERROR);
     }/*if*/

     /* Set  Message type */
     sbuf.mtype = 1;


 while(1){
    
     /* Wait for a message from a client */
     do conn = net_poll_listen (listen);
     while (!conn);

      /* TODO  Wait for data */  
      while (1){
            buf_length = net_query_rdm (conn);
           if(buf_length)break;
      }    
        /* if so receive */
          if ( net_receive_rdm(conn,sbuf.mtext,MAX_MESS_LEN) > 0){
                /* Send Message */
              if ( msgsnd(msqid, &sbuf, sizeof(sbuf), IPC_NOWAIT) < 0 ){
                  perror("msgsnd");
                   exit(1);
              }/*if*/
          }/*if net_receive*/ 
 
}/* while(1) */

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
void *pv_ret_value_commands = NULL;
pthread_attr_t attr[3];
pthread_t pt[3];


 /*  init thread's attributs */
  for(i=0;i<3;i++){
   if( ( result = pthread_attr_init(&attr[i])) != 0 ){
   fprintf(stderr," pthread_attr_init() err %d \n",result);
   return ERROR;
   }
  } /* for */

  /*  set thread's attributs */
  for(i=0;i<3;i++){
   if( (result = pthread_attr_setdetachstate(&attr[i],PTHREAD_CREATE_JOINABLE)) != 0 ){
   fprintf(stderr," pthread_attr_setdetachstate () err %d \n",result);
   return ERROR;
   }
  } /* for */

  /*  create and start  Command Line Interface thread */

  result = pthread_create(pt,attr,pv_messserver_send,NULL);
  if(result){
    fprintf(stderr," pthread_create() err %d\n",result);
    return ERROR;
  }
 /*  create and start Shell_Exec  thread */
                                                                                                      
  result = pthread_create(&pt[1],&attr[1],pv_messserver_receive,NULL);
   if(result){
     fprintf(stderr," pthread_create() err %d\n",result);
     return ERROR;
   }

  result = pthread_create(&pt[2],&attr[2],pv_messserver_commands,NULL);
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

  /* wait to finish second thread */
  if( (result = pthread_join(pt[2],&pv_ret_value_commands)) != 0 ){
    fprintf(stderr,"pthread_join() err %d\n",result);
    return ERROR;
  }
  return OK;

} /* end : int  b_pthreads_init() */



