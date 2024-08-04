//
// request.c: Does the bulk of the work for the web server.
// 
#include "segel.h"
#include "request.h"

extern pthread_mutex_t mtx;
// requestError(      fd,    filename,        "404",    "Not found", "OS-HW3 Server could not find this file");
void requestError(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg,  struct timeval arrival, struct timeval dispatch, threads_stats t_stats)
{
   char buf[MAXLINE], body[MAXBUF];

   // Create the body of the error message
   sprintf(body, "<html><title>OS-HW3 Error</title>");
   sprintf(body, "%s<body bgcolor=""fffff"">\r\n", body);
   sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
   sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
   sprintf(body, "%s<hr>OS-HW3 Web Server\r\n", body);

   // Write out the header information for this response
   sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
   Rio_writen(fd, buf, strlen(buf));
   printf("%s", buf);

   sprintf(buf, "Content-Type: text/html\r\n");
   Rio_writen(fd, buf, strlen(buf));
   printf("%s", buf);

   sprintf(buf, "Content-Length: %lu\r\n", strlen(body));

    sprintf(buf, "%sStat-Req-Arrival:: %lu.%06lu\r\n", buf, arrival.tv_sec, arrival.tv_usec);
    sprintf(buf, "%sStat-Req-Dispatch:: %lu.%06lu\r\n", buf, dispatch.tv_sec, dispatch.tv_usec);
    sprintf(buf, "%sStat-Thread-Id:: %d\r\n", buf, t_stats->id);
    sprintf(buf, "%sStat-Thread-Count:: %d\r\n", buf, t_stats->total_req);
    sprintf(buf, "%sStat-Thread-Static:: %d\r\n", buf, t_stats->stat_req);
    sprintf(buf, "%sStat-Thread-Dynamic:: %d\r\n\r\n", buf, t_stats->dynm_req);


    Rio_writen(fd, buf, strlen(buf));
   printf("%s", buf);
   // Write out the content
   Rio_writen(fd, body, strlen(body));
   printf("%s", body);

}


//
// Reads and discards everything up to an empty text line
//
void requestReadhdrs(rio_t *rp)
{
   char buf[MAXLINE];

   Rio_readlineb(rp, buf, MAXLINE);
   while (strcmp(buf, "\r\n")) {
      Rio_readlineb(rp, buf, MAXLINE);
   }
   return;
}

//
// Return 1 if static, 0 if dynamic content
// Calculates filename (and cgiargs, for dynamic) from uri
//
int requestParseURI(char *uri, char *filename, char *cgiargs) 
{
   char *ptr;

   if (strstr(uri, "..")) {
      sprintf(filename, "./public/home.html");
      return 1;
   }

   if (!strstr(uri, "cgi")) {
      // static
      strcpy(cgiargs, "");
      sprintf(filename, "./public/%s", uri);
      if (uri[strlen(uri)-1] == '/') {
         strcat(filename, "home.html");
      }
      return 1;
   } else {
      // dynamic
      ptr = index(uri, '?');
      if (ptr) {
         strcpy(cgiargs, ptr+1);
         *ptr = '\0';
      } else {
         strcpy(cgiargs, "");
      }
      sprintf(filename, "./public/%s", uri);
      return 0;
   }
}

//
// Fills in the filetype given the filename
//
void requestGetFiletype(char *filename, char *filetype)
{
   if (strstr(filename, ".html")) 
      strcpy(filetype, "text/html");
   else if (strstr(filename, ".gif")) 
      strcpy(filetype, "image/gif");
   else if (strstr(filename, ".jpg")) 
      strcpy(filetype, "image/jpeg");
   else 
      strcpy(filetype, "text/plain");
}

void requestServeDynamic(int fd, char *filename, char *cgiargs,  struct timeval arrival, struct timeval dispatch, threads_stats t_stats)
{
   char buf[MAXLINE], *emptylist[] = {NULL};

   // The server does only a little bit of the header.  
   // The CGI script has to finish writing out the header.
   sprintf(buf, "HTTP/1.0 200 OK\r\n");
   sprintf(buf, "%sServer: OS-HW3 Web Server\r\n", buf);

    // StatS
    sprintf(buf, "%sStat-Req-Arrival:: %lu.%06lu\r\n", buf, arrival.tv_sec, arrival.tv_usec);
    sprintf(buf, "%sStat-Req-Dispatch:: %lu.%06lu\r\n", buf, dispatch.tv_sec, dispatch.tv_usec);
    sprintf(buf, "%sStat-Thread-Id:: %d\r\n", buf, t_stats->id);
    sprintf(buf, "%sStat-Thread-Count:: %d\r\n", buf, t_stats->total_req);
    sprintf(buf, "%sStat-Thread-Static:: %d\r\n", buf, t_stats->stat_req);
    sprintf(buf, "%sStat-Thread-Dynamic:: %d\r\n", buf, t_stats->dynm_req);

   Rio_writen(fd, buf, strlen(buf));
   int pid = 0;
   if ((pid = Fork()) == 0) {
      /* Child process */
      Setenv("QUERY_STRING", cgiargs, 1);
      /* When the CGI process writes to stdout, it will instead go to the socket */
      Dup2(fd, STDOUT_FILENO);
      Execve(filename, emptylist, environ);
   }
   WaitPid(pid, NULL, WUNTRACED);
}


