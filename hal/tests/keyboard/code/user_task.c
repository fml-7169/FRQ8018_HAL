#include <stdint.h>

#include "os_task.h"
#include "os_msg_q.h"

#include "co_printf.h"
#include "user_task.h"
#include "button.h"

uint16_t user_task_id;

static int user_task_func(os_event_t *param)
{
    switch(param->event_id)
    {
        case USER_EVT_BUTTON:
            {
                struct button_msg_t *button_msg;
                button_msg = (struct button_msg_t *)param->param;
                co_printf("KEY 0x%08x, TYPE %d.\r\n", button_msg->button_index, button_msg->button_type);
            }
            break;
    }

    return EVT_CONSUMED;
}

void user_task_init(void)
{
    user_task_id = os_task_create(user_task_func);
}


void user_report_keys(unsigned int gpio,int type,int count){
    os_event_t button_event;
    struct button_msg_t msg;

    msg.button_index = gpio;
    msg.button_type = (uint8_t)type;
    msg.button_cnt = (uint8_t)count;

    button_event.event_id = USER_EVT_BUTTON;
    button_event.src_task_id = user_task_id;
    button_event.param = (void *)&msg;
    button_event.param_len = sizeof(msg);
    os_msg_post(user_task_id, &button_event);
}

