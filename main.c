/*
 * L. 20140918
 */

#include "main.h"


int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    /*serial init
     *B96008N1*/


    const char dev[] = "/dev/ttyUSB0";
    int fd;
    uint8_t RxBuf[255];

    FrameTypeDef frame;

    frame.frameBuf   = (uint8_t*)&RxBuf;
    frame.frameLen   = 0;
    frame.frameClass = Data;

    fd = SERIAL_Open(dev);

    if(SERIAL_SetBaudRate(B9600, fd) == -1)
    {
        printf("SERIAL_SetBaudRate:set baud error\n");
        return -1;
    }

    if(SERIAL_SetDataParityStop(8, 1, 'N', fd) == -1)
    {
        printf("SERIAL_SetDataParityStop:set parity or data or stop error\n");
        return -1;
    }

    printf("Open serial OK\n");

    FRAME_StartFrame(&frame, fd);

    return 0;
}

