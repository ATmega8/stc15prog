#include "main.h"

int FRAME_ReadFrame(FrameTypeDef* frame, int fd)
{
    /*激活帧*/
    const uint8_t writeValue = 0x7F;

    /*读写返回状态*/
    int nwrite, nread;

    /*读缓存*/
    int8_t readBuf[255];

    /*缓存指针*/
    uint8_t* pRead = (uint8_t*)&readBuf;

    /*创建状态机*/
    ReadSMTypeDef readSM;

    /*状态机初始化*/
    readSM.state = wait;
    readSM.pRxBuf = frame->frameBuf;
    readSM.receiveSum = 0;
    readSM.receiveCount = 0;
    readSM.receiveBuffIndex = 0;

    while(1)
    {
        /*循环发送 0x7F*/

        if(readSM.state == wait)
        {
            if((nwrite =  write(fd, &writeValue, 1)) < 0)
                perror("write");
        }

        /*等待10ms*/
        usleep(1000);

        /*读取串口*/
        pRead = (uint8_t*)&readBuf;

        while((nread = read(fd, (void*)pRead, 1)) != 0)
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

            /*指针地址自增*/
            pRead += nread;
        }

        pRead = (uint8_t*)&readBuf;

        if(nread > 0)
        {

            while(nread--)
            {
                /*执行状态机*/
                FRAME_ReadSMDispatch(&readSM, *pRead);
                pRead++;
            }
        }

        if(readSM.state == done)
            break;
    }

    frame->frameLen = readSM.receiveBuffIndex;

    return 0;
}

void FRAME_ReadSMDispatch(ReadSMTypeDef* readSM, uint8_t buf)
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
                       printf("READ_Dispatch:check sum high byte ok\n");
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
                       printf("READ_Dispatch:check sum low byte ok\n");
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

