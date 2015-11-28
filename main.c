#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "cJSON.h"

#define IPSTR "127.0.0.1"
//#define IPSTR "192.168.0.102"

#define SERVER_ADDRESS "www.boxkam.com"

#define PORT 80
#define SERVER_PORT 8080

#define BUFSIZE 8192

#define TUTK "TUTKPlatform"

char SERVER_BUF[BUFSIZE];
char UID[BUFSIZE];
static char ID[20];
static char SERVER_IP[16];
int send_to_server();

void get_ip(char *host){
  struct hostent *hent;
  int iplen=15;
  GETIP:
  if((hent=gethostbyname(host))==NULL){
    perror("Can't get ip");
	sleep(5);
    goto GETIP;
  }
  if(inet_ntop(AF_INET,(void *)hent->h_addr_list[0],SERVER_IP,iplen)==NULL){
    perror("Can't resolve host!\n");
	sleep(5);
     goto GETIP;
  }
  printf("host:%s -> ip:%s \n",hent->h_name,inet_ntoa(*((struct in_addr *)hent->h_addr)));
  return;
}


/* Parse text to JSON, then render back to text, and print! */
int doit(char *text)
{
	cJSON *json;
	json=cJSON_Parse(text);
	if (!json)
	{
		printf("Error before: [%s]\n",cJSON_GetErrorPtr());
		return -1;
	}
	else
	{
		if(json->type == cJSON_String)
		{
			printf("cJSON_String ->%s , %s \n",json->valuestring,json->string);
		}
		if(json->type == cJSON_Object){
			cJSON *child = json->child;
			do{
				if(child->type == cJSON_String){
					//printf("child cJSON_String %s ,type:%d, %s\n",child->string,child->type,child->valuestring);
				}
				if(child->type == cJSON_Object)
				{
					if(strcmp(child->string,TUTK) == 0)
					{
						printf(" child cJSON_Object %s ,type:%d, %s\n",child->string,child->type,child->valuestring);
				    		printf("2 cJSON_Object child :%s,%s \n",child->child->string,child->child->valuestring);
						strcpy(ID,child->child->valuestring);
					}
					child->child = child->child->next;
				}
				child = child->next;
			}while(child);
		}
		
	}
	return 0;
}

void thread_getuid()
{
	int sockfd = -1,ret1, i;
    struct sockaddr_in servaddr;
    char str1[BUFSIZE],buf[BUFSIZE];
	GETUID:
	if(sockfd > 0)
	{
		close(sockfd);
		sockfd = -1;
		sleep(3);
	}
	
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        printf("getuid---socket error!\n");
         goto GETUID;
    }
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
	
    if (inet_pton(AF_INET, IPSTR, &servaddr.sin_addr) <= 0 ){
       printf("getuid--inet_pton error!\n");
        goto GETUID;
    }
 
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
       printf("getuid,connect error!\n");
       goto GETUID;
    }
    printf("connect success: %s\n",IPSTR);
	//sned data
   	memset(str1, 0, BUFSIZE);
    strcat(str1, "GET /cgi-bin/getparam.cgi HTTP/1.1\r\n");
	strcat(str1, "Accept: text/html, application/xhtml+xml\r\n");
	strcat(str1, "Accept-Encoding: gzip, deflate\r\n");
	strcat(str1, "User-Agent: Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; WOW64; Trident/5.0) \r\n");
	strcat(str1, "Connection: Keep-Alive\r\n\r\n");

	printf("======= getparam.cgi ======\n%s\n",str1);
	
	ret1 = write(sockfd,str1,strlen(str1));
    if (ret1 <= 0) {
        printf("cgi getparam send fail code:%d error msg: '%s'\n",errno, strerror(errno));
        goto GETUID;
    }else{
        printf(" cgi getparam send msg success length: %d \n", ret1);
    }
	
    while(1){
      	memset(buf, 0, 4096);
     	i= read(sockfd, buf, 4096);
       	if (i <= 0 )
	   	{
           printf("=== getuid read %d ==\n",i);
		   close(sockfd);
		   return;
        }
	 	printf("========== get uid ==========\n");
		int t = 0;
		do
		{
			t++;
		}
		while(buf[t]!= '{');
		strcpy(UID,buf+t);
		doit(UID);
		printf("=====================================\n");
    }
    close(sockfd);
}

void thread_server()
{
  	int sockfd = -1, ret, i, h;
    struct sockaddr_in servaddr;
    char str1[BUFSIZE], buf[BUFSIZE];
    fd_set   t_set1;
    struct timeval  tv;
 	GETSERVER:
	if(sockfd > 0)
	{
		close(sockfd);
		sockfd = -1;
		sleep(3);
	}
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        printf("thread_server---socket error!\n");
        goto GETSERVER;
    };
 
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, IPSTR, &servaddr.sin_addr) <= 0 ){
       printf("thread_server--inet_pton error!\n");
	   goto GETSERVER;
    }
 
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
       printf("thread_server  connect error!\n");
	   goto GETSERVER;
    }
    printf("thread_server connect success: %s\n",IPSTR);
	
    //send data
    memset(str1, 0, BUFSIZE);
    strcat(str1, "GET /event.cgi HTTP/1.1\r\n");
	strcat(str1, "Accept: text/html, application/xhtml+xml\r\n");
	strcat(str1, "Accept-Encoding: gzip, deflate\r\n");
	strcat(str1, "User-Agent: Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; WOW64; Trident/5.0) \r\n");
	strcat(str1, "Connection: Keep-Alive\r\n\r\n");
	
    printf("%s\n",str1);
	
    ret = write(sockfd,str1,strlen(str1));
    if (ret <= 0) {
        printf(" cgi send fail code: %d error msg: '%s'\n",errno, strerror(errno));
        goto GETSERVER;
    }else{
        printf(" cgi send msg success length: %d \n", ret);
    }
	
    while(1)
	{
       FD_ZERO(&t_set1);
       FD_SET(sockfd, &t_set1);
       tv.tv_sec= 10;
       tv.tv_usec= 0;
       h= 0;
       h= select(sockfd +1, &t_set1, NULL, NULL, &tv);
       if (h == 0)
	   		continue;
	   //printf("select:%d\n",h);
	   if (h < 0) {
	        printf("thread_server read error\n");
			goto GETSERVER;
		}
 
       if (h > 0)
	   	{
	      	 memset(buf, 0, BUFSIZE);
	      	 i= read(sockfd, buf, BUFSIZE);
			 
	      	 if (i <= 0)
			 {
	            printf("thread_server read %d \n",i);
	      	 }
			 if(i > 18 && doit(buf) == 0)
	 	 	 {
				strcpy(SERVER_BUF,buf);			
				if(send_to_server() == -1)
					printf("=== send_to_server fail ===");
				else
					printf("==%s\n", buf);
		  	}
       	}
      }
    close(sockfd);
}
int send_to_server()
{
  	int sockfd, ret;
    struct sockaddr_in servaddr;
    char str[BUFSIZE];
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
         printf("send server---socket error!\n");
		 goto END;
    }
 
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT);
	printf("server ip: %s \n",SERVER_IP);
	
    if (inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr) <= 0 ){
        printf("send server--inet_pton error  ! \n");
	    goto END;
    }
 
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
        printf("send server,connect error!\n");
	    goto END;
    }
	
   	printf("connect success: %s\n",SERVER_ADDRESS);
	
 	const char* req_format =
   			 "GET /365PushServer/apns/alarm_in?uid=%s&msg=%s HTTP/1.1\r\n"
   			 "Host: %s:%d\r\n"
   			 "User-Agent: Mozilla/5.0 (Windows NT 6.1; rv:32.0)"
   			 "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
    		 "Content-Type: application/x-www-form-urlencoded\r\n"
    		 "Accept-Language: zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3\r\n"
    		 "Accept-Encoding: gzip, deflate\r\n"
   			 "Connection: Keep-Alive\r\n"
   			 "Cache-Control: no-cache\r\n";
       
	//send data
	memset(str, 0, BUFSIZE);
	sprintf(str, req_format, ID, SERVER_BUF, SERVER_ADDRESS, SERVER_PORT);
	printf("%s \n",str);
	ret = write(sockfd,str,strlen(str));
	
	if (ret <= 0) 
	{
      printf(" send error code: %d , error msg:'%s'\n",errno, strerror(errno));
	  goto END;
	}
	else
	{
       	printf(" send success length: %d \n", ret);
   		 close(sockfd);
		return 0;
	}
	END:
    close(sockfd);
	return -1;
		
}

int main(int argc, char **argv)
{		
	pthread_t id,id3;
	int ret,ret3;
	signal(SIGPIPE, SIG_IGN);
	get_ip(SERVER_ADDRESS);
  	ret = pthread_create(&id,NULL,(void  *) thread_server,NULL);
  	ret3 = pthread_create(&id3,NULL,(void  *) thread_getuid,NULL);

  	if(ret!=0){
  		printf("Create pthread1 error cgi! \n");
	}
	
	if(ret3!=0){
  		printf("Create pthread3 error! \n");
	}

	pthread_join(id,NULL);
	pthread_join(id3,NULL);
	
    return 0;
}

