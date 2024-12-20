#ifndef TASKCONTROL_H
#define TASKCONTROL_H

#include "HassMqtt.h"
#include "Status.h"

/**
 * @brief Container for TaskControl parameters.
 */
struct TaskControlContainer {
	Status* status = nullptr;
	HassMqtt::Discovery* hass = nullptr;

	/**
	 * @brief Constructor for TaskControlContainer.
	 *
	 * @param s Pointer to Status object.
	 * @param h Pointer to HassMqtt::Discovery object.
	 */
	TaskControlContainer (Status* s, HassMqtt::Discovery* h) : status(s), hass(h) {}
	~TaskControlContainer (void) {}
};

/**
 * @brief Task to control the system.
 *
 * @param pvParameters Pointer to task parameters.
 */
void TaskControl (void* pvParameters);

#endif /* TASKCONTROL_H */
