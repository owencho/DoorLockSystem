#ifndef _EVENT_H
#define _EVENT_H

typedef enum{
    DOOR_INIT_EVENT,
    DOOR_OPENED_EVENT,
    DOOR_CLOSED_EVENT,
    VALID_DOOR_ACCESS_EVENT,
    INVALID_DOOR_ACCESS_EVENT,
}EventType;

typedef struct Event Event;

struct Event{
    EventType type;
    void * data;
};

#endif // _EVENT_H
