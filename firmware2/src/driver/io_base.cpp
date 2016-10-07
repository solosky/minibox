#include "io_base.h"

void  buffer_init(io_buffer_t* buffer, byte_t* buff, uint8_t size){
  buffer->buff = buff;
  buffer->size = size;
  buffer->pos = 0;
}


void callback_init(callback_t* callback, callback_fn_t cb_fn, void* cb_param){
  callback->cb_fn = cb_fn;
  callback->cb_param = cb_param;
}


void callback_execute(callback_t* callback){
    callback->cb_fn(callback->cb_param);
}
