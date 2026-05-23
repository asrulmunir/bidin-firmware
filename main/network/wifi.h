/*
 * WiFi Driver
 * Minimal implementation for station mode
 */

#ifndef WIFI_H
#define WIFI_H

#include <stdbool.h>

// Initialize WiFi
void wifi_init(void);

// Connect to WiFi (uses stored credentials from NVS)
void wifi_connect(void);

// Disconnect from WiFi
void wifi_disconnect(void);

// Check if connected
bool wifi_is_connected(void);

// Get IP address as string
const char* wifi_get_ip_address(void);

// Get RSSI (signal strength)
int wifi_get_rssi(void);

#endif // WIFI_H
