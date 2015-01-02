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
    struct aiocb serial_aio;

    fd = SERIAL_Open(dev);

    if(SERIAL_SetBaudRate(B1200, fd) == -1)
    {
        printf("set baud error\n");
        return -1;
    }

    if(SERIAL_SetDataParityStop(8, 1, 'N', fd) == -1)
    {
        printf("set parity or data or stop error\n");
        return -1;
    }

    memset((char*)&serial_aio, '\0', sizeof(struct aiocb));

    /*异步串口读写初始化*/
    serial_aio.aio_buf = &buf;
    serial_aio.aio_fildes = fd;
    serial_aio.aio_nbytes = sizeof(buf);
    serial_aio.aio_offset = 0;

    /*串口回调函数初始化*/
    serial_aio.aio_sigevent.sigev_notify = SIGEV_THREAD;
    serial_aio.aio_sigevent.sigev_notify_function =
        serial_aio_handler;
    serial_aio.aio_sigevent.sigev_notify_attributes = NULL;
    serial_aio.aio_sigevent.sigev_value.sival_ptr = &serial_aio;


    while(1)
    {

        /*循环发送 0x7F*/
        nwrite =  write(fd, TXBuf, 1);
        if(nwrite < 0)
            perror("write");

        /*等待10ms*/
        usleep(10000);

        aio_read(&serial_aio);

    }

    return 0;
}

void serial_aio_handler(sigval_t sigval)
{
    struct aiocb *req;
    int ret;

    req = (struct aiocb*)sigval.sival_ptr;

    if(aio_error(req) == 0)
    {
        ret = aio_return(req);
        printf("ret:%d\n", ret);

        if(ret > 0)
        {

        }
    }

    return;
}

