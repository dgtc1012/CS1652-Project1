/* Daniel Mcgrath <djm158@pitt.edu> 
   Dannah Gersh   <drg@pitt.edu>    
   
   huge thanks to Beej's Guide to Network Programming
*/

#include "minet_socket.h" 
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>

#define BUFSIZE 1024
#define FILENAMESIZE 100

int handle_connection(int client);

int main(int argc, char * argv[]) {
    int server_port = -1;
    int rc          =  0;

    // for selecting
    fd_set master;
    fd_set read_fds;
    int fdmax;

    int listener;
    int newfd;

    int i;

    // init file descriptor sets
    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    /* parse command line args */
    if (argc != 3) {
        fprintf(stderr, "usage: http_server2 k|u port\n");
        exit(-1);
    }

    if (toupper(*(argv[1])) == 'K') { minet_init(MINET_KERNEL);
    } else if (toupper(*(argv[1])) == 'U') { 
	minet_init(MINET_USER);
    } else {
	fprintf(stderr, "First argument must be k or u\n");
	exit(-1);
    }

    server_port = atoi(argv[2]);

    if (server_port < 1500) {
        fprintf(stderr, "INVALID PORT NUMBER: %d; can't be < 1500\n", server_port);
        exit(-1);
    }
    
    /* initialize and make socket */
    struct sockaddr_in saddr;
	memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(server_port);

    if((listener = minet_socket(SOCK_STREAM)) < 0){
        //handle error
        minet_perror("minet socket error\n");
        exit(-1);
    }

    if(minet_bind(listener, &saddr) < 0) {
        // handle error
        minet_perror("minet bind error\n");
        exit(-1);
    }

    if(minet_listen(listener, 32) < 0) {
        // handle error
        minet_perror("minet listen error\n");
        exit(-1);
    }
    
    FD_SET(listener, &master);
    fdmax = listener;
    /* set server address*/

    /* bind listening socket */

    /* start listening */

    /* connection handling loop: wait to accept connection */

    while (1) {
        read_fds = master;
        if(minet_select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            // handle select error
            minet_perror("select error");
            exit(-1);
        }

        for(i = 0; i <= fdmax; i++) {
            // is the selected socket in the set
            if(FD_ISSET(i, &read_fds)) {
                // check if there is a new connection pending
                if(i == listener) {
                    // handle new connections
                    newfd = minet_accept(listener, &saddr);
                    if(newfd == -1) {
                        minet_perror("accept error");
                    } else {
                        // add the new connection to the master set
                        FD_SET(newfd, &master);
                        if(newfd > fdmax) {
                            fdmax = newfd;
                        }
                    }
                } // end if i == listener
                else { // no new connection, need to handle data from clients
	                rc = handle_connection(i);
                    FD_CLR(i, &master);
                }
            } // end if FD_ISSET
        } // end for loop
	
	/* create read list */
	
	/* do a select */
	
	/* process sockets that are ready */
	
	/* for the accept socket, add accepted connection to connections */
	
	/* for a connection socket, handle the connection */
	
    // this shouldn't be listener
	
    }
}

int handle_connection(int client) {
    bool ok = true;

    char * ok_response_f = "HTTP/1.0 200 OK\r\n"	\
	"Content-type: text/plain\r\n"			\
	"Content-length: %d \r\n\r\n";
 
    char * notok_response = "HTTP/1.0 404 FILE NOT FOUND\r\n"	\
	"Content-type: text/html\r\n\r\n"			\
	"<html><body bgColor=black text=white>\n"		\
	"<h2>404 FILE NOT FOUND</h2>\n"
	"</body></html>\n";
    //int client;
    int len;
    char buf[BUFSIZE];

    char *token = NULL;
    char filename[FILENAMESIZE];


        buf[BUFSIZE-1] = '\0';
        if((len = minet_read(client, buf, BUFSIZE-1)) <= 0) {
            // handle error
            minet_perror("minet read error");
            ok = false;
        }
        /* first read loop -- get request and headers*/
        
        token = strtok(buf, " ");
        token = strtok(NULL, " ");
        strcpy(filename, token);

        /* parse request to get file name */
        /* Assumption: this is a GET request and filename contains no spaces*/

        /* try opening the file */
        FILE * f = fopen(filename, "r");
        if(f == NULL) {
            ok = false;
        }

        /* send response */
        if (ok) {
        /* send headers */
            minet_write(client, ok_response_f, strlen(ok_response_f));
        /* send file */
            while(fgets(buf, BUFSIZE-1, f) != NULL) {
                // write to socket
                minet_write(client, buf, BUFSIZE-1);
            }
            
        } else {
        // send error response
            minet_write(client, notok_response, strlen(notok_response));
        }
        minet_close(client);
    //}
    
    
    /* close socket and free space */
    //minet_close(sock);
  
    if (ok) {
	return 0;
    } else {
	return -1;
    }
}
