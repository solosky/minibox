#ifndef DRV_BASE_H
#define DRV_BASE_H
#include "io_base.h"

//操作类型
enum op_t {
        NONE, READ, WRITE
};
//设备状态
enum d_state_t {
        IDLE, BUSY
};

enum io_state_t {
        IO_OK,
        IO_DRIVER_BUSY
};

//IO 请求结构体
typedef struct _io_req {
        uint8_t op; //read or write, NONE
        io_buffer_t buff; // 发送或者接受缓冲区缓冲区
        callback_t callback; //io操作完成后的回调结构
} io_req_t;


//设备状态结构体
typedef struct _io_device {
        uint8_t state; //当前设备状态 BUSY, IDLE
        io_req_t io_req; //当前设备请求
} io_device_t;

#endif
