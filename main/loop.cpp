/**
 * @file loop.cpp
 * @brief Contains the main loop function for the FreeRTOS task.
 */

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/**
 * @brief Main loop function for FreeRTOS.
 */
void loop (void) {
	vTaskDelay (pdMS_TO_TICKS (1000));
}
