#include "../include/http.hpp"
#include "../include/websites.hpp"
#include "../include/mqtt.hpp"
#include "esp_http_server.h"
#include "esp_log.h"

static const char *TAG_HTTP = "HTTP";
static httpd_handle_t webServer;

// External functions.
void HTTP_startWebServer();

// Helper functions.
/**
 * @brief
 * @param
 * @return
 */
static esp_err_t getHandler(httpd_req_t *req);

/**
 * @brief
 * @param
 * @return
 */
static esp_err_t postHandler(httpd_req_t *req);

// Function definitions.
void HTTP_startWebServer()
{
    ESP_LOGI(TAG_HTTP, "Starting webserver");

    httpd_uri_t configWebsiteGet = {
        .uri = mqttURI,
        .method = HTTP_GET,
        .handler = getHandler,
        .user_ctx = NULL};

    httpd_uri_t configWebsitePost = {
        .uri = mqttURI,
        .method = HTTP_POST,
        .handler = postHandler,
        .user_ctx = NULL};

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    webServer = NULL;
    ESP_ERROR_CHECK(httpd_start(&webServer, &config));
    ESP_ERROR_CHECK(httpd_register_uri_handler(webServer, &configWebsiteGet));
    ESP_ERROR_CHECK(httpd_register_uri_handler(webServer, &configWebsitePost));
}

static esp_err_t getHandler(httpd_req_t *req)
{
    static char buf[2048];

    if (strcmp(req->uri, mqttURI) == 0)
    {
        ESP_LOGI(TAG_HTTP, "Received GET on mqtt's uri");

        MQTT_resourceTake();
        const char *brokerIp = MQTT_getIP();
        const char *brokerPort = MQTT_getPort();
        const char *user = MQTT_getUser();
        const char *passwd = MQTT_getPassword();
        const char *ns = MQTT_getNamespace();
        snprintf(buf, 4096, mqttWebsite, brokerIp, brokerPort, user, passwd, ns);
        MQTT_resourceRelease();

        ESP_ERROR_CHECK(httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN));
    }

    return ESP_OK;
}

static esp_err_t postHandler(httpd_req_t *req)
{
    static char content[256];

    memset(content, 0, sizeof(content));

    if (strcmp(req->uri, mqttURI) == 0)
    {
        ESP_LOGI(TAG_HTTP, "Received POST on mqtt's uri");

        // Truncate recv size.
        size_t recv_size = req->content_len;
        if (recv_size < sizeof(content))
            recv_size = sizeof(content);

        int ret = httpd_req_recv(req, content, recv_size);

        // Handle closed connection.
        if (ret <= 0)
        {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT)
                ESP_ERROR_CHECK(httpd_resp_send_408(req));

            return ESP_FAIL;
        }

        // Get key value pairs of MQTT's config.
        const int numKeyValuePairs = 5;
        const char *keyValuePair[numKeyValuePairs];

        keyValuePair[0] = strtok(content, "&"); // First key value pair of MQTT's config.

        // Next key value pairs for every received param of MQTT's config.
        bool finished = false;
        int cntr = numKeyValuePairs - 1;
        while (cntr && !finished)
        {
            keyValuePair[numKeyValuePairs - cntr] = strtok(nullptr, "&");
            if (keyValuePair[numKeyValuePairs - cntr] == NULL)
                finished = true;
            cntr--;
        }

        // Get key and value of each pair.
        // And save each valid key value pair to MQTT.
        int keyValuePairsReceived = numKeyValuePairs - cntr;
        for (int i = 0; i < keyValuePairsReceived; i++)
        {
            const char *key = strtok((char *)keyValuePair[i], "=");
            const char *val = strtok(nullptr, "=");

            if (strcmp(key, "brokerip") == 0 && val != NULL)
            {
                MQTT_updateIP(val);
            }
            else if (strcmp(key, "brokerport") == 0 && val != NULL)
            {
                MQTT_updatePort(val);
            }
            else if (strcmp(key, "user") == 0)
            {
                if (val == NULL)
                    val = "";

                MQTT_updateUser(val);
            }
            else if (strcmp(key, "password") == 0)
            {
                if (val == NULL)
                    val = "";

                MQTT_updatePassword(val);
            }
            else if (strcmp(key, "namespace") == 0)
            {
                if (val == NULL)
                    val = "";

                MQTT_updateNamespace(val);
            }
        }
        MQTT_reInit();

        // Reply with same website but updated data.
        getHandler(req);
    }

    return ESP_OK;
}
