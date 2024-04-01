#include <stdlib.h>
#include <stdio.h>

#include "actor.h"

void counting_func(Msg * p_msg)
{
    int * p_counter = (int*)&p_msg->p_sender->p_data;
    *p_counter = *p_counter + 1;

    if(*p_counter > 10000000)
    {
        p_msg->p_system->running = false;
        /* needed otherwise whole program gets optimized away */
        printf("done %i\n", *p_counter);
    }

    Msg counting_msg = {
        .used = true,
        .a_data = {0},
        .dataType = DT_INT,
        .dataFlags = 0,
        .p_sender = p_msg->p_receiver,
        .p_receiver = p_msg->p_receiver,
        .p_system = p_msg->p_system
        };
    
    /* add msg */
    {
        int index = GetNewMessageQueueIndex(p_msg->p_system);
        p_msg->p_system->p_msg_queue[index] = counting_msg;
    }
}

void main()
{
    System system = {
        .running = true,
        .queue_size = 0,
        .p_msg_queue = 0,
        .index_last_free = -1,
        .msg_index = 0,
        .last_msg = 0
    };

    Actor counting_actor =
        {.p_data=0, .fp_MsgHandler = &counting_func};

    Msg counting_msg = {
        .used = true,
        .a_data = {0},
        .dataType = DT_INT,
        .dataFlags = 0,
        .p_sender = &counting_actor,
        .p_receiver = &counting_actor,
        .p_system = &system,
        };
      
    /* add msg */
    {
        int index = GetNewMessageQueueIndex(&system);
        system.p_msg_queue[index] = counting_msg;
    }

    while(system.running)
    {
        Msg * msg = &system.p_msg_queue[system.msg_index];
        int msg_index = system.msg_index;

        if(!msg->used)
        {
            /* might be messages further down the queue*/
            system.msg_index = (system.msg_index+1) % system.queue_size;
            continue;
        }
        
        msg->p_receiver->fp_MsgHandler(msg);        
        
        system.msg_index = msg->next_msg;

        msg->used = false;
        system.index_last_free = msg_index;
    }
}
