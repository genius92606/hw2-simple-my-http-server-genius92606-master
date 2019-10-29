#include "client.h"

void *connection_handler(void *);
int main(int argc, char **argv[])
{
    mkdir("output",0700);
    chdir("output");
    mkdir("testdir",0700);
    mkdir("testdir/secfolder",0700);
    mkdir("testdir/secfolder/trifolder",0700);

    connection_handler((void *)argv[2]);
    return 0;

}

void *connection_handler(void *gosend)
{
    char sendcopy[1024];
    strcpy(sendcopy,(char *)gosend);
    struct sockaddr_in serv_addr;
    int sock,valread,i=0,k=0;
    int tom=0;
    int direct=0;
    char buffer[5000]= {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0))<0) {
        printf("\n Socket creation error \n");
        return -1;
    }
    memset(&serv_addr,'0',sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(8080);

    //Convert IPv4 and IPv6 addresses from text tot binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    if(connect(sock,(struct sock_addr *)&serv_addr, sizeof(serv_addr))<0) {
        printf("\nConnection Failed\n");
        return -1;
    }
    char *total_send=(char *)malloc(sizeof(char)*128);
    strcpy(total_send,"GET ");
    strcat(total_send,sendcopy);
    strcat(total_send," HTTP/1.x\r\nHOST:");
    strcat(total_send,"127.0.0.1");
    strcat(total_send,":");
    strcat(total_send,"8080");
    strcat(total_send,"\r\n\r\n");

    send(sock, total_send, strlen(total_send),0);
    valread= read( sock, buffer,5000);
    printf("%s\n",buffer);

    int isDirectory=0;
    int good=0;
    pthread_t thread_send;
    char directs[10][30];
    char *token;
    //printf("%d\n",strstr(buffer,": directory"));
    if(strstr(buffer,": directory")>0) isDirectory=1;
    if(strstr(buffer,"200 OK")>0) good=1;
    if(good==1) {
        if(isDirectory==1) {
            char *sendthread=(char *)malloc(sizeof(char)*128);
            memset(sendthread,'\0',sizeof(char)*128);
            // printf("%s",strstr(buffer," "));
            strtok(buffer,"\n" " ");
            strtok(NULL,"\n" " ");
            strtok(NULL,"\n" " ");
            strtok(NULL,"\n" " ");
            strtok(NULL,"\n" " ");
            strtok(NULL,"\n" " ");
            strtok(NULL,"\n" " ");
            token=strtok(NULL,"\n" " ");
            int file_index=0;
            while(token !=NULL ) {
                strcpy(directs[file_index],token);
                token=strtok(NULL,"\n" " ");
                file_index++;
            }

            for(i=0; i<file_index; i++) {
                strcpy(sendthread,sendcopy);
                if(sendthread[strlen(sendcopy)-1]!='/')
                    strcat(sendthread,"/");
                strcat(sendthread,directs[i]);
                pthread_create(&thread_send, NULL, connection_handler,(void *)sendthread);
                sleep(1);
                pthread_join(thread_send,NULL);
            }
            free(sendthread);

        } else {
            char *filename=(char *)malloc(sizeof(char)*1024);
            for(i=1; i<strlen(sendcopy); i++)
                filename[i-1]=sendcopy[i];
            /*i=strlen(sendcopy);
                while(sendcopy[i]!='/')
                    i--;
                i++;

            char smallname[20];
            for(k=0; i<strlen(sendcopy); i++) {
                    smallname[k]=sendcopy[i];
                    k++;
            }
            strcat(filename,smallname);*/
            FILE *fp;
            fp=fopen(filename,"w");
            char many[2048];
            char *tmp=NULL;
            strtok_r(buffer,"\n",&tmp);
            strtok_r(NULL,"\n",&tmp);
            strtok_r(NULL,"\n",&tmp);
            strcpy(many,tmp);
            for(i=1; i<strlen(tmp); i++)
                many[i-1]=tmp[i];
            fprintf(fp,many);
            fclose(fp);
            free(filename);
        }
    }



    pthread_exit(NULL);
    //printf("%s\n",sendcopy);
    //free(sendcopy);
    //free(thread_send2);



}