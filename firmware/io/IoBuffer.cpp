#include "IoBuffer.h"
#include <string.h>

IoBuffer::IoBuffer(){
  memset( buff, 0, sizeof(buff));
}

inline byte_t IoBuffer::get()
{
  if(pos >= limit){
    return -1;
  }
  return buff[pos++];
}

inline uint8_t IoBuffer::get(byte_t* dst, uint8_t size)
{
    if(pos >= limit){
      retur -1;
    }
    uint8_t size = min(limit - pos, size);
    memcpy(dst, buff + pos, size);
    pos += size;
    return size;
}

inline void IoBuffer::put(byte_t data)
{
  if(limit < cap){
      buff[limit++] = data;
  }
}

inline uint8_t IoBuffer::pos()
{
  return pos;
}

inline uint8_t IoBuffer::remain()
{
  return limit - pos;
}

inline void IoBuffer::flip()
{
    limit = pos;
    pos = 0;
}

inline void IoBuffer::mark()
{
  mark = pos;
}

inline void IoBuffer::reset()
{
    pos = mark;
}

inline void IoBuffer::clear()
{
  pos = 0;
  mark = 0;
  limit = 0;
}
