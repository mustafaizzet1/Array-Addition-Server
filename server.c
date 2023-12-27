#include<stdio.h>
#include<string.h>    // for strlen
#include<sys/socket.h>
#include<arpa/inet.h> // for inet_addr
#include<unistd.h>    // for write
#include <pthread.h>
#include <ctype.h>
#include <stdlib.h>




#define DATA_SIZE 100
pthread_t THREAD_ARRAY [DATA_SIZE]; 
int PORT_NUMBER = 60000;
char INPUT_STRING [DATA_SIZE];
int FIRST_ARRAY [DATA_SIZE];
int SECOND_ARRAY [DATA_SIZE];
int CARRY_ARRAY [DATA_SIZE];
int RESULT_ARRAY [DATA_SIZE];
int num=0;
int maxSize = 0;
// Mutex for thread synchronization
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
void* routine(void* arg) {
    int index = *(int*)arg;
    int sum = 0;
    
    RESULT_ARRAY[index]= FIRST_ARRAY[index]+SECOND_ARRAY[index];
    if (RESULT_ARRAY[index] > 999) { // if result more than 999 we take the modulus of a number with respect to 1000 and set carry to 1
        pthread_mutex_lock(&mutex);
        RESULT_ARRAY[index]=RESULT_ARRAY[index]%1000;
        CARRY_ARRAY[index+1]=1;
        pthread_mutex_unlock(&mutex);
    }
  
    free(arg);
}

void summation(){
    int i;
    for (i = 0; i < num; i++) {  // msb at adress 0
        int* a = malloc(sizeof(int));
        *a = i;
        if (pthread_create(&THREAD_ARRAY[i], NULL, &routine, a) != 0) {//  thread is created as many as size of array 
            perror("Failed to create thread");
        }
    }
    
   for (int k = 0; k < num; ++k) {
        pthread_join(THREAD_ARRAY[k], NULL);
    }
    if(CARRY_ARRAY[num]==1)// checking whether the  last value  of result array have carry or dont have
        num+=1;
    for (int j = 0; j <num; j++)
    {
        RESULT_ARRAY[j]+=CARRY_ARRAY[j];// after thread summation if there is carry it sum with result array again
    }

}





void isNumeric(const char *num,int new_socket) { // it is checked is number numeric or not
    char *end;
    long value = strtol(num, &end, 10);
    if (end == num) {
        write(new_socket,"input is not a number", sizeof("input is not a number"));
        exit(EXIT_FAILURE); 
    } 


}

void string_to_integer(char array_1[],int array_2[],int new_socket){

    int l=0;
   // Extract the first token
   char * token = strtok(array_1, " ");
   if (strchr(token, '\t') != NULL) {
        write(new_socket, "Error: Tab character encountered\n", strlen("Error: Tab character encountered\n"));
        exit(EXIT_FAILURE); // Terminate the program with a failure status
        }
    while (strlen(token) > 0 && token[0] == ' ' && token[1] == ' ') {// double space chechking
            token += 1; // Move to the next character
        }
   
   
   while( token != NULL ) {

    
    isNumeric(token,new_socket);
    
    if(atoi(token)<=999 &&atoi(token) >= 0 && strlen(token)<100){//checking of whether number is smaller than 999 or not 
    array_2[l]=atoi(token); //printing each token
    l++;
    token = strtok(NULL, " ");
    }
    else{
        write(new_socket,"Number is more than 999\n",strlen("Number is more than 999\n"));
         exit(EXIT_FAILURE);
    }
 
   }
   num=l;

}
void writing_to_Console(int new_socket){// writing to result of summation to telnet console 

   for (int i = 0; i < num; ++i) {
    int x = RESULT_ARRAY[i];
    int length = snprintf(NULL, 0, "%d", x);
    
    char* str = malloc(length + 1);
    snprintf(str, length + 1, "%d", x);
    
    write(new_socket, str, length + 1);
    write(new_socket, " ", sizeof(" "));

    
    free(str);
    }   
}


void handle_connection(int new_socket){// we handle everthing here related to process 
write(new_socket, "Hello, this is Array Addition Server!\n", strlen("Hello, this is Array Addition Server!\n"));
write(new_socket,"Please enter the first array for addition:\n",strlen("Please enter the first array for addition:\n"));
read(new_socket,INPUT_STRING,sizeof(INPUT_STRING));
string_to_integer(INPUT_STRING,FIRST_ARRAY,new_socket);
bzero(INPUT_STRING,sizeof(INPUT_STRING));
write(new_socket,"Please enter the second array for addition:\n",strlen("Please enter the second array for addition:\n"));
read(new_socket,INPUT_STRING,sizeof(INPUT_STRING));
string_to_integer(INPUT_STRING,SECOND_ARRAY,new_socket);
if((sizeof(FIRST_ARRAY)/sizeof(FIRST_ARRAY[0]))!= (sizeof(SECOND_ARRAY)/sizeof(SECOND_ARRAY[0]))){// checking of whether number of both arrays are equal or not 
 write(new_socket,"Number of integers are different for both arrays. You must send equal number of integers both arrays\n",strlen("Please enter the first array for addition:\n"));
  exit(EXIT_FAILURE);
}
else{
summation();

 write(new_socket,"The result of array addition are given below:\n",strlen("The result of array addition are given below:\n"));
 writing_to_Console(new_socket);
   
}
    
}



 
int main(int argc, char *argv[])
{
    int sockfd , new_socket , c;
    struct sockaddr_in server , client;
    char *message;
     
    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    //Address Family - AF_INET (this is IP version 4)
    //Type - SOCK_STREAM (this means connection oriented TCP protocol)
    //Protocol - 0 (Specifies a particular protocol to be used with the socket...
    //Specifying a protocol of 0 causes socket() to use an unspecified default protocol) 
    //[ or IPPROTO_IP This is IP protocol]
    if (sockfd == -1)
    {
        puts("Could not create socket");
        return 1;
    }
     
    server.sin_family = AF_INET;  //IPv4 Internet protocols
    server.sin_addr.s_addr = INADDR_ANY; //IPv4 local host address
    server.sin_port = htons(PORT_NUMBER); // server will listen to 60000 port
     
    // Bind
    if(bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        puts("Binding failed");
        return 1;
    }
    puts("Socket is binded");
   
    listen(sockfd, 5);
     
    // Accept and incoming connection
    puts("Waiting for incoming connections...");
    
    c = sizeof(struct sockaddr_in);
    new_socket= accept(sockfd,(struct sockaddr *)&client,(socklen_t*)&c);
    if (new_socket<0)
    {
        puts("Accept failed");
        return 1;
    }
     
    puts("Connection accepted");
     
    // Reply to the client
  
    handle_connection(new_socket);

  
    
   

    
    close(sockfd);
    close(new_socket); 

    return 0;
}