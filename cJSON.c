/*
  Copyright (c) 2009 Dave Gamble
 
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
 
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
 
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef cJSON__h
#define cJSON__h

#ifdef __cplusplus
extern "C"
{
#endif

/* cJSON Types: */
#define cJSON_False 0
#define cJSON_True 1
#define cJSON_NULL 2
#define cJSON_Number 3
#define cJSON_String 4
#define cJSON_Array 5
#define cJSON_Object 6
	
#define cJSON_IsReference 256

/* The cJSON structure: */
typedef struct cJSON {
	struct cJSON *next,*prev;	/* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
	struct cJSON *child;		/* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */

	int type;					/* The type of the item, as above. */

	char *valuestring;			/* The item's string, if type==cJSON_String */
	int valueint;				/* The item's number, if type==cJSON_Number */
	double valuedouble;			/* The item's number, if type==cJSON_Number */

	char *string;				/* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
} cJSON;

typedef struct cJSON_Hooks {
      void *(*malloc_fn)(size_t sz);
      void (*free_fn)(void *ptr);
} cJSON_Hooks;

/* Supply malloc, realloc and free functions to cJSON */
extern void cJSON_InitHooks(cJSON_Hooks* hooks);


/* Supply a block of JSON, and this returns a cJSON object you can interrogate. Call cJSON_Delete when finished. */
extern cJSON *cJSON_Parse(const char *value);
/* Render a cJSON entity to text for transfer/storage. Free the char* when finished. */
extern char  *cJSON_Print(cJSON *item);
/* Render a cJSON entity to text for transfer/storage without any formatting. Free the char* when finished. */
extern char  *cJSON_PrintUnformatted(cJSON *item);
/* Delete a cJSON entity and all subentities. */
extern void   cJSON_Delete(cJSON *c);

/* Returns the number of items in an array (or object). */
extern int	  cJSON_GetArraySize(cJSON *array);
/* Retrieve item number "item" from array "array". Returns NULL if unsuccessful. */
extern cJSON *cJSON_GetArrayItem(cJSON *array,int item);
/* Get item "string" from object. Case insensitive. */
extern cJSON *cJSON_GetObjectItem(cJSON *object,const char *string);

/* For analysing failed parses. This returns a pointer to the parse error. You'll probably need to look a few chars back to make sense of it. Defined when cJSON_Parse() returns 0. 0 when cJSON_Parse() succeeds. */
extern const char *cJSON_GetErrorPtr();
	
/* These calls create a cJSON item of the appropriate type. */
extern cJSON *cJSON_CreateNull();
extern cJSON *cJSON_CreateTrue();
extern cJSON *cJSON_CreateFalse();
extern cJSON *cJSON_CreateBool(int b);
extern cJSON *cJSON_CreateNumber(double num);
extern cJSON *cJSON_CreateString(const char *string);
extern cJSON *cJSON_CreateArray();
extern cJSON *cJSON_CreateObject();

/* These utilities create an Array of count items. */
extern cJSON *cJSON_CreateIntArray(int *numbers,int count);
extern cJSON *cJSON_CreateFloatArray(float *numbers,int count);
extern cJSON *cJSON_CreateDoubleArray(double *numbers,int count);
extern cJSON *cJSON_CreateStringArray(const char **strings,int count);

/* Append item to the specified array/object. */
extern void cJSON_AddItemToArray(cJSON *array, cJSON *item);
extern void	cJSON_AddItemToObject(cJSON *object,const char *string,cJSON *item);
/* Append reference to item to the specified array/object. Use this when you want to add an existing cJSON to a new cJSON, but don't want to corrupt your existing cJSON. */
extern void cJSON_AddItemReferenceToArray(cJSON *array, cJSON *item);
extern void	cJSON_AddItemReferenceToObject(cJSON *object,const char *string,cJSON *item);

/* Remove/Detatch items from Arrays/Objects. */
extern cJSON *cJSON_DetachItemFromArray(cJSON *array,int which);
extern void   cJSON_DeleteItemFromArray(cJSON *array,int which);
extern cJSON *cJSON_DetachItemFromObject(cJSON *object,const char *string);
extern void   cJSON_DeleteItemFromObject(cJSON *object,const char *string);
	
/* Update array items. */
extern void cJSON_ReplaceItemInArray(cJSON *array,int which,cJSON *newitem);
extern void cJSON_ReplaceItemInObject(cJSON *object,const char *string,cJSON *newitem);

#define cJSON_AddNullToObject(object,name)	cJSON_AddItemToObject(object, name, cJSON_CreateNull())
#define cJSON_AddTrueToObject(object,name)	cJSON_AddItemToObject(object, name, cJSON_CreateTrue())
#define cJSON_AddFalseToObject(object,name)		cJSON_AddItemToObject(object, name, cJSON_CreateFalse())
#define cJSON_AddNumberToObject(object,name,n)	cJSON_AddItemToObject(object, name, cJSON_CreateNumber(n))
#define cJSON_AddStringToObject(object,name,s)	cJSON_AddItemToObject(object, name, cJSON_CreateString(s))

#ifdef __cplusplus
}
#endif

