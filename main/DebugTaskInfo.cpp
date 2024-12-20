#include "DebugTaskInfo.h"

#include <cstdio>

#include <esp_heap_caps.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/**
 * @brief Task to print debug information about the heap and tasks.
 *
 * @param pvParameters Pointer to task parameters.
 */
void DebugTaskInfo (void*) {
	vTaskDelay (pdMS_TO_TICKS (1000));

	while (true) {
		printf ("Free heap: %ld bytes\n", esp_get_free_heap_size ());
		printf ("Free internal heap: %ld bytes\n", esp_get_free_internal_heap_size ());

		vTaskDelay (pdMS_TO_TICKS (10));

		volatile UBaseType_t status_array_size = uxTaskGetNumberOfTasks ();

		TaskStatus_t* status_array = static_cast<TaskStatus_t*>(heap_caps_malloc (status_array_size * sizeof(TaskStatus_t), MALLOC_CAP_SPIRAM));

		vTaskDelay (pdMS_TO_TICKS (10));

		printf ("#  | id | name             | state | core | priority (base) | Stack high water\n");
		UBaseType_t ret = uxTaskGetSystemState (status_array, status_array_size, nullptr);

		for (int i = 0; i < ret; ++i) {
			char state;
			switch (status_array[i].eCurrentState) {
				case eRunning:
					state = 'R';
					break;
				case eReady:
					state = 'r';
					break;
				case eBlocked:
					state = 'B';
					break;
				case eSuspended:
					state = 'S';
					break;
				case eDeleted:
					state = 'D';
					break;
				case eInvalid:
					state = 'I';
					break;
				default:
					state = 'U';
					break;
			}

			printf ("%2d | %2d | %16s |   %c   | %2d   | %2d (%2d)         | %6ld\n", i, status_array[i].xTaskNumber, status_array[i].pcTaskName, state, (status_array[i].xCoreID == tskNO_AFFINITY ? -1 : status_array[i].xCoreID), status_array[i].uxCurrentPriority, status_array[i].uxBasePriority, status_array[i].usStackHighWaterMark);

			vTaskDelay (pdMS_TO_TICKS (10));
		}

		heap_caps_free (status_array);

		vTaskDelay (pdMS_TO_TICKS (10000));
	}
}
