#define OK 0
#define MAX_MESS_LINE 255
#define ERROR -1
#define MAX_MESS_LEN 255
#define IP_LEN 17
#define IP_PORT 22

/* Type for the Message Queue*/
typedef struct msgbuf {
    long    mtype;
    char    mtext[MAX_MESS_LEN];
} message_buf;

struct str_user_data{
char local_IP_udp_port[IP_PORT];
char local_IP[IP_LEN];
int  pid;
};
