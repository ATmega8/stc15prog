#include "serial.h"
#include <stdint.h>

typedef enum
{
    wait,       /*等待帧头*/
    head1,      /*帧头 0x46*/
    head2,      /*帧头 0xB9*/
    head3,      /*帧头 0x68*/
    head4,      /*帧头 0x00*/
    len,        /*数据长度*/
    checksum1,  /*校验和高位*/
    checksum2,  /*校验和低位*/
    tail        /*尾包*/
} ReadStateTypeDef;

typedef struct
{
    volatile ReadStateTypeDef state;
    uint8_t*          pRxBuf;
    uint32_t receiveSum;
    uint32_t          receiveCount;
    uint32_t          receiveBuffIndex;
    volatile uint8_t  receiveOK;
} ReadSMTypeDef;

/*functions*/
void READ_Dispatch(ReadSMTypeDef* readSM, uint8_t buf);

