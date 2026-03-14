#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <process.h>

int main(int argc, char **argv) {
    int fb_fd = open("/dev/fb0", 0);

    if (fb_fd < 0)
        exit(1);

    uint32_t fb_buf[100];

    for (int i = 0; i < 100; i++)
        fb_buf[i] = 0xffff0000;

    write(fb_fd, (void *)fb_buf, 100);
    close(fb_fd);

    pid_t pid = getpid();
    pid_t ppid = getppid();

    printf("hello from /bin/test/test\n  pid = %d\n  parent pid = %d\n", (int)pid, (int)ppid);

    return 0;
}