#include <stdio.h>
extern "C" int _ZN3CPP1iE;

namespace CPP{
    int i = 8;
}
int main() {
    printf("%d\n", _ZN3CPP1iE);
    return 0;
}

