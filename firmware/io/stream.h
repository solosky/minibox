#define byte_t char
#define RX_BUFFER_SIZE 16
#define TX_BUFFER_SIZE 16




enum device_status_t {
  BUSY,IDLE
}

enum device_mode_t {
  WRITE, READ, DUPLEX
}

enum device_event_t{
  READ_DATA, WRITE_DATA, READ_ERROR, WRITE_ERROR
}

int (*device_callback_t)(device_event_t event, byte_t data, void* ext);

int device_init(device_t* device);
int device_write(device_t* device, byte_t data, device_callback_t callback);
int device_read(device_t* device, device_callback_t callback);
int device_close(device_t* device);


struct device_define_t {
  device_init init_fn;
  device_write write_fn;
  device_read read_fn;
  device_close close_fn;
}

byte_t rx_buffer[RX_BUFFER_SIZE];
byte_t tx_buffer[TX_BUFFER_SIZE];




class IoDriver{
public:
  virtual int init();
  virtual int write(byte_t data);
  virtual int close();
}
