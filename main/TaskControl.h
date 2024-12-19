#ifndef TASKCONTROL_H
#define TASKCONTROL_H

#include "HassMqtt.h"
#include "Status.h"

struct TaskControlContainer {
    Status* status = nullptr;
	HassMqtt::Discovery* hass = nullptr;

    TaskControlContainer (Status* s, HassMqtt::Discovery* h) : status(s), hass(h) {}
    ~TaskControlContainer (void) {}
};

void TaskControl (void* pvParameters);

#endif /* TASKCONTROL_H */