#endif
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
	            printf("thread_server read %d¡\\n",i);
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
       	printf(" send success length: %d¡\\n", ret);
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

APPS = alarm2server
OBJS = main.o cJSON.o
CC= arm-hisiv100nptl-linux-gcc
CFLAGS = -Wall -O -g -lm -lpthread

main: $(OBJS)
	$(CC) $(OBJS) -o $(APPS) -lm -lpthread

main.o: main.c cJSON.h
	$(CC) $(CFLAGS) -c main.c -o main.o

cJSON.o: cJSON.c cJSON.h
	$(CC) $(CFLAGS) -c cJSON.c -o cJSON.o

clean:
	rm -f *.o
ntries[i]);ptr+=strlen(entries[i]);
		if (i!=numentries-1) *ptr++=',';
		if (fmt) *ptr++='\n';*ptr=0;
		cJSON_free(names[i]);cJSON_free(entries[i]);
	}
	
	cJSON_free(names);cJSON_free(entries);
	if (fmt) for (i=0;i<depth-1;i++) *ptr++='\t';
	*ptr++='}';*ptr++=0;
	return out;	
}

/* Get Array size/item / object item. */
int    cJSON_GetArraySize(cJSON *array)							{cJSON *c=array->child;int i=0;while(c)i++,c=c->next;return i;}
cJSON *cJSON_GetArrayItem(cJSON *array,int item)				{cJSON *c=array->child;  while (c && item>0) item--,c=c->next; return c;}
cJSON *cJSON_GetObjectItem(cJSON *object,const char *string)	{cJSON *c=object->child; while (c && cJSON_strcasecmp(c->string,string)) c=c->next; return c;}

/* Utility for array list handling. */
static void suffix_object(cJSON *prev,cJSON *item) {prev->next=item;item->prev=prev;}
/* Utility for handling references. */
static cJSON *create_reference(cJSON *item) {cJSON *ref=cJSON_New_Item();if (!ref) return 0;memcpy(ref,item,sizeof(cJSON));ref->string=0;ref->type|=cJSON_IsReference;ref->next=ref->prev=0;return ref;}

/* Add item to array/object. */
void   cJSON_AddItemToArray(cJSON *array, cJSON *item)						{cJSON *c=array->child;if (!item) return; if (!c) {array->child=item;} else {while (c && c->next) c=c->next; suffix_object(c,item);}}
void   cJSON_AddItemToObject(cJSON *object,const char *string,cJSON *item)	{if (!item) return; if (item->string) cJSON_free(item->string);item->string=cJSON_strdup(string);cJSON_AddItemToArray(object,item);}
void	cJSON_AddItemReferenceToArray(cJSON *array, cJSON *item)						{cJSON_AddItemToArray(array,create_reference(item));}
void	cJSON_AddItemReferenceToObject(cJSON *object,const char *string,cJSON *item)	{cJSON_AddItemToObject(object,string,create_reference(item));}

cJSON *cJSON_DetachItemFromArray(cJSON *array,int which)			{cJSON *c=array->child;while (c && which>0) c=c->next,which--;if (!c) return 0;
	if (c->prev) c->prev->next=c->next;if (c->next) c->next->prev=c->prev;if (c==array->child) array->child=c->next;c->prev=c->next=0;return c;}
