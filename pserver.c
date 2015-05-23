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
#include <math.h>

#define MAX_BUFFER 1024
#define MAX_USERNAME 128
#define MAX_PASSWORD 256
#define MAX_FIRST_NAME 128
#define MAX_LAST_NAME 128
#define MAX_QUOTE 768
#define MAX_THREAD 1024
#define MAX_KEY 4096

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
    int Y;
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
    int XA, YA, q, alpha;
    Active_Client_List ac_client;
    Client_List c_client;
} Thread_Arg;

//Encryption and Decription
int* Stream_Key(char key[], int keylen, int msglen){
	int S[256];
	int *stream = 0;
	stream = (int*)malloc(sizeof(int)*25);
	int i;
	for(i=0; i<256; i++){
		S[i] = i;
	}
	int j=0;
	for(i=0; i<256; i++){
		j = (j+S[i]+(key[i%keylen]-'0'))%256;
		int temp = S[i];
		S[i] = S[j];
		S[j] = temp;
	}
	j = 0;
	i=0;
	int n=0;
	while(n<msglen){
		i = (i+1)%256;
		j = (j+S[i])%256;
		int temp = S[i];
		S[i] = S[j];
		S[j] = temp;
		int K = S[(S[i]+S[j])%256];
		stream[n] = K;
		n++;
	}
	return stream;
}

char* Stream_Key_to_Hex(char key[], int keylen, char msg[]){
	int S[256];
	char *charstream;
	charstream = (char*)malloc(512);
	int i;
	for(i=0; i<256; i++){
		S[i] = i;
	}
	int j=0;
	for(i=0; i<256; i++){
		j = (j+S[i]+(key[i%keylen]-'0'))%256;
		int temp = S[i];
		S[i] = S[j];
		S[j] = temp;
	}
	j = 0;
	i=0;
	int n=0;
	while(n<strlen(msg)){
		i = (i+1)%256;
		j = (j+S[i])%256;
		int temp = S[i];
		S[i] = S[j];
		S[j] = temp;
		int K = S[(S[i]+S[j])%256];
		sprintf(&charstream[n],"%02x",K);
		n+=strlen(&charstream[n]);
	}
	return charstream;
}

char* Int_To_Hex(int numbers[], int msglen){
	char* res;
	res = (char*)malloc(512);
	int j=0;
	int i;
	for(i=0; i<msglen; i++){
		if(numbers[i] <= 0xFFFF){
			sprintf(&res[j],"%02x",numbers[i]);
		}
		j+=strlen(&res[j]);
	}
	return res;
}

char* String_To_Hex(char text[]){
	int textlen = strlen(text);
	char* res;
	res = (char*)malloc(512);
	int i;
	for(i=0; i<textlen; i++){
		sprintf(res+i*2, "%02X", text[i]);
	}
	return res;
}

int Hex_to_Int(char c){
    int first = c / 16 - 3;
    int second = c % 16;
    int result = first*10 + second;
    if(result > 9) result--;
    return result;
}

int Hex_to_Ascii(char c, char d){
    int high = Hex_to_Int(c) * 16;
    int low = Hex_to_Int(d);
    return high+low;
}

static inline unsigned int value(char c){
    if (c >= '0' && c <= '9') { return c - '0';      }
    if (c >= 'a' && c <= 'f') { return c - 'a' + 10; }
    if (c >= 'A' && c <= 'F') { return c - 'A' + 10; }
    return -1;
}

char* Encrypt_Message(char key[], char msg[]){
	char* res;
	res = (char*)malloc(512);
	static char const alphabet[] = "0123456789abcdef";

    int i;
	for(i=0; i!=strlen(key); ++i){
		unsigned int v = value(msg[i]) ^ value(key[i]);
		res[i] = alphabet[v];
	}
	return res;
}

