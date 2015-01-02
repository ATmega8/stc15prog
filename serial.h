#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

#define FALSE -1;
#define TRUE   0;

int SERIAL_Open(const char* devd);
int SERIAL_SetBaudRate(speed_t speed, int fd);
int SERIAL_SetDataParityStop(int dataBits, int stopBits,
int parity, int fd);

