#include "timestamp.h"
bool operator<(Timestamp lhs, Timestamp rhs) {
    return lhs.microSeconds() < rhs.microSeconds();
}

bool operator==(Timestamp lhs, Timestamp rhs) {
    return lhs.microSeconds() == rhs.microSeconds();
}

int64 operator-(Timestamp high, Timestamp low) {
    return static_cast<int64>(high.microSeconds() - low.microSeconds());
}

Timestamp Timestamp::now() {
    // struct timeval {
    //     time_t      tv_sec;     /* seconds */
    //     suseconds_t tv_usec;    /* microseconds */
    // };

    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64 secs = tv.tv_sec;
    return Timestamp(secs * Timestamp::kMicroSecondsPerSecond + tv.tv_usec);
}
