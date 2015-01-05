#include "serial.h"

int SERIAL_Open(const char* dev)
{
    int fd;

    fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK);

    if(fd == -1)
        printf("open_port:Cannot open port:%s", dev);
    else
    {
        //fcntl(fd, F_SETOWN, getpid());
        //fcntl(fd, 0, FASYNC);
        fcntl(fd, F_SETFL, FNDELAY);

    }

    return fd;
}

int SERIAL_SetBaudRate(speed_t speed, int fd)
{
    struct termios opt;
    tcgetattr(fd, &opt);

    tcflush(fd, TCIOFLUSH);

    if(cfsetispeed(&opt, speed) == -1)
    {
        printf("setBaudRate: set in speed error\n");
        return FALSE;
    }

    if(cfsetospeed(&opt, speed) == -1)
    {
        printf("setBaudRate: set out speed error\n");
        return FALSE;
    }

    if(tcsetattr(fd, TCSANOW, &opt) != 0)
    {
        printf("setBaudRate: set tcattr error\n");
        return FALSE;
    }

    tcflush(fd, TCIOFLUSH);

    return TRUE;
}

int SERIAL_SetDataParityStop(int dataBits, int stopBits,
        int parity, int fd)
{
    struct termios opt;

    if(tcgetattr(fd, &opt) != 0)
    {
        printf("setDataParityStop: get tcattr error\n");
        return FALSE;
    }

    opt.c_cflag &= ~CSIZE;

    switch(dataBits)
    {
        case 7:
            opt.c_cflag |= CS7;
            break;

        case 8:
            opt.c_cflag |= CS8;
            break;

        default:
            printf("setDataParityStop:Unsupported data size\n");
            return FALSE;
            break;
    }

    switch(parity)
    {
        case 'N':
            opt.c_cflag &= ~PARENB;
            opt.c_iflag &= ~INPCK;
            break;

        case 'O':
            opt.c_cflag |= (PARODD | PARENB);
            opt.c_iflag &= ~INPCK;
            break;

        case 'E':
            opt.c_cflag |= PARENB;
            opt.c_iflag &= ~PARODD;
            opt.c_iflag |= INPCK;
            break;

        case 'S':
            opt.c_cflag &= ~PARENB;
            opt.c_cflag &= ~CSTOPB;
            break;

        default:
            printf("setDataParityStop: Unsupported parity\n");
            return FALSE;
            break;
    }

    switch(stopBits)
    {
        case 1:
            opt.c_cflag &= ~CSTOPB;
            break;

        case 2:
            opt.c_cflag |= CSTOPB;
            break;

        default:
            printf("setDataParityStop: Unsupported stopBits\n");
            return FALSE;
            break;

    }

    tcflush(fd, TCIFLUSH);
    opt.c_cc[VTIME] = 150;
    opt.c_cc[VMIN] = 0;

    if(tcsetattr(fd, TCSANOW, &opt) != 0)
    {
        printf("setDataParityStop: set tcattr error\n");
        return FALSE;
    }

    return TRUE;
}


