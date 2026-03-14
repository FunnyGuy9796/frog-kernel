#include "init.h"

int main(int argc, char **argv) {
    int text_fd = open("/hello.txt", 0);

    if (text_fd < 0)
        exit(1);

    char text_buf[255];
    int bytes = read(text_fd, text_buf, sizeof(text_buf));

    if (bytes > 0)
        printf("%s\n", text_buf);
    
    time_t t;

    if (gettimeofday(&t) == 0) {
        uint8_t hour = t.hours % 12;
        const char *ampm = t.hours >= 12 ? "PM" : "AM";

        if (hour == 0)
            hour = 12;
        
        printf("\n%02d:%02d %s - %s %d, %d\n\n",
            hour, t.minutes, ampm,
            month[t.month - 1], t.day, t.year);
    }

    char line[256];
    char *argv_buf[MAX_ARGS];

    char *test_argv[] = { "/bin/test/test" };
    pid_t test_pid = spawn("/bin/test/test", 1, test_argv);

    if (test_pid < 0)
        printf("failed to spawn /bin/test/test\n\n");
    else
        printf("spawning /bin/test/test with PID = %d\n\n", test_pid);

    return 0;
}