#ifndef SM_H
#define SM_H
#include <arduino.h>

struct _state_machine_;

//state machine events callback (input)
typedef void (*sm_event_fn)(struct _state_machine_ *sm, uint8_t event, void* param);
//tsansition callback
typedef void (*ts_callback_fn)(struct _state_machine_* sm, uint8_t from_state, uint8_t to_state);
// state machine defines

typedef struct _state_machine_{
  uint8_t volatile state;  //current state
  sm_event_fn event_fn;
  ts_callback_fn ts_fn;
  uint8_t pend_event;
  void*  pend_param;
  uint8_t pending; 
} sm_t;





///////////////////////////////////////////////////////////////////////
/// public functions
///////////////////////////////////////////////////////////////////////
//initial a state machine
void sm_init(sm_t* sm, sm_event_fn event_fn, ts_callback_fn ts_fn, uint8_t inial_state);
// publish a event to state machine, case state tsansition
void sm_publish(sm_t* sm, uint8_t event, void* param);
// transition to another state
void sm_transition(sm_t* sm, uint8_t to_state);
//
void sm_trigger(sm_t* sm);


#endif
