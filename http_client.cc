#include "minet_socket.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define BUFSIZE 1024

int main(int argc, char * argv[]) {

    char * server_name = NULL;
    int server_port    = -1;
    char * server_path = NULL;
    char * req         = NULL;
    bool ok            = false;

    /*parse args */
    if (argc != 5) {
	fprintf(stderr, "usage: http_client k|u server port path\n");
	exit(-1);
    }

    server_name = argv[2];
    server_port = atoi(argv[3]);
    server_path = argv[4];

    req = (char *)malloc(strlen("GET  HTTP/1.0\r\n\r\n") 
			 + strlen(server_path) + 1);  

    /* initialize */
    if (toupper(*(argv[1])) == 'K') { 
	minet_init(MINET_KERNEL);
    } else if (toupper(*(argv[1])) == 'U') { 
	minet_init(MINET_USER);
    } else {
	fprintf(stderr, "First argument must be k or u\n");
	exit(-1);
    }

    int soc;
    char buf[BUFSIZE];
    struct sockaddr_in saddr;
    
    /* make socket */
    if((soc = minet_socket(SOCK_STREAM)) < 0){
        //handle error
        minet_perror("socket");
    }
    /* get host IP address  */
	memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(server_port);

    /* Hint: use gethostbyname() */
    struct hostent * host;
    if( (host = gethostbyname(server_name)) == NULL){ //maybe give it server_name?
        minet_perror("gethostname");
    }
    /* set address */
    memcpy(&saddr.sin_addr.s_addr, host->h_addr, host->h_length);

    /* connect to the server socket */
    
    if (minet_connect(soc, &saddr) < 0) {
      // error processing;
      close(soc);
      minet_perror("client: connect");
    }

    /* send request message */
    sprintf(req, "GET %s HTTP/1.0\r\n\r\n", server_path);
    int send_val = minet_write(soc, req, strlen(req));

    /* wait till socket can be read. */
    /* Hint: use select(), and ignore timeout for now. */
    struct timeval tv;
    fd_set readfds;
	tv.tv_sec = 0;
	tv.tv_usec = 500000;
    FD_ZERO(&readfds);
    FD_SET(soc, &readfds);

    int ready = minet_select(soc+1, &readfds, NULL, NULL, NULL);
	if(FD_ISSET(soc, &readfds)) {}
	else{
        perror("cannot read from socket");
	}
    /* first read loop -- read headers */
	int bytes_read;
	buf[BUFSIZE-1] = '\0';
	bytes_read = minet_read(soc, buf, BUFSIZE-1);

    char *token = NULL;
    token = strtok(buf, "\n");

    char status_text[100];
    memcpy(&status_text, token, strlen(token));

    // get return code
	char return_code_txt[3];
	memcpy(&return_code_txt, &token[9], 3);
	int return_code = atoi(return_code_txt);
    bool header_flag = false;

    // check return code
    if(return_code == 200) {
        
        // parse rest of header
        while(token) {
            // set flag at end of header
            if(strcmp(token, "\r") == 0) {
                header_flag = true;
            }

            // if at end of header, start printing html payload
            if(header_flag) {
                printf("%s", token);
            }
            token = strtok(NULL, "\n");
        }
        
        // do the rest
            memset(buf, 0, BUFSIZE);
            while((bytes_read = minet_read(soc, buf, BUFSIZE-1)) > 0){
                buf[bytes_read] = '\0';
                printf("%s", buf);
            }
    } 
    else { 
        // TODO: print this shit to stderr (use minet_perror or somethign)
        ok = false;
        while(token) {
            printf("%s\n", token);
            token = strtok(NULL, "\n");
        }
        while((bytes_read = minet_read(soc, buf, BUFSIZE-1)) > 0){
            printf("%s", buf);
        } 
    }

    /* examine return code */   

    //Skip "HTTP/1.0"
    //remove the '\0'

    // Normal reply has return code 200

    /* print first part of response: header, error code, etc. */

    /* second read loop -- print out the rest of the response: real web content */

    /*close socket and deinitialize */
    free(req);
	minet_deinit();
    minet_close(soc);

    if (ok) {
	return 0;
    } else {
	return -1;
    }
}
