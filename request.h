#ifndef __REQUEST_H__
#include "Queue.h"
void requestHandle(int fd,  struct Queue * waiting_ptr, struct Queue * running_ptr);

#endif
