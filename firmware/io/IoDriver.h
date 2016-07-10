#ifndef IO_DRIVER_H
#define IO_DRIVDER_H

#include "IoBuffer.h"

enum IoStatus{
  OK, ERR
};

enum IoEvent {
  READ_DATA, WRITE_DATA, READ_ERROR, WRITE_ERROR
};

class IoHandler {
public:
  virtual void handle(IoEvent event, byte_t data) = 0;
};


class IoDriver {
public:
    IoStatus open() = 0;
    IoStatus close() = 0;
    IoStatus write(byte_t data) = 0;
    void handler(IoHandler* pIoHandler) = 0;
};
#endif
