#include "sm.h"

void sm_init(sm_t* sm, sm_event_fn event_fn, ts_callback_fn ts_fn, uint8_t inial_state){
  sm->state = inial_state;
  sm->event_fn = event_fn;
  sm->ts_fn = ts_fn;
}
// publish a event to state machine, case state tsansition
void sm_publish(sm_t* sm, uint8_t event, void* param){
  //printf("%d ", event);
  sm->pend_event = event;
  sm->pend_param = param;
  sm->pending = 1;
}
// tsansition to another state
void sm_transition(sm_t* sm, uint8_t to_state){
  uint8_t from_state = sm->state;
  sm->state = to_state;
  sm->ts_fn(sm, from_state, to_state);
}

void sm_trigger(sm_t* sm){
  if(sm->pending){
    //printf("%d ", sm->pend_event);
    sm->event_fn(sm, sm->pend_event, sm->pend_param);
    sm->pending = 0;
  }
}
