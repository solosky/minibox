#ifndef IO_STREAM_H
#define IO_STREAM_H

#include "IoBuffer.h"
#include "IoDriver.h"

class IoStream : public IoHandler {

public:
  virtual void handle(IoEvent event, byte_t data);
  virtual bool open();
  virtual uint8_t read(byte_t* dst, uint8_t size);
  virtual uint8_t readFull(byte_t* dst, uint8_t size);
  virtual uint8_t write(byte_t* dst, uint8_t size);
  virtual bool close();
  virtual uint8_t err();

private:
  IoDriver* m_pIoDriver;
  IoBuffer m_txBuffer;
  IoBuffer m_rxBuffer;
  StaticSemaphore_t m_opMutex;
  StaticSemaphore_t m_txSem;
  StaticSemaphore_t m_rxSem;
  StaticSemaphore_t m_rxMutex;
  StaticSemaphore_t m_rxMutex;

  void handle(IoEvent event, byte_t data)
  {
    uint8_t pxHigherPriorityTaskWoken = 0;
    switch(event)
    {
      case READ_DATA:
        xSemaphoreTakeFromISR(&m_rxMutex, &pxHigherPriorityTaskWoken);
        m_rxBuffer.put(data);
        xSemaphoreGiveFromISR(&m_rxSem, &pxHigherPriorityTaskWoken);

    }
  }

};


#endif
