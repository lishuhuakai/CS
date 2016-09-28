#include <ulib.h>

int main(void);

void
umain(void) {
    int ret = main(); // 哪里调用main函数，main函数是系统调用嘛？
    exit(ret);
}

