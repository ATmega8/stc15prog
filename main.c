/*
 * L. 20140918
 */

#include "main.h"

int fd, nread, nwrite, readLen;
unsigned char readValue[8];
unsigned char* pRead;
char* ReceiveBuff[255];

int RxCount = 0;

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    /*serial init
     *B96008N1*/


    char dev[] = "/dev/ttyUSB0";
    char writeValue = 0x7F;
    uint8_t RxBuf[255];

    ReadSMTypeDef readSM;

    fd = SERIAL_Open(dev);

    if(SERIAL_SetBaudRate(B9600, fd) == -1)
    {
        printf("set baud error\n");
        return -1;
    }

    if(SERIAL_SetDataParityStop(8, 1, 'N', fd) == -1)
    {
        printf("set parity or data or stop error\n");
        return -1;
    }

    /*状态机初始化*/
    readSM.state = wait;
    readSM.pRxBuf = (uint8_t*)&RxBuf;
    readSM.receiveSum = 0;
    readSM.receiveCount = 0;
    readSM.receiveBuffIndex = 0;

    while(1)
    {

        /*循环发送 0x7F*/
        if((nwrite =  write(fd, &writeValue, 1)) < 0)
        {
            perror("write");
        }

        /*等待10ms*/
        usleep(1000);

        pRead = (uint8_t*)&readValue;
        readLen = 1;

        /*读取串口*/
        while(readLen != 0 && (nread = read(fd, (void*)pRead, readLen) != 0))
        {
            if(errno == EAGAIN)
                break;

            if(nread == -1)
            {
                if(errno == EINTR)
                    continue;
                perror("read");
                break;
            }

            readLen -= nread;
            pRead += nread;

        }

        pRead = (uint8_t*)&readValue;

        if(nread > 0)
        {
            while(nread--)
            {
                READ_Dispatch(&readSM, *pRead);
                pRead++;
            }
        }


        if(readSM.state == done)
            break;

    }

    return 0;
}

void READ_Dispatch(ReadSMTypeDef* readSM, uint8_t buf)
{
            switch(readSM->state)
            {
                case wait:       /*等待帧头*/
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
                   if(buf == 0xB9)
                   {
                       printf("READ_Dispatch: Get frame head\n");
                       readSM->state = head2;
                       return;
                   }
                   else
                   {
                       readSM->state = wait;
                       return;
                   }
                case head2:      /*帧头 0xB9*/
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
                   readSM->receiveSum = buf + 0x68;
                   readSM->receiveCount = buf - 6;
                   readSM->state = len;
                   return;

                case len:        /*数据长度*/
                   readSM->receiveSum += buf;
                   *(readSM->pRxBuf + readSM->receiveBuffIndex) = buf;
                   printf("index: %d value 0x%X\n", readSM->receiveBuffIndex, buf);

                   readSM->receiveBuffIndex++;

                   if(readSM->receiveBuffIndex == readSM->receiveCount)
                   {
                       readSM->state = checksum1;
                       return;
                   }
                   else
                       return;

                case checksum1:  /*校验和高位*/
                   if(buf == (readSM->receiveSum >> 8))
                   {
                       printf("READ_Dispatch:check sum high byte ok\nreceive: %d\nsum: %d\n", buf, (readSM->receiveSum >> 8));
                       readSM->state = checksum2;
                       return;
                   }
                   else
                   {
                       printf("READ_Dispat:check sum high byte error\nreceive: %d\nsum: %d\n", buf, (readSM->receiveSum >> 8));
                       readSM->state = wait;
                       return;
                   }

                case checksum2:  /*校验和低位*/
                   if(buf == (readSM->receiveSum & 0x000000FF))
                   {
                       printf("READ_Dispatch:check sum low byte ok\nreceive: %d\nsum: %d\n", buf, readSM->receiveSum & 0x000000FF);
                       readSM->state = tail;
                       return;
                   }
                   else
                   {
                       printf("READ_Dispath:check sum low byte error \nreceive: %d\nsum: %d\n", buf, readSM->receiveSum & 0x000000FF);
                       readSM->state = wait;
                       return;
                   }

                case tail:       /*尾包*/
                   if(buf == 0x16)
                   {
                       printf("READ_Dispath:OK\n");
                       readSM->state = done;
                       return;
                   }
                   else
                   {
                       printf("READ_Dispatch:check tail error \nreceive: %d\n", buf);
                       readSM->state = wait;
                       return;
                   }

                default:
                    printf("READ_Dispatch: Enter default loop\n");
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

