#ifndef HTTPSERVER_WORKERDATA_H
#define HTTPSERVER_WORKERDATA_H

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

namespace HttpServer {
	struct WorkerData {
		QueueHandle_t queue;
		const size_t num_workers;
		const TickType_t worker_wait_ticks;
	};
}

#endif /* HTTPSERVER_WORKERDATA_H */