void requestServeStatic(int fd, char *filename, int filesize, struct timeval arrival, struct timeval dispatch, threads_stats t_stats)
{
   int srcfd;
   char *srcp, filetype[MAXLINE], buf[MAXBUF];

    requestGetFiletype(filename, filetype);

    srcfd = Open(filename, O_RDONLY, 0);

    // Rather than call read() to read the file into memory,
    // which would require that we allocate a buffer, we memory-map the file
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    Close(srcfd);

    // put together response
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: OS-HW3 Web Server\r\n", buf);
    sprintf(buf, "%sContent-Length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-Type: %s\r\n", buf, filetype);

    // StatS
    sprintf(buf, "%sStat-Req-Arrival:: %lu.%06lu\r\n", buf, arrival.tv_sec, arrival.tv_usec);
    sprintf(buf, "%sStat-Req-Dispatch:: %lu.%06lu\r\n", buf, dispatch.tv_sec, dispatch.tv_usec);
    sprintf(buf, "%sStat-Thread-Id:: %d\r\n", buf, t_stats->id);
    sprintf(buf, "%sStat-Thread-Count:: %d\r\n", buf, t_stats->total_req);
    sprintf(buf, "%sStat-Thread-Static:: %d\r\n", buf, t_stats->stat_req);
    sprintf(buf, "%sStat-Thread-Dynamic:: %d\r\n\r\n", buf, t_stats->dynm_req);

    Rio_writen(fd, buf, strlen(buf));

   //  Writes out to the client socket the memory-mapped file
   Rio_writen(fd, srcp, filesize);
   Munmap(srcp, filesize);

}

int ends_with_skip(const char *str) {
    if (!str) {
        return 0;
    }
    size_t len = strlen(str);
    if (len < 5) {
        return 0;
    }
    return (strcmp(str + len - 5, ".skip") == 0);
}
// handle a request
void requestHandle(Request_info request_info,  struct Queue * waiting_ptr, struct Queue * running_ptr, threads_stats t_stats)
{
    struct Queue * skip_to = createQueue();
//    Request_info top_request_info;
//    memset(&top_request_info, 0, sizeof(top_request_info));
    struct timeval arrival = request_info.arrival_time;
    struct timeval dispatch = request_info.dispatch_time;
   struct timeval dispatch_time_skip;
   memset(&dispatch_time_skip, 0, sizeof(dispatch_time_skip));
   Request_info top_request_info_skip;
   memset(&top_request_info_skip, 0, sizeof(top_request_info_skip));
    /*
    gettimeofday(&dispatch_time, NULL);
    dispatch_time.tv_sec = dispatch_time.tv_sec - arrival.tv_sec;
    dispatch_time.tv_usec = dispatch_time.tv_usec - arrival.tv_usec;
    */
    int is_static;
    int fd = request_info.fd;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, fd);
    Rio_readlineb(&rio, buf, MAXLINE);
    sscanf(buf, "%s %s %s", method, uri, version);

    printf("%s %s %s\n", method, uri, version);

    t_stats->total_req += 1;

    //TODO: what if .skip is error????
    if (strcasecmp(method, "GET")) {
      requestError(fd, method, "501", "Not Implemented", "OS-HW3 Server does not implement this method", arrival, dispatch, t_stats);
      return;
    }
    requestReadhdrs(&rio);
    is_static = requestParseURI(uri, filename, cgiargs);
    if (ends_with_skip(filename) == 1) {
        filename[strlen(filename)-5] = '\0';
        pthread_mutex_lock(&mtx);
        if(isEmpty(waiting_ptr) == 0){
            top_request_info_skip = popQueueFromEnd(waiting_ptr);
            gettimeofday(&dispatch_time_skip, NULL);
            timersub(&top_request_info_skip.arrival_time, &dispatch_time_skip,&top_request_info_skip.dispatch_time);
            //top_request_info.dispatch_time = dispatch_time_skip;
            enQueue(running_ptr, top_request_info_skip);
            enQueue(skip_to, top_request_info_skip);
        }
        pthread_mutex_unlock(&mtx);
    }
    if (stat(filename, &sbuf) < 0) {
      requestError(fd, filename, "404", "Not found", "OS-HW3 Server could not find this file", arrival, dispatch, t_stats);
      return;
    }

    if (is_static) {
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
         requestError(fd, filename, "403", "Forbidden", "OS-HW3 Server could not read this file", arrival, dispatch, t_stats);
         return;
        }
        t_stats->stat_req += 1;
        requestServeStatic(fd, filename, sbuf.st_size, arrival, dispatch, t_stats);
    } else {
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
            requestError(fd, filename, "403", "Forbidden", "OS-HW3 Server could not run this CGI program", arrival, dispatch, t_stats);
            return;
        }
        t_stats->dynm_req += 1;
        requestServeDynamic(fd, filename, cgiargs, arrival, dispatch, t_stats);
    }
    while(isEmpty(skip_to) == 0){
        Request_info req = popQueue(skip_to);
        // struct timeval disp = popQueue(skip_dispatch);
        requestHandle(req, waiting_ptr, running_ptr, t_stats);
    }

//    if(top_request_info.fd != 0){//this is a skip request
//        requestHandle(top_request_info, waiting_ptr, running_ptr, t_stats, dispatch_time_skip);
//    }
}


