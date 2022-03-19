#include "hook.h"
#include <dlfcn.h>
#include <stdio.h>

#define HOOK_FUNC(XX) \
    XX(sleep) \
    XX(usleep)

static thread_local bool t_hooked = false;

bool is_hooked() { return t_hooked; }
void set_hook_enable() { t_hooked = true; }
void set_hook_disable() { t_hooked = false; }


extern "C" {

#define XX(name) name ## _func name ## _f = nullptr;
    HOOK_FUNC(XX)
#undef XX

unsigned int sleep(unsigned int seconds) {
    if(!is_hooked())
        return sleep_f(seconds);
    printf("hook sleep success!\n");
    return 0;
}

int usleep(useconds_t usec) {
    if(!is_hooked())
        return usleep_f(usec);
    printf("hook usleep success!\n");
    return 0;
}

}


struct Init {
    Init() {
        // set_hook_enable();
#define XX(name) name ## _f = (name ## _func) ::dlsym(RTLD_NEXT, #name);
        HOOK_FUNC(XX)
#undef XX
    }
};
static Init init;
