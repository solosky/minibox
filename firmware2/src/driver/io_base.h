#ifndef IO_BASE_H
#define IO_BASE_H
#include <arduino.h>


//通用的回调方法，通常在传输或者接受完毕后回调
typedef void (*callback_fn_t)(void*);
typedef uint8_t byte_t;

//缓冲区数据结构
typedef struct _buffer_ {
    byte_t* buff; //缓冲区内存指针
    uint8_t size; //缓冲区的长度
    uint8_t pos;  //当前读或者写位置
} io_buffer_t;

//回调数据结构
typedef struct _callback_ {
    callback_fn_t cb_fn;
    void* cb_param;
} callback_t;


//////////////////////////////////////////////////////////////////
// 缓冲区接口定义
//////////////////////////////////////////////////////////////////
//初始化buffer,注意缓冲区内存指针需要外部分配好
void buffer_init(io_buffer_t* buffer, byte_t* buff, uint8_t size);
//缓冲区写入一个字节，返回实际写入的字节数
//int16_t buffer_write(ring_buffer_t* buffer, byte_t one);
//缓冲区写入多个字节，返回实际写入的字节数
//int16_t buffer_write(ring_buffer_t* buffer, byte_t* buff, uint8_t size);
//缓冲区读去多个字节，返回实际读入的字节数，-1为读取到末尾
//int16_t buffer_read(ring_buffer_t* buffer, byte_t* buff, uint8_t size);
//缓冲区读取单个字节，返回实际读入的字节数，-1为读取到末尾
//int16_t buffer_read(ring_buffer_t* buffer);


//////////////////////////////////////////////////////////////////
// 回调定义
//////////////////////////////////////////////////////////////////
//初始化回调
void callback_init(callback_t* callback, callback_fn_t cb_fn, void* cb_param);
void callback_execute(callback_t* callback);























#endif
