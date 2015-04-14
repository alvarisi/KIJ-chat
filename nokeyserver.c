/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <limits.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>

#define MAX_BUFFER 1024
#define MAX_USERNAME 128
#define MAX_PASSWORD 256
#define MAX_FIRST_NAME 128
#define MAX_LAST_NAME 128
#define MAX_QUOTE 768
#define MAX_THREAD 1024
#define MAX_KEY 4096

static volatile int keepRunning = 1;

void intHandler(int dummy){
    keepRunning = 0;
}

int total_active_client = 0;
int total_client = 2;
const int MAX_SESS = (CHAR_BIT * sizeof(int) - 1) / 3 + 2;

// Pendefinisian struct

// List detail client
typedef struct Client{
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    int curr_online;
    struct Client* next;
} Client;

typedef struct Client_List{
    Client* first;
} Client_List;

// List detail klien aktif
typedef struct Active_Client{
    int active_sock, active_session;
    char client_IP[INET_ADDRSTRLEN];
    char username[MAX_USERNAME];
    struct Active_Client* next;
} Active_Client;

typedef struct Active_Client_List{
    Active_Client* first;
} Active_Client_List;

// Struct untuk argumen fungsi thread
typedef struct Thread_Arg{
    int receiver_sock;
    char receiver_IP[INET_ADDRSTRLEN];
    char username[MAX_USERNAME];
    int session;
    Active_Client_List ac_client;
    Client_List c_client;
} Thread_Arg;

void Diffie_Hellman(){
	/*later discussed*/
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void serror(const char *msg)
{
    perror(msg);
    puts("Server socket closed");
    exit(1);
}

// Cek username + password
int checklogin_client(Client_List* l, char user[MAX_USERNAME], char pass[MAX_PASSWORD]){
    if(strcmp(l->first->username,user)==0 && strcmp(l->first->password,pass)==0){
        return 1;
    }
    else{
        Client *iter = l->first;
        printf("%s + %s\n", iter->username, iter->password);
        do{
            if(strcmp(iter->username,user)==0 && strcmp(iter->password,pass)==0) return 1;
            else{
                if(iter->next!=NULL) iter = iter->next;
                else break;
            }
        }while(1);
    }
    return 0;
}

// signup client
void signup_client(Client_List* l, char user[MAX_USERNAME], char pass[MAX_PASSWORD]){
    Client *temp = (Client*)malloc(sizeof(Client));
    strcpy(temp->username,user);
    strcpy(temp->password,pass);
    temp->curr_online = 1;
    if(l->first==NULL){
        l->first = temp;
        temp->next = NULL;
    }
    else{
        Client *iter = l->first;
        while(iter->next!=NULL){
            iter = iter->next;
        }
        iter->next = temp;
        temp->next = NULL;
    }
}

// Menambah list klien aktif
void add_active_client(Active_Client_List *l, int newsock, int session, char IP[INET_ADDRSTRLEN], char username[MAX_USERNAME]){
    Active_Client *temp = (Active_Client*)malloc(sizeof(Active_Client));
    strcpy(temp->client_IP,IP);
    strcpy(temp->username,username);
    temp->active_sock = newsock;
    temp->active_session = session;

    if(l->first==NULL){
        l->first = temp;
        temp->next = NULL;
    }
    else{
        Active_Client *iter = l->first;
        while(iter->next!=NULL){
            iter = iter->next;
        }
        iter->next = temp;
        temp->next = NULL;
    }
}

// meng-set client offline
int set_offline_client(Client_List *l, char username[INET_ADDRSTRLEN]){
    Client *iter = l->first;
    while(iter!=NULL){
        if(strcmp(iter->username,username)==0){
            iter->curr_online = 0;
            return 1;
        }
        if(iter->next!=NULL){
            iter = iter->next;
        }
        else{
            break;
        }
    }
    return 0;
}

// Menghapus klien yang sedang aktif
void delete_active_client(Active_Client_List *l, char username[INET_ADDRSTRLEN]){
    Active_Client *iter = l->first;
    while(iter->next!=NULL){
        if(strcmp(iter->next->username,username)==0){
            break;
        }
        iter = iter->next;
    }
    if(iter==l->first){
        Active_Client *temp = iter;
        l->first = iter->next;
        l->first->next = iter->next->next;
        free(temp);
    }
    else{
        Active_Client *temp = iter->next;
        iter->next = iter->next->next;
        free(temp);
    }
}

//menghapus client
void delete_client(Client_List* l, char user[MAX_USERNAME]){
    Client *iter = l->first;
    while(iter->next!=NULL){
        if(strcmp(iter->next->username,user)==0) break;
        iter = iter->next;
    }
    if(iter==l->first){
        Client *temp = iter;
        l->first = iter->next;
        l->first->next = iter->next->next;
        free(temp);
    }
    else{
        Client *temp = iter->next;
        iter->next = iter->next->next;
        free(temp);
    }
}

// fungsi thread (diimplementasi di bawah)
void *client_thread_func(void *arg);

// fungsi main
int main()
{
     int sockfd, newsockfd, portno = 9999, *newsock;
     pthread_t threads;
     socklen_t clilen;
     char buffer[1025];

     Client_List CL;
     CL.first = NULL;
     Active_Client_List ACL;
     ACL.first = NULL;

     signup_client(&CL, "andre", "andre");
     signup_client(&CL, "test", "test");

     struct sockaddr_in serv_addr, cli_addr;
     int n;

     srand(time(NULL));

     printf("Server socket uses IP: '127.0.0.1' with port: %d.\n", portno);

     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) error("Server ERROR: on opening socket");
     puts("Server SUCCESS: socket opened successfully");

     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) error("Server ERROR: on binding");
     puts("Server SUCCESS: on binding");
     listen(sockfd,5);

     puts("Server waiting for a client...");

     while(1){
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0){
            close(newsockfd);
            close(sockfd);
            serror("Server ERROR: on accept");
        }
        puts("Server SUCCESS: Server accepted");

        char session[MAX_SESS];
        int sess = rand()%10000;
        snprintf(session, MAX_SESS,"%s%d%s","RTR:SESSION:",sess,":!>\n");

        //Kirim session ID dulu
        write(newsockfd,session,strlen(session));

        char IP_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(cli_addr.sin_addr), IP_str, INET_ADDRSTRLEN);

        Thread_Arg TAL;
        strcpy(TAL.receiver_IP, IP_str);
        TAL.receiver_sock = newsockfd;
        TAL.session = sess;
        TAL.ac_client = ACL;
        TAL.c_client = CL;

        if(pthread_create(&threads, NULL, client_thread_func, &TAL)){
            printf("Server ERROR: could not create thread\n");
        }

        puts("Server SUCCESS: Thread assigned. Clients can communicate freely");

     }

     if(pthread_join(threads, NULL)){
        printf("Server ERROR: joining threads\n");
     }
     else printf("Server SUCCESS: joining threads\n");

     close(newsockfd);
     close(sockfd);
     pthread_exit(NULL);
     printf("Server closed\n");
     return 0;
}

