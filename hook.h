#ifndef HOOK_H
#define HOOK_H

#include <unistd.h>

bool is_hooked();
void set_hook_enable();


extern "C" {

// sleep
typedef unsigned int (*sleep_func)(unsigned int);
typedef int (*usleep_func)(useconds_t);

extern sleep_func sleep_f;
extern usleep_func usleep_f;

// socket

}
#endif
