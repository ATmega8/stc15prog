#include "aio.h"
#include <aio.h>
#include <string.h>

void AIO_Init(int fd, unsigned char buf, void(callBack)(sigval_t))
{
    struct aiocb serial_aio;

    memset((char*)&serial_aio, '\0', sizeof(struct aiocb));
    serial_aio.aio_buf = &buf;
    serial_aio.aio_fildes = fd;
    serial_aio.aio_nbytes = sizeof(buf);
    serial_aio.aio_offset = 0;

    serial_aio.aio_sigevent.sigev_notify = SIGEV_THREAD;
    serial_aio.aio_sigevent.sigev_notify_function =
        callBack;
    serial_aio.aio_sigevent.sigev_notify_attributes = NULL;
    serial_aio.aio_sigevent.sigev_value.sival_ptr = &serial_aio;
}

