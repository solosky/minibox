#ifndef IO_BUFER_H

#define DEFAULT_BUFFER_SIZE 16

typedef signed char byte_t

class IoBuffer {

public:
  IoBuffer():cap(0),pos(0), limit(0), mark(0);
  byte_t get();
  uint8_t get(byte_t* dst, uint8_t size);
  void put(byte_t data);
  uint8_t cap();
  uint8_t pos();
  uint8_t remain();
  void flip();
  void mark();
  void reset();
  void clear();

private:
    byte_t buff[DEFAULT_BUFFER_SIZE];
    uint8_t cap;
    uint8_t pos;
    uint8_t limit;
    uint8_t mark;
};

#endif
