/*
 * L. 20140918
 */

#include "main.h"
#include <aio.h>
#include <signal.h>

void serial_aio_handler(sigval_t sigval);


int fd, nread, nwrite;
volatile unsigned char readValue;
uint8_t RxBuf[255];
char* ReceiveBuff[255];

int RxCount = 0;

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    /*serial init
     *B1200 8N1*/


    char dev[] = "/dev/ttyUSB0";
    char writeValue = 0x7F;
    struct aiocb serial_aio, serial_aio_write;

    ReadSMTypeDef readSM;

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

    /*信号初始化*/
    memset((char*)&serial_aio, '\0', sizeof(struct aiocb));

    /*异步串口读写初始化*/
    serial_aio.aio_buf = &readValue;
    serial_aio.aio_fildes = fd;
    serial_aio.aio_nbytes = sizeof(readValue);
    serial_aio.aio_offset = 0;

    serial_aio_write.aio_buf = &writeValue;
    serial_aio_write.aio_fildes = fd;
    serial_aio_write.aio_nbytes = sizeof(writeValue);
    serial_aio_write.aio_offset = 0;

    /*串口回调函数初始化*/
    /*serial_aio.aio_sigevent.sigev_notify = SIGEV_THREAD;
    serial_aio.aio_sigevent.sigev_notify_function =
        serial_aio_handler;
    serial_aio.aio_sigevent.sigev_notify_attributes = NULL;
    serial_aio.aio_sigevent.sigev_value.sival_ptr = &serial_aio;*/


    /*状态机初始化*/
    readSM.state = wait;
    readSM.pRxBuf = RxBuf;
    readSM.receiveSum = 0;
    readSM.receiveCount = 0;
    readSM.receiveBuffIndex = 0;

    while(1)
    {

        /*循环发送 0x7F*/


        if((nwrite =  aio_write(&serial_aio)) < 0)
            perror("aio_write");

        /*等待10ms*/
        usleep(1000);

        /*读取串口*/
        if((nread = aio_read(&serial_aio)) < 0)
        {
            perror("aio_read");
        }

        while(aio_error(&serial_aio_write) == 0)
        {
            if((nread = aio_return(&serial_aio)) > 0)
            {
                READ_Dispatch(&readSM, readValue);
            }
            else
            {
                perror("aio_return");
            }
        }


    }

    return 0;
}

void READ_Dispatch(ReadSMTypeDef* readSM, uint8_t buf)
{
            switch(readSM->state)
            {
                case wait:       /*等待帧头*/
                   printf("enter wait");
                   if(buf == 0x46)
                   {
                       readSM->state = head1;
                       return;
                   }
                   else
                   {
                       readSM->state = wait;
                       return;
                   }

                case head1:      /*帧头 0x46*/
                   printf("enter head1");
                   if(buf == 0xB9)
                   {
                       readSM->state = head2;
                       return;
                   }
                   else
                   {
                       readSM->state = wait;
                       return;
                   }
                case head2:      /*帧头 0xB9*/
                   printf("enter head2");
                   if(buf == 0x68)
                   {
                       readSM->state = head3;
                       return;
                   }
                   else
                   {
                       readSM->state = wait;
                       return;
                   }

                case head3:      /*帧头 0x68*/
                   printf("enter head3");
                   if(buf == 0x00)
                   {
                       readSM->state = head4;
                       return;
                   }
                   else
                   {
                       readSM->state = wait;
                       return;
                   }

                case head4:      /*帧头 0x00*/
                   printf("enter head4");
                   readSM->receiveSum = buf + 0x68;
                   readSM->receiveCount = buf - 6;
                   readSM->state = len;
                   return;

                case len:        /*数据长度*/
                   printf("enter len\n");
                   readSM->receiveSum += buf;
                   *(readSM->pRxBuf + readSM->receiveBuffIndex++) =
                       buf;
                   printf("index: %d\n value: %d\n",
                           readSM->receiveBuffIndex, *(readSM->pRxBuf + readSM->receiveBuffIndex));

                   if(readSM->receiveBuffIndex == readSM->receiveCount)
                   {
                       readSM->state = checksum1;
                       return;
                   }
                   else
                       return;

                case checksum1:  /*校验和高位*/
                   printf("enter checksum1");
                   if(buf == (readSM->receiveSum >> 8))
                   {
                       printf("check sum high byte ok\nreceive: %d\nsum: %d\n", buf, (readSM->receiveSum >> 8));
                       readSM->state = checksum2;
                       return;
                   }
                   else
                   {
                       printf("check sum high byte error\nreceive: %d\nsum: %d\n", buf, (readSM->receiveSum >> 8));
                       readSM->state = wait;
                       return;
                   }

                case checksum2:  /*校验和低位*/
                   printf("enter checksum2");
                   if(buf == (readSM->receiveSum & 0x000000FF))
                   {
                       printf("check sum low byte ok\nreceive: %d\nsum: %d\n", buf, readSM->receiveSum & 0x000000FF);
                       readSM->state = tail;
                       return;
                   }
                   else
                   {
                       printf("check sum low byte error \nreceive: %d\nsum: %d\n", buf, readSM->receiveSum & 0x000000FF);
                       readSM->state = wait;
                       return;
                   }

                case tail:       /*尾包*/
                   printf("enter tail");
                   if(buf == 0x16)
                   {
                       printf("OK");
                       return;
                   }
                   else
                   {
                       printf("check tail error \nreceive: %d\n", buf);
                       readSM->state = wait;
                       return;
                   }

                default:
                    printf("default\n");
                    readSM->state = wait;
                    return;
            }
}

/*void serial_aio_handler(sigval_t sigval)
{
    struct aiocb *req;
    int ret;

    req = (struct aiocb *)sigval.sival_ptr;


    if(aio_error(req) == 0)
    {
        ret = aio_return(req);
        printf("ret:%d\n", ret);
        printf("buf:%d\n", readValue);

        while(ret > 0)
        {

        }
    }

    return;
}*/

