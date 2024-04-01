#include <stdbool.h>
#include <string.h>

#define SYSTEM_MSG_QUEUE_SIZE 4096

typedef enum
{
    DT_CHAR,
    DT_STRING,
    DT_SHORT,
    DT_INT,
    DT_LONG,
    DT_FLOAT,
    DT_DOUBLE,
    DT_LONG_DOUBLE
} DataType;

typedef enum
{
    DF_SIGNED,
    DF_POINTER,
    DF_ARRAY
} DataFlags;

typedef struct Actor Actor;
typedef struct System System;
typedef struct Msg Msg;

struct Actor{
    void * p_data;

    void (*fp_MsgHandler)(Msg * msg);
};

struct Msg{
    bool used;
    unsigned char a_data [10];
    DataType dataType;
    unsigned char dataFlags;
    System * p_system;
    Actor * p_receiver;
    Actor * p_sender;

    int next_msg;
};

typedef struct System{
    bool running;
    Msg * p_msg_queue;
    int queue_size;
    int msg_index;
    int index_last_free;

    int last_msg;
} System;

void AllocateSystemQueue(System * p_system)
{
    /* increase queue size*/
    if(p_system->queue_size < 0)
        p_system->queue_size = 0;
    
    p_system->queue_size += SYSTEM_MSG_QUEUE_SIZE;

    if(!p_system->p_msg_queue)
        p_system->p_msg_queue = calloc(p_system->queue_size, sizeof(Msg));
    else
    {
        p_system->p_msg_queue = realloc(p_system->p_msg_queue, p_system->queue_size * sizeof(Msg));
        memset(p_system->p_msg_queue-sizeof(Msg)*SYSTEM_MSG_QUEUE_SIZE, 0, SYSTEM_MSG_QUEUE_SIZE*sizeof(Msg));
    }
}

int GetNewMessageQueueIndex(System * p_system)
{
    if(p_system->queue_size <= 0)
        AllocateSystemQueue(p_system);
    
    /* find empty spot */
    int spot = -1;

    /* check last used message is free */
    if(p_system->index_last_free > 0)
        spot = p_system->index_last_free;
    p_system->index_last_free = -1;


    /* check last message added + 1 */
    {
        int last_msg = (p_system->last_msg+1) % p_system->queue_size;

        if(!p_system->p_msg_queue[last_msg].used)
        {
            spot = last_msg;
        }
    }

    /* couldn't find spot */
    if(spot == -1)
    {
        AllocateSystemQueue(p_system);
        spot = p_system->queue_size - SYSTEM_MSG_QUEUE_SIZE + 1;
    }

    p_system->p_msg_queue[p_system->last_msg].next_msg = spot;
    p_system->last_msg = spot;
    p_system->p_msg_queue[spot].used = true;

    return spot;
}