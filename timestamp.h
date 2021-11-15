#ifndef TIMESTAMP_H
#define TIMESTAMP_H
#include <sys/time.h>
#include <cstddef>
typedef long long int64;
class Timestamp {
public:
    Timestamp()
        : _microSeconds(0)
    {}
    Timestamp(int64 microSeconds)
        : _microSeconds(microSeconds)
    {}

    int64 microSeconds() { return _microSeconds; }

    bool valid() { return _microSeconds > 0; }
    static Timestamp now();
    static const int64 kMicroSecondsPerSecond = 1000 * 1000;
private:
    int64 _microSeconds;
};
bool operator<(Timestamp lhs, Timestamp rhs);
bool operator==(Timestamp lhs, Timestamp rhs);
int64 operator-(Timestamp high, Timestamp low);
// inline bool operator<(Timestamp lhs, Timestamp rhs) {
//     return lhs.microSeconds() < rhs.microSeconds();
// }
// 
// inline bool operator==(Timestamp lhs, Timestamp rhs) {
//     return lhs.microSeconds() == rhs.microSeconds();
// }
// 
// inline int64 operator-(Timestamp high, Timestamp low) {
//     return static_cast<int64>(high.microSeconds() - low.microSeconds());
// }
// 
// Timestamp Timestamp::now() {
//     // struct timeval {
//     //     time_t      tv_sec;     /* seconds */
//     //     suseconds_t tv_usec;    /* microseconds */
//     // };
// 
//     struct timeval tv;
//     gettimeofday(&tv, NULL);
//     int64 secs = tv.tv_sec;
//     return Timestamp(secs * Timestamp::kMicroSecondsPerSecond + tv.tv_usec);
// }

#endif