// Implementasi body fungsi thread
void *client_thread_func(void *arg){
    char buffer[1030];

    Thread_Arg *thread_arg = (Thread_Arg*)arg;
    // Cek user sebelum login dan sukses/gagal login
    /*
    Protocol secara umum untuk pra-chat:
    Login= "Header:Status_Code:ID:PASSWORD:!>"
    Fail= "Header:Status_Code!>"
    Success= "Header:Status_Code:ID:!>"
    */

    while(1){

        bzero(buffer,sizeof(buffer));
        read(thread_arg->receiver_sock,buffer,sizeof(buffer));

        printf("Server Receive: %s\n", buffer);

        char token[15][250];
        char *tokens;

        int counter = 0;
        tokens = strtok(buffer,":!>");
        while(tokens!=NULL){
            strcpy(token[counter],tokens);
            tokens = strtok(NULL,":!>");
            counter += 1;
        }

        int o;
        for(o=0; o<4; o++){
            puts(token[o]);
        }

        if(strcmp(token[1],"LOGIN")==0){
            if(checklogin_client(&(thread_arg->c_client), token[2], token[3]) == 0){
                char msgserver[] = "RTR:FAILEDLOGIN:!>\n";
                write(thread_arg->receiver_sock,msgserver,strlen(msgserver));
            }
            else{
                printf("Server SUCCESS: %s logged in successfully\n", token[2]);
                char msgsend[1024];
                add_active_client(&(thread_arg->ac_client),thread_arg->receiver_sock,thread_arg->session,thread_arg->receiver_IP,thread_arg->username);
                snprintf(msgsend,sizeof(msgsend),"%s%s%s","RTR:SUCCESSLOGIN:",token[2],":!>\n");
                //write(thread_arg->receiver_sock,msgsend,strlen(msgsend));
                if(write((thread_arg->receiver_sock),msgsend,sizeof(msgsend)) < 0){
                    printf("Server ERROR: on writing to client\n");
                }
                printf("Server SUCCESS: on writing to client\n");
                total_active_client += 1;
                break;
            }
        }
        else if(strcmp(token[1],"REGISTER")==0){
            signup_client(&(thread_arg->c_client), token[2], token[3]);
            printf("Server SUCCESS: account %s created\n", token[3]);
            char msgsend[1024];
            snprintf(msgsend,sizeof(msgsend),"%s%s%s","RTR:SUCCESSREGISTER:",token[2],":!>\n");
            add_active_client(&(thread_arg->ac_client),thread_arg->receiver_sock,thread_arg->session,thread_arg->receiver_IP,thread_arg->username);
            printf("Server SUCCESS: %s logged in successfully\n", token[2]);
            if(write((thread_arg->receiver_sock),msgsend,sizeof(msgsend)) < 0){
                printf("Server ERROR: on writing to client\n");
            }
            printf("Server SUCCESS: on writing to client\n");
            break;
        }
    }

    puts("Server SUCCESS: getting into UI features");

    /*
    Protokol pasca login
    "Header:LIST:ID_sender:Data_block:!>"
    "Header:LOGOUT:ID_sender:!>"
    "Header:CHAT:ID_sender:ID_receiver:Data_block:!>"
    Data_block = Message/LogOut
    */

    while(1){
        bzero(buffer,sizeof(buffer));
        read(thread_arg->receiver_sock,buffer,sizeof(buffer));

        printf("Server Receive: %s\n", buffer);

        char token[10][900];
        char *tokens;

        int counter = 0;
        tokens = strtok(buffer,":!>");
        while(tokens!=NULL){
            strcpy(token[counter],tokens);
            tokens = strtok(NULL,":!>");
            counter += 1;
        }

        if(strcmp(token[1],"LOGOUT")==0){
            char msgsend[1024];
            delete_active_client(&(thread_arg->ac_client),token[1]);
            int successlog = set_offline_client(&(thread_arg->c_client),token[1]);
            snprintf(msgsend,sizeof(msgsend),"%s%s%s","RTR:SUCCESSLOGOUT:",token[1],":!>\n");
            if(write((thread_arg->receiver_sock),msgsend,sizeof(msgsend)) < 0 || successlog==0){
                printf("Server ERROR: on writing to client\n");
            }
            else printf("Server SUCCESS: on writing to client\n");
            close((thread_arg->receiver_sock));
        }
        else if(strcmp(token[1],"LIST")==0){
            char msgsend[1024];
            Active_Client_List *list = &(thread_arg->ac_client);
            Active_Client *iter = list->first;
            char* lists;
            while(iter!=NULL){
                snprintf(lists,sizeof(lists),"%s%s",iter->username,":");
                if(iter->next!=NULL)iter = iter->next;
                else break;
            }
            snprintf(msgsend,sizeof(msgsend),"%s%s%s%s%s","RTR:SUCCESSLIST:",token[2],":DATA:",lists,"!>\n");
            if(write((thread_arg->receiver_sock),msgsend,sizeof(msgsend)) < 0){
                printf("Server ERROR: on writing to client\n");
            }
            else printf("Server SUCCESS: on writing to client\n");
        }
        else if(strcmp(token[1],"CHAT")==0){
            Active_Client_List *list = &(thread_arg->ac_client);
            Active_Client *iter = list->first;
            while(iter!=NULL){
                if(strcmp(iter->username,token[3])==0){
                    break;
                }
                if(iter->next!=NULL) iter = iter->next;
                else break;
            }
            int rec_sock = iter->active_sock;
            char msgsend[1024];
            snprintf(msgsend,sizeof(msgsend),"%s%s%s%s%s%s%s","RTR:RCHAT:",token[2],":",token[3],":",token[4],":!>\n");
            if(write(rec_sock,msgsend,sizeof(msgsend)) < 0){
                printf("Server ERROR: on writing to client user\n");
            }
            else printf("Server SUCCESS: on writing to client user\n");
        }
    }
}
