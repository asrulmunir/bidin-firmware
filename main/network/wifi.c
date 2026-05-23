/*
 * WiFi Driver Stub Implementation
 * TODO: Implement proper WiFi station mode with credential storage
 */

#include "wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"

static const char *TAG = "wifi";
static bool g_connected = false;

void wifi_init(void)
{
    ESP_LOGI(TAG, "WiFi initialized (stub)");
}

void wifi_connect(void)
{
    ESP_LOGI(TAG, "Connecting to WiFi (stub) - will auto-connect in 2 seconds...");
    
    // Stub: simulate connection after 2 seconds
    g_connected = true;
    ESP_LOGI(TAG, "WiFi connected (stub)");
}

void wifi_disconnect(void)
{
    g_connected = false;
    ESP_LOGI(TAG, "WiFi disconnected (stub)");
}

bool wifi_is_connected(void)
{
    return g_connected;
}

void wifi_save_credentials(const char *ssid, const char *password)
{
    if (ssid == NULL) return;
    
    ESP_LOGI(TAG, "Saving WiFi credentials (stub): SSID=%s", ssid);
    // TODO: Save to NVS
}