void   cJSON_DeleteItemFromArray(cJSON *array,int which)			{cJSON_Delete(cJSON_DetachItemFromArray(array,which));}
cJSON *cJSON_DetachItemFromObject(cJSON *object,const char *string) {int i=0;cJSON *c=object->child;while (c && cJSON_strcasecmp(c->string,string)) i++,c=c->next;if (c) return cJSON_DetachItemFromArray(object,i);return 0;}
void   cJSON_DeleteItemFromObject(cJSON *object,const char *string) {cJSON_Delete(cJSON_DetachItemFromObject(object,string));}

/* Replace array/object items with new ones. */
void   cJSON_ReplaceItemInArray(cJSON *array,int which,cJSON *newitem)		{cJSON *c=array->child;while (c && which>0) c=c->next,which--;if (!c) return;
	newitem->next=c->next;newitem->prev=c->prev;if (newitem->next) newitem->next->prev=newitem;
	if (c==array->child) array->child=newitem; else newitem->prev->next=newitem;c->next=c->prev=0;cJSON_Delete(c);}
void   cJSON_ReplaceItemInObject(cJSON *object,const char *string,cJSON *newitem){int i=0;cJSON *c=object->child;while(c && cJSON_strcasecmp(c->string,string))i++,c=c->next;if(c){newitem->string=cJSON_strdup(string);cJSON_ReplaceItemInArray(object,i,newitem);}}

/* Create basic types: */
cJSON *cJSON_CreateNull()						{cJSON *item=cJSON_New_Item();if(item)item->type=cJSON_NULL;return item;}
cJSON *cJSON_CreateTrue()						{cJSON *item=cJSON_New_Item();if(item)item->type=cJSON_True;return item;}
cJSON *cJSON_CreateFalse()						{cJSON *item=cJSON_New_Item();if(item)item->type=cJSON_False;return item;}
cJSON *cJSON_CreateBool(int b)					{cJSON *item=cJSON_New_Item();if(item)item->type=b?cJSON_True:cJSON_False;return item;}
cJSON *cJSON_CreateNumber(double num)			{cJSON *item=cJSON_New_Item();if(item){item->type=cJSON_Number;item->valuedouble=num;item->valueint=(int)num;}return item;}
cJSON *cJSON_CreateString(const char *string)	{cJSON *item=cJSON_New_Item();if(item){item->type=cJSON_String;item->valuestring=cJSON_strdup(string);}return item;}
cJSON *cJSON_CreateArray()						{cJSON *item=cJSON_New_Item();if(item)item->type=cJSON_Array;return item;}
cJSON *cJSON_CreateObject()						{cJSON *item=cJSON_New_Item();if(item)item->type=cJSON_Object;return item;}

/* Create Arrays: */
cJSON *cJSON_CreateIntArray(int *numbers,int count)				{int i;cJSON *n=0,*p=0,*a=cJSON_CreateArray();for(i=0;a && i<count;i++){n=cJSON_CreateNumber(numbers[i]);if(!i)a->child=n;else suffix_object(p,n);p=n;}return a;}
cJSON *cJSON_CreateFloatArray(float *numbers,int count)			{int i;cJSON *n=0,*p=0,*a=cJSON_CreateArray();for(i=0;a && i<count;i++){n=cJSON_CreateNumber(numbers[i]);if(!i)a->child=n;else suffix_object(p,n);p=n;}return a;}
cJSON *cJSON_CreateDoubleArray(double *numbers,int count)		{int i;cJSON *n=0,*p=0,*a=cJSON_CreateArray();for(i=0;a && i<count;i++){n=cJSON_CreateNumber(numbers[i]);if(!i)a->child=n;else suffix_object(p,n);p=n;}return a;}
cJSON *cJSON_CreateStringArray(const char **strings,int count)	{int i;cJSON *n=0,*p=0,*a=cJSON_CreateArray();for(i=0;a && i<count;i++){n=cJSON_CreateString(strings[i]);if(!i)a->child=n;else suffix_object(p,n);p=n;}return a;}
