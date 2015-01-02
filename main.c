/*
 * L. 20140918
 */

#include "main.h"
#include <aio.h>

void serial_aio_handler(sigval_t sigval);

int fd, nread, nwrite;
unsigned char buf;
char RxBuf[255];
int RxCount = 0;

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    /*serial init
     *B1200 8N1*/


    char dev[] = "/dev/ttyUSB0";
    char TXBuf[] = {0x7F};


    fd = SERIAL_Open(dev);
    SERIAL_SetBaudRate(B1200, fd);

    if(SERIAL_SetDataParityStop(8, 1, 'N', fd) == -1)
    {
        printf("set parity or data or stop error\n");
        return -1;
    }

    while(1)
    {

        /*循环发送 0x7F*/
        nwrite =  write(fd, TXBuf, 1);
        if(nwrite < 0)
            perror("write");

        /*等待10ms*/
        usleep(10000);

        nread = aio_read(&serial_aio);

        /*if(nread < 0)
            perror("aio_read");

        if(aio_error(&serial_aio) != EINPROGRESS )
        {
            printf("OK\n");
            if((nread = aio_return(&serial_aio)) > 0)
                printf("\n%s", buf);
        }*/
    }

    return 0;
}

void serial_aio_handler(sigval_t sigval)
{
    struct aiocb *req;
    int ret /*, i*/;

    req = (struct aiocb*)sigval.sival_ptr;

    if(aio_error(req) == 0)
    {
        ret = aio_return(req);
        printf("ret:%d\n", ret);

        if(ret > 0)
        {
            switch(RxCount)
            {
                case 1:
                    if(buf != 0xB9)
                        return;
                    RxCount++;
                    break;

                case 2:
                    if(buf != 0x68)
                        return;
                    RxCount++;
                    break;
            }
        }
        /*printf("Return Data:\n");
        for(i = 0; i < ret; i++)
            printf("0x%X\n", buf);8*/
    }

    return;
}

