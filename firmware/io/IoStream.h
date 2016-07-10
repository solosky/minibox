#ifndef IO_STREAM_H
#define IO_STREAM_H

#include "IoBuffer.h"
#include "IoDriver.h"

class IoStream : public IoHandler {

public:
  virtual void handle(IoEvent event, byte_t data);
  virtual bool open();
  virtual uint8_t read(byte_t* dst, uint8_t size);
  virtual uint8_t write(byte_t* dst, uint8_t size);
  virtual bool close();
  virtual uint8_t err();

private:
  IoDriver* m_pIoDriver;
  IoBuffer m_txBuffer;
  IoBuffer m_rxBuffer;
  StaticSemaphore_t m_mutex;

};


#endif
