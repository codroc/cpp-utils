#include <sys/syscall.h>
#include <unistd.h>
using namespace std;
namespace CurrentThread {
__thread int tid_cache = 0;
// __thread const char *name_cache = 0;

pid_t gettid() { 
    if (tid_cache == 0)
        tid_cache = ::syscall(SYS_gettid);
    return static_cast<pid_t>(tid_cache);
}

}
