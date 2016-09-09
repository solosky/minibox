#include <arduino.h>

//通用的回调方法，通常在传输或者接受完毕后回调
typedef void (*callback_t)(void*);
typedef uint8_t byte_t;

//操作类型
enum op_t{
  NONE, READ, WRITE
};
//设备状态
enum d_state_t {
  BUSY, IDLE
};

enum io_state_t {
  IO_OK,
  IO_DRIVER_BUSY
};


//IO 请求结构体
typedef struct _io_req {
  uint8_t op; //read or write, NONE
  byte_t* buff; // 发送或者接受缓冲区缓冲区
  uint8_t len; //缓冲区长度
  uint8_t pos; //当前读或者写的位置
  callback_t callback;  //io操作完成后的回调地址
  void* cb_param; // io回调后的参数
} io_req_t;


//设备状态结构体
typedef struct _io_device{
  uint8_t state;  //当前设备状态 BUSY, IDLE
  io_req_t io_req;  //当前设备请求
} io_device_t;
