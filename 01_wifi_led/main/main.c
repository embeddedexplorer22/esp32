#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include <esp_http_server.h>
#include "driver/gpio.h"

#define WIFI_SSID		CONFIG_ESP_WIFI_SSID
#define WIFI_PASSWORD	CONFIG_ESP_WIFI_PASSWORD
#define WIFI_CHANNEL	CONFIG_ESP_WIFI_CHANNEL
#define WIFI_MAX_STA	CONFIG_ESP_MAX_STA_CONN

#define LED_PIN			GPIO_NUM_32

static const char *TAG = "MyApp";
static uint8_t led_state = 0; // off

esp_err_t get_handler(httpd_req_t *req)
{
    const char on_resp[] = "<h3>LED State: ON</h3><a href=\"/on\"><button>Turn ON</button></a><a href=\"/off\"><button>Turn OFF</button</a>";
    const char off_resp[] = "<h3>LED State: OFF</h3><a href=\"/on\"><button>Turn ON</button></a><a href=\"/off\"><button>Turn OFF</button</a>";
    
	if (led_state == 0)
		httpd_resp_send(req, off_resp, HTTPD_RESP_USE_STRLEN);
	else
		httpd_resp_send(req, on_resp, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

esp_err_t on_handler(httpd_req_t *req)
{
    gpio_set_level(LED_PIN, 1);
	led_state = 1;
    const char resp[] = "<h3>LED State: ON</h3><a href=\"/on\"><button>Turn ON</button></a><a href=\"/off\"><button>Turn OFF</button</a>";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t off_handler(httpd_req_t *req)
{
    gpio_set_level(LED_PIN, 0);
	led_state = 0;
    const char resp[] = "<h3>LED State: OFF</h3><a href=\"/on\"><button>Turn ON</button></a><a href=\"/off\"><button>Turn OFF</button</a>";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

httpd_uri_t uri_get = {
    .uri      = "/",
    .method   = HTTP_GET,
    .handler  = get_handler,
    .user_ctx = NULL
};

httpd_uri_t uri_on = {
    .uri      = "/on",
    .method   = HTTP_GET,
    .handler  = on_handler,
    .user_ctx = NULL
};

httpd_uri_t uri_off = {
    .uri      = "/off",
    .method   = HTTP_GET,
    .handler  = off_handler,
    .user_ctx = NULL
};


httpd_handle_t setup_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &uri_get);
		httpd_register_uri_handler(server, &uri_on);
		httpd_register_uri_handler(server, &uri_off);
    }

    return server;
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        ESP_LOGI(TAG, "New station joined");
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        ESP_LOGI(TAG, "A station left");
    }
}

static void setup_wifi()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
   
	ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t * p_netif = esp_netif_create_default_wifi_ap();

	esp_netif_ip_info_t ipInfo;
	IP4_ADDR(&ipInfo.ip, 192,168,1,1);
	IP4_ADDR(&ipInfo.gw, 192,168,1,1);
	IP4_ADDR(&ipInfo.netmask, 255,255,255,0);
	esp_netif_dhcps_stop(p_netif);
	esp_netif_set_ip_info(p_netif, &ipInfo);
	esp_netif_dhcps_start(p_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_SSID,
            .ssid_len = strlen(WIFI_SSID),
            .channel = WIFI_CHANNEL,
            .password = WIFI_PASSWORD,
            .max_connection = WIFI_MAX_STA,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi init done.");

	esp_netif_ip_info_t if_info;
	ESP_ERROR_CHECK(esp_netif_get_ip_info(p_netif, &if_info));
	ESP_LOGI(TAG, "ESP32 IP:" IPSTR, IP2STR(&if_info.ip));
}

void app_main(void)
{
	gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
	gpio_set_level(LED_PIN, 0);
	led_state = 0;

	setup_wifi();
	setup_server();
}
