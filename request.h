#ifndef __REQUEST_H__
#include "Queue.h"

typedef struct Threads_stats{
    int id;
    int stat_req;
    int dynm_req;
    int total_req;
} * threads_stats;


// returns the fd of the skip, if it is a skip
// otherwise returns 0
int requestHandle(Request_info fd,  struct Queue * waiting_ptr, struct Queue * running_ptr, threads_stats t_stats);

#endif
