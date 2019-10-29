#include "server.h"

pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
//The thread function
void *connection_handler(void *);

int main(int argc, char *argv[])
{

    int server_fd, new_socket, valread;
    struct sockaddr_in address, client;
    int opt=1;
    int addrlen =sizeof(address);
    char buffer[1024]= {0};
    char *hello="Hello from server";

    //Creating socket file descriptor
    if ((server_fd=socket(AF_INET,SOCK_STREAM,0))==0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    printf("listening...\n");
    //Forcefully attaching socket to the port 8080
    if(setsockopt(server_fd,SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr=INADDR_ANY;
    address.sin_port = htons(PORT);

    //Forcefully attaching socket to the port 8080
    if (bind(server_fd,(struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if(listen(server_fd,3)<0) {
        perror("listen");

        exit(EXIT_FAILURE);
    }



    pthread_t thread_id;


    //Really not locking for any reason other than to make the point

    while( (new_socket = accept(server_fd, (struct sockaddr *)&address,(socklen_t*)&addrlen)) ) {
        //puts("Connection accepted");
        if(pthread_create(&thread_id, NULL, connection_handler,(void*)&new_socket )<0) {
            perror("Could not create thread");
            return 1;
        }

        //Now join the thread, so that we don't terminate before the thread
        pthread_join( thread_id, NULL);
        //puts("Handler assigned");


    }

    if( new_socket<0) {
        perror("accept failed");
        return 1;
    }


    /*

    if((new_socket = accept(server_fd, (struct sockaddr *)&address,(socklen_t*)&addrlen))<0)
    {
    	perror("accept");
    	exit(EXIT_FAILURE);
    }

    valread=read(new_socket,buffer,1024);
    printf("%s\n",buffer);
    send(new_socket, hello, strlen(hello),0);
    printf("Hello message sent\n");
    */
    return 0;
}



/* Handle connection for each client*/

void *connection_handler(void *server_fd)
{

    pthread_mutex_lock(&mutex);
    //Get the socket descriptor
    int sock = *(int*)server_fd;
    int read_size;
    char total_send[5000]= {0};
    char client_message[1000]= {0};
    int features=0,i=0,k=0;
    int directory=1;
    char status[50]= {0};
    char Content_type[50]= {0};
    char file_content[2000]= {0};
    //Receive a message from client
    if((read_size = recv(sock, client_message,1000,0))>0) {
        //end of string marker
        //client_message[read_size] = '\0';
        //send the message back to clientd
        printf("%s",client_message);
        strcpy(total_send,"HTTP/1.x ");


        char *pch;
        pch= strtok(client_message," " ":" "\n");
        //break the string
        while(pch !=NULL) {
            //get the file name
            if(features==1) {


                directory=1;

                if(pch[0]!='/') {

                    sprintf(status,"%d",status_code[BAD_REQUEST]);
                    strcat(status," BAD_REQUEST\n");
                    strcpy(Content_type," ");
                } else {
                    for(i=0; i<strlen(pch); i++) {
                        if(pch[i]=='.')
                            directory=0;
                    }
                    char search[50]= {0};
                    for(i=1; i<strlen(pch); i++)
                        search[i-1]=pch[i];
                    //printf("search:%s directory:%d\n",search,directory);
                    //directory
                    if(directory==1) {
                        DIR *dirp;
                        struct dirent *direntp;
                        dirp = opendir(search);
                        if(dirp !=NULL) {
                            while(1) {
                                direntp=readdir(dirp);
                                if(direntp==NULL)
                                    break;
                                else if(direntp->d_name[0] != '.') {
                                    //printf("%s\n", direntp->d_name);
                                    strcat(file_content,direntp->d_name);
                                    strcat(file_content," ");
                                }
                            }
                            closedir(dirp);
                            strcat(file_content,"\n");
                            sprintf(status,"%d",status_code[OK]);
                            strcat(status," OK\n");
                            strcpy(Content_type,"directory");
                        } else {
                            strcat(file_content,"\n");
                            sprintf(status,"%d",status_code[NOT_FOUND]);
                            strcat(status," NOT_FOUND\n");
                        }
                    }
                    // kind of file type
                    else {
                        //find if filename exist
                        char filename[20]= {0};
                        i=strlen(pch);
                        while(pch[i]!='.')
                            i--;
                        i++;
                        for(k=0; i<strlen(pch); i++) {
                            filename[k]=pch[i];
                            k++;
                        }
                        for(i=0; i<8; i++) {
                            if(strcmp(filename,extensions[i].ext)==0)
                                strcpy(Content_type,extensions[i].mime_type);
                        }
                        if(!isalpha(Content_type[0])) {
                            sprintf(status,"%d",status_code[UNSUPPORT_MEDIA_TYPE]);
                            strcat(status," UNSUPPORT_MEDIA_TYPE\n");
                        } else {
                            char buf[1024]= {0};
                            FILE *file;
                            file=fopen(search,"r");
                            if(file) {
                                sprintf(status,"%d",status_code[OK]);
                                strcat(status," OK\n");

                                //start to read the file
                                while(fgets(buf,sizeof(buf),file)!=NULL) {
                                    buf[strlen(buf)-1]='\0';
                                    strcat(file_content,buf);
                                    strcat(file_content,"\n");
                                }

                                fclose(file);
                            } else {
                                sprintf(status,"%d",status_code[NOT_FOUND]);
                                strcat(status," NOT_FOUND\n");
                            }

                        }

                    }

                }

                strcat(total_send,status);
                strcat(total_send,"Content-type: ");
                strcat(total_send,Content_type);
                strcat(total_send,"\nServer: httpserver/1.x\n\n");
                strcat(total_send,file_content);
                directory=1;
            }
            pch= strtok(NULL, " " ":" "\n");
            features++;
        }

        write(sock, total_send,strlen(total_send));

        //clear the message buffer
        //memset(client_message, 0, 100);

    }

    if(read_size==0) {
        puts("Client disconnected");
        fflush(stdout);
    } else if(read_size == -1) {
        perror("recv failed");
    }
    pthread_mutex_unlock(&mutex);
    return 0;
}