char* Decrypt_Message(char key[], char msg[]){
	char* res;
	res = (char*)malloc(512);
	static char const alphabet[] = "0123456789abcdef";

    int i;
	for(i=0; i<strlen(key); i++){
		unsigned long int v = value(msg[i]) ^ value(key[i]);
		res[i] = alphabet[v];
	}
	return res;
}
//end

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
    temp->curr_online = 0;
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
void add_active_client(Active_Client_List *l, int newsock, char IP[INET_ADDRSTRLEN], char username[MAX_USERNAME], int y){
    Active_Client *temp = (Active_Client*)malloc(sizeof(Active_Client));
    strcpy(temp->client_IP,IP);
    strcpy(temp->username,username);
    temp->active_sock = newsock;
    temp->Y = y;

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

//Menambahkan public key q dan YB pada client
void add_key_client(Active_Client_List *l, char username[MAX_USERNAME], int y){
    Active_Client *iter = l->first;
    if(strcmp(l->first->username,username)==0){
        l->first->Y = y;
        return;
    }
    else{
        Active_Client *iter = l->first;
        do{
            if(strcmp(iter->username,username)==0){
                iter->Y = y;
                return;
            }
            else{
                if(iter->next!=NULL) iter = iter->next;
                else break;
            }
        }while(1);
    }
    return;
}

//Cek status klien
int check_status_client(Client_List *l, char username[INET_ADDRSTRLEN]){
    Client *iter = l->first;
    while(iter!=NULL){
        if(strcmp(iter->username,username)==0){
            if(iter->curr_online==0) return 0;
            else return 1;
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

// set client online
int set_online_client(Client_List *l, char username[INET_ADDRSTRLEN]){
    Client *iter = l->first;
    while(iter!=NULL){
        if(strcmp(iter->username,username)==0){
            iter->curr_online = 1;
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

// Fungsi Diffie-Hellman
typedef struct PrimitiveRoot{
	int n, root;
}PrimitiveRoot;

int Miller_Rabin_Test(int n){
    int q = rand()%n;
    while(q%2!=1) q = rand()%n;
    int k = (rand()%(n-1))+1;
    int z = (int)pow(2,k)*q;
    while(z!=(n-1)){
        q = rand()%n;
        while(q%2!=1) q = rand()%n;
        k = (rand()%(n-1))+1;
        z = (int)pow(2,k)*q;
    }
    int a = (rand()%(n-2))+2;
    int u = (int)pow(a,q);
    if(u%n==1) return 1;
    int j;
    for(j=0; j<k; j++){
        int l = (int)pow(2,j);
        int v = (int)pow(a,l*q);
        if(v%n==(n-1)) return 1;
    }
    return 0;
}

PrimitiveRoot primitiveRoot[100];

int find_n_root(int q){
    int i;
    for(i=0;i<32;i++){
        if(primitiveRoot[i].n==q) return primitiveRoot[i].root;
    }
    return q;
}

int* random_q_alpha(){
    int* numbers = 0;
    numbers = (int*)malloc(sizeof(int)*16);

	primitiveRoot[0].n = 3;
	primitiveRoot[0].root = 2;
	primitiveRoot[1].n = 5;
	primitiveRoot[1].root = 2;
	primitiveRoot[2].n = 7;
	primitiveRoot[2].root = 3;
	primitiveRoot[3].n = 9;
	primitiveRoot[3].root = 2;
	primitiveRoot[4].n = 11;
	primitiveRoot[4].root = 2;
	primitiveRoot[5].n = 13;
	primitiveRoot[5].root = 6;
	primitiveRoot[6].n = 16;
	primitiveRoot[6].root = 5;
	primitiveRoot[7].n = 17;
	primitiveRoot[7].root = 10;
	primitiveRoot[8].n = 19;
	primitiveRoot[8].root = 10;
	primitiveRoot[9].n = 23;
	primitiveRoot[9].root = 10;
	primitiveRoot[10].n = 25;
	primitiveRoot[10].root = 2;
	primitiveRoot[11].n = 27;
	primitiveRoot[11].root = 2;
	primitiveRoot[12].n = 29;
	primitiveRoot[12].root = 10;
	primitiveRoot[13].n = 31;
	primitiveRoot[13].root = 17;
	primitiveRoot[14].n = 32;
	primitiveRoot[14].root = 5;
	primitiveRoot[15].n = 37;
	primitiveRoot[15].root = 5;
	primitiveRoot[16].n = 41;
	primitiveRoot[16].root = 6;
	primitiveRoot[17].n = 43;
	primitiveRoot[17].root = 28;
	primitiveRoot[18].n = 47;
	primitiveRoot[18].root = 10;
	primitiveRoot[19].n = 49;
	primitiveRoot[19].root = 10;
	primitiveRoot[20].n = 53;
	primitiveRoot[20].root = 26;
	primitiveRoot[21].n = 59;
	primitiveRoot[21].root = 10;
	primitiveRoot[22].n = 61;
	primitiveRoot[22].root = 10;
	primitiveRoot[23].n = 64;
	primitiveRoot[23].root = 5;
	primitiveRoot[24].n = 67;
	primitiveRoot[24].root = 12;
	primitiveRoot[25].n = 71;
	primitiveRoot[25].root = 62;
	primitiveRoot[26].n = 73;
	primitiveRoot[26].root = 5;
	primitiveRoot[27].n = 79;
	primitiveRoot[27].root = 29;
	primitiveRoot[28].n = 81;
	primitiveRoot[28].root = 11;
	primitiveRoot[29].n = 83;
	primitiveRoot[29].root = 50;
	primitiveRoot[30].n = 89;
	primitiveRoot[30].root = 30;
	primitiveRoot[31].n = 97;
	primitiveRoot[31].root = 10;

    int rq = rand()%32;
	int random_q = primitiveRoot[rq].n;
    int result_miller = Miller_Rabin_Test(random_q);
    while(result_miller!=1){
        int rq = rand()%32;
        int random_q = primitiveRoot[rq].n;
        int result_miller = Miller_Rabin_Test(random_q);
    }

    int q_Skey = random_q, alpha_Skey = primitiveRoot[rq].root;

    numbers[0] = q_Skey;
    numbers[1] = alpha_Skey;
    return numbers;
}
//end

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

	 int q = random_q_alpha()[0], alpha = random_q_alpha()[1];
	 int XA = (rand()%(q-1))+1;
	 int YA = pow(alpha,XA);
     YA = YA%q;
	 
	 printf("Server SUCCESS: got key XA = %d and YA = %d\n", XA, YA);	 
	 
	 puts("Server waiting for a client...");

     while(1){
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0){
            close(newsockfd);
            close(sockfd);
            serror("Server ERROR: on accept");
        }
        puts("Server SUCCESS: Server accepted a client");

        srand(time(NULL));
/*
        char session[MAX_SESS];
        int sess = rand()%10000;
        snprintf(session, MAX_SESS,"%s%d%s","RTR:SESSION:",sess,":!>\n");

        //Kirim session ID dulu
        write(newsockfd,session,strlen(session));*/

        char IP_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(cli_addr.sin_addr), IP_str, INET_ADDRSTRLEN);

        Thread_Arg TAL;
        strcpy(TAL.receiver_IP, IP_str);
        TAL.receiver_sock = newsockfd;
        TAL.ac_client = ACL;
        TAL.c_client = CL;
        TAL.q = q;
        TAL.alpha = alpha;
        TAL.XA = XA;
        TAL.YA = YA;

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

    // Primitive root

	int q_Skey = (int)thread_arg->q;
	int alpha_SKey = (int)thread_arg->alpha;
    int XA_Skey = (int)thread_arg->XA;
    int YA_Skey = (int)thread_arg->YA;
    int YB_Ckey = -1;
    int Key_Used = -1;

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
		
		//Request public key server
		//RTR:KEYGEN:Server:PublicQ:!>
        if(strcmp("REQKEY",token[1])==0){
            char msgsend[1024];
            snprintf(msgsend,sizeof(msgsend),"%s%s%s%d%s","RTR:KEYGEN:","Server",":",q_Skey,":!>\n");
            if(write((thread_arg->receiver_sock),msgsend,sizeof(msgsend)) < 0){
                printf("Server ERROR: on writing to client\n");
            }
            printf("Server SUCCESS: on writing to client\n");
        }
        //Set dictionary for public key of clients: REQ:SETKEY:User1:YB:!>
        else if(strcmp("SETKEY",token[1])==0){
            YB_Ckey = atoi(token[3]);
            add_key_client(&(thread_arg->ac_client),token[2],YB_Ckey);
            Key_Used = (pow(YB_Ckey,XA_Skey))%q_Skey;
            printf("Server SUCCESS: adding public key dictionary\n");
        }
        //Login client: REQ:LOGIN:User1:Encrypt(Pass,K):!>
        //Mohon password user dienkripsi (untuk login saja)
        else if(strcmp(token[1],"LOGIN")==0){
            Active_Client_List *list = &(thread_arg->ac_client);
            Active_Client *iter = list->first;
            while(iter!=NULL){
                if(strcmp(iter->username,token[2])==0){
                    break;
                }
                if(iter->next!=NULL) iter = iter->next;
                else break;
            }
            int K = Key_Used;
            char key[256];
            sprintf(key,"%d",K);
            char* newK;
            newK = (char*)malloc(512);
            strcpy(newK,Stream_Key_to_Hex(key,strlen(key),token[3]));

            char pass[512];
            int len = strlen(newK), w=0, u;
            char buf = 0;
            for(u=0; u<len; u++){
            	if(u%2!=0){
            		pass[w] = Hex_to_Ascii(buf,newK[u]) + '0';
				}
				else{
					buf = newK[u];
				}
			}

            if(checklogin_client(&(thread_arg->c_client), token[2], pass) == 0){
                char msgserver[1024];
                snprintf(msgserver,sizeof(msgserver),"%s%s%s","RTR:FAILEDLOGIN:",token[2],":!>\n");
                write(thread_arg->receiver_sock,msgserver,strlen(msgserver));
                puts("Server waiting for login attempt");
            }
            else{
                printf("Server SUCCESS: %s logged in successfully\n", token[2]);
                char msgsend[1024];
                add_active_client(&(thread_arg->ac_client),thread_arg->receiver_sock,thread_arg->receiver_IP,thread_arg->username,YB);
                snprintf(msgsend,sizeof(msgsend),"%s%s%s","RTR:SUCCESSLOGIN:",token[2],":!>\n");
                if(write((thread_arg->receiver_sock),msgsend,sizeof(msgsend)) < 0){
                    printf("Server ERROR: on writing to client\n");
                }
                printf("Server SUCCESS: on writing to client\n");
                total_active_client += 1;
                break;
            }
        }
        // Sementara password untuk register jangan dienkripsi
        else if(strcmp(token[1],"REGISTER")==0){
            char key[256];
            sprintf(key,"%d",Key_Used);
            char* newK;
            newK = (char*)malloc(512);
            strcpy(newK,Stream_Key_to_Hex(key,strlen(key),token[3]));

            char pass[512];
            int len = strlen(newK), w=0, u;
            char buf = 0;
            for(u=0; u<len; u++){
            	if(u%2!=0){
            		pass[w] = Hex_to_Ascii(buf,newK[u]) + '0';
				}
				else{
					buf = newK[u];
				}
			}

            signup_client(&(thread_arg->c_client), token[2], token[3]);
            printf("Server SUCCESS: account %s created\n", token[2]);
            char msgsend[1024];
            snprintf(msgsend,sizeof(msgsend),"%s%s%s","RTR:SUCCESSREGISTER:",token[2],":!>\n");
            printf("Server SUCCESS: %s registered successfully\n", token[3]);
            if(write((thread_arg->receiver_sock),msgsend,sizeof(msgsend)) < 0){
                printf("Server ERROR: on writing to client\n");
            }
            printf("Server SUCCESS: on writing to client\n");
        }

	    /*
	    Protokol pasca login
	    "Header:Status_code:ID_sender:ID_receiver:Data_block:!>"
	    Data_block = Message/LogOut
	    */
		
        else if(strcmp(token[1],"LOGOUT")==0){
            char msgsend[1024];
            delete_active_client(&(thread_arg->ac_client),token[2]);
            int successlog = set_offline_client(&(thread_arg->c_client),token[2]);
            snprintf(msgsend,sizeof(msgsend),"%s%s%s","RTR:SUCCESSLOGOUT:",token[2],":!>\n");
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
        //protokol= RTR:REQCHAT:User1:User2:!>
        else if(strcmp(token[1],"REQCHAT")==0){
            int status = check_status_client(&(thread_arg->c_client), token[3]);
            if(status==0){
                char msgsend[1024];
                snprintf(msgsend,sizeof(msgsend),"%s%s%s%s%s%s","RTR:FAILEDREQCHAT:",token[2],":",token[3],":",":!>\n");
                if(write(thread_arg->receiver_sock,msgsend,sizeof(msgsend)) < 0){
                    printf("Server ERROR: on writing to client user\n");
                }
                else printf("Server SUCCESS: on writing to client user\n");
            }
            //protokol = RTR:SUCCESSREQCHAT:User1:User2:Encrypt(session,K):YA:!>
            else{
                Active_Client_List *list = &(thread_arg->ac_client);
                Active_Client *iter = list->first;
                while(iter!=NULL){
                    if(strcmp(iter->username,token[2])==0){
                        break;
                    }
                    if(iter->next!=NULL) iter = iter->next;
                    else break;
                }
                int YB = iter->Y;
                int K = (int)pow(YB,XA_Skey)%q_Skey;
                char key[256];
                sprintf(key,"%d",K);
                char session[MAX_SESS];
                int sess = rand()%10000;
                sprintf(session,"%d",sess);
                int* newK = Stream_Key(key,strlen(key),strlen(session));
                char* hexK = Int_To_Hex(newK, strlen(session));
                char* message = Encrypt_Message(hexK,String_To_Hex(session));

                char msgsend1[1024], msgsend2[1024];
                snprintf(msgsend1,sizeof(msgsend1),"%s%s%s%s%s%s%s%d%s","RTR:SUCCESSREQCHAT:",token[2],":",token[3],":",message,":",YA_Skey,":!>\n");

                if(write(thread_arg->receiver_sock,msgsend1,sizeof(msgsend1)) < 0){
                    printf("Server ERROR: on writing to client user 1\n");
                }
                else printf("Server SUCCESS: on writing to client user 2\n");

                iter->active_session = sess;

                iter = list->first;
                while(iter!=NULL){
                    if(strcmp(iter->username,token[3])==0){
                        break;
                    }
                    if(iter->next!=NULL) iter = iter->next;
                    else break;
                }
                YB = iter->Y;
                K = (int)pow(YB,XA_Skey)%q_Skey;
                char key2[256];
                sprintf(key2,"%d",K);
                int* newK2 = Stream_Key(key2, strlen(key2),strlen(session));
                char* hexK2 = Int_To_Hex(newK2, strlen(session));
                char* message2 = Encrypt_Message(hexK2,String_To_Hex(session));

                snprintf(msgsend2,sizeof(msgsend2),"%s%s%s%s%s%s%s%d%s","RTR:SUCCESSREQCHAT:",token[3],":",token[2],":",message2,":",YA_Skey,":!>\n");

                if(write(iter->active_sock,msgsend2,sizeof(msgsend2)) < 0){
                    printf("Server ERROR: on writing to client user 2\n");
                }
                else printf("Server SUCCESS: on writing to client user 2\n");

                iter->active_session = sess;
            }
        }
        // REQ:CHAT:User1:User2:Encrypted:Hash:!>
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
            snprintf(msgsend,sizeof(msgsend),"%s%s%s%s%s%s%s%s%s","RTR:RCHAT:",token[2],":",token[3],":",token[4], ":", token[5],":!>\n");
            if(write(rec_sock,msgsend,sizeof(msgsend)) < 0){
                printf("Server ERROR: on writing to client user\n");
            }
            else printf("Server SUCCESS: on writing to client user\n");
        }
        else{
        	printf("Server ERROR: accepted wrong header\n");
		}
        // Last
    }
}
