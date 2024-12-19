#ifndef WIFI_CONNECT_H
#define WIFI_CONNECT_H

#include "esp_wifi.h"

#ifdef __cplusplus
extern "C" {
#endif
void wifi_init_sta (wifi_auth_mode_t mode, const char* ssid, const char* password);
#ifdef __cplusplus
}
#endif

#endif /* WIFI_CONNECT_H */
