#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    int fd; /* file descriptor */
    int i, j; /* loop variables */
    char input = 0;

    /* open device file for reading and writing */
    /* use 'open' to open '/dev/multiplier' */
    /* handle error opening file */
    fd = open("/dev/multiplier", O_RDWR);
    if (fd == -1) {
        printf("Failed to open device file!\n");
        return -1;
    }
    int inputt[3]; // 12 bytes
    char buffer[12];
    while (input != 'q') { /* continue unless user entered 'q' */
        for (i = 0; i <= 16; i++) {
            for (j = 0; j <= 16; j++) {
                /* write values to registers using char dev */
                /* use write to write i and j to peripheral */
                /* read i, j, and result using char dev */
                /* use read to read from peripheral */
                /* print unsigned int to screen */
                inputt[0] = i;
                inputt[1] = j;
                write(fd, (char *)inputt, 2*sizeof(int));
                read(fd, buffer, 12);
                inputt[0] = *((int*)buffer + 0);
                inputt[1] = *((int*)buffer + 1);
                inputt[2] = *((int*)buffer + 2);
                printf("%u * %u = %u\n", inputt[0], inputt[1], inputt[2]);
                /* validate result */
                if (inputt[2] == (i * j))
                    printf("Result Correct!\n");
                else
                    printf("Result Incorrect!\n");
                /* read from terminal */
                input = getchar();
            }
        }
    }
    close(fd);
    return 0;
}
