/* This a simple test file for testing driver. */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define DEVICE "/dev/etx_device"  // Device file path

int main(int argc, char *argv[]) 
{
    int fd;
    char buffer[2];  // For storing the character to write
    buffer[1] = '\0';  // Set the second byte to null terminator

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ID>\n", argv[0]);
        return 1;
    }

    char *id = argv[1];
    int length = strlen(id);

    for (int i = 0; i < length; i++) {
        if (id[i] < '0' || id[i] > '9') {
            fprintf(stderr, "Error: ID must contain only digits\n");
            return 1;
        }
    }

    fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device");
        return 1;
    }

    // Write the ID to the device.
    // One character at a time.
    for (int i = 0; i < length; i++) {
        buffer[0] = id[i];
        write(fd, buffer, 1);
        printf("Write %c to the device\n", buffer[0]);\

        sleep(1);  // 等待 1 秒
    }

    close(fd);

    return 0;
}