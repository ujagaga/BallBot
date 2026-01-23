// http_server.cpp
#include <Arduino.h>
#include <ArduinoJson.h>
#include <esp_http_server.h>
#include "camera.h"
#include "ui.h"
#include "http_server.h"
#include "http_client.h"
#include "logger.h"
#include "motor.h"
#include "distance.h"

typedef struct {
  httpd_req_t *req;
  size_t len;
} jpg_chunking_t;

#define PART_BOUNDARY "123456789000000000000987654321"
static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\nX-Timestamp: %d.%06d\r\n\r\n";

static httpd_handle_t stream_httpd = NULL;
static httpd_handle_t cmd_httpd = NULL;

static esp_err_t stream_handler(httpd_req_t *req) {
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    char part_buf[128];

    httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    while (true) {
        fb = esp_camera_fb_get();
        if (!fb) {
            vTaskDelay(10 / portTICK_PERIOD_MS);
            continue;
        }

        // Send Boundary
        res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        
        // Send simplified header (No timestamp)
        if (res == ESP_OK) {
            char h_buf[64];
            int h_len = sprintf(h_buf, "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", fb->len);
            res = httpd_resp_send_chunk(req, h_buf, h_len);
        }

        // Send Image
        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
        }

        esp_camera_fb_return(fb);
        fb = NULL;

        if (res != ESP_OK) break;
        
        // Important: A slightly longer delay helps the Wi-Fi stack process the Port 80 ACK
        vTaskDelay(20 / portTICK_PERIOD_MS); 
    }
    return res;
}

// ---------------- Snapshot Handler ----------------
static esp_err_t capture_handler(httpd_req_t *req)
{
    camera_fb_t *fb = CAM_Capture();

    if (!fb) {        
        httpd_resp_send_500(req);
        return ESP_FAIL;        
    }

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    esp_err_t res = httpd_resp_send(req, (const char *)fb->buf, fb->len);

    CAM_Dispose(fb);
    return res;
}

// ---------------- OTA API Handler ----------------
static esp_err_t api_ota_handler(httpd_req_t *req)
{
    if (req->method != HTTP_POST) {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    /* Reject if OTA already running */
    if (HTTPC_fwUpdateInProgress()) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "OTA already in progress");
        return ESP_FAIL;
    }

    /* Read firmware URL from POST body */
    char firmwareUrl[512] = {0};
    int ret = httpd_req_recv(req, firmwareUrl, sizeof(firmwareUrl) - 1);
    if (ret <= 0) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read OTA URL");
        return ESP_FAIL;
    }
    firmwareUrl[ret] = '\0';

    /* Optional: get log callback URL from header */
    char logUrl[255] = {0};
    if (httpd_req_get_hdr_value_len(req, "X-Log-Callback") > 0) {
        httpd_req_get_hdr_value_str(req,
                                    "X-Log-Callback",
                                    logUrl,
                                    sizeof(logUrl));
    }

    /* Start OTA asynchronously */
    HTTPC_fwUpdateRequest(
        firmwareUrl,
        logUrl[0] != '\0' ? logUrl : nullptr
    );

    httpd_resp_send(req, "OTA started", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t index_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, index_html, strlen_P(index_html));
}

static esp_err_t api_wiki_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, api_html, strlen_P(api_html));
}

// ---------------- Log Handler ----------------
static esp_err_t api_logs_handler(httpd_req_t *req) {
    static char buf[1400]; // 5 * 256 + overhead
    LOG_get(buf, sizeof(buf));

    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// ---------------- API Handler ----------------
static esp_err_t api_cmd_handler(httpd_req_t *req)
{
    char query[128] = {0};
    char action[32] = {0};    

    /* Extract query string */
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing query");
        return ESP_FAIL;
    }

    /* Extract 'action' parameter */
    if (httpd_query_key_value(query, "action", action, sizeof(action)) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing action");
        return ESP_FAIL;
    }

    /* Optional: block commands during OTA */
    if (HTTPC_fwUpdateInProgress()) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "OTA in progress");
        return ESP_FAIL;
    }

    /* Extract 'timeout' optional parameter */
    char timeoutRequest[32] = {0};
    char* endPtr;    
    uint32_t timeOut = MOTOR_TIMEOUT;
    if (httpd_query_key_value(query, "timeout", timeoutRequest, sizeof(timeoutRequest)) == ESP_OK){        
        uint32_t value = strtoul(timeoutRequest, &endPtr, 10);
        if(*endPtr == '\0'){
            timeOut = value;
        }        
    }
    uint32_t timeLimit = timeOut + millis();
    MOTOR_setTimeout(timeLimit);
    char resultData[256] = {0};
    // Set initial message
    resultData[0] = 'O';
    resultData[1] = 'K';

    /* Dispatch command */
    if (strcmp(action, "grab") == 0) {
        httpd_resp_set_type(req, "text/plain");
        httpd_resp_set_hdr(req, "Connection", "close");
        httpd_resp_send(req, resultData, HTTPD_RESP_USE_STRLEN);
        // MOTOR_grabBall();
        return ESP_OK;
    }
    else if (strcmp(action, "release") == 0) {
        httpd_resp_set_type(req, "text/plain");
        httpd_resp_set_hdr(req, "Connection", "close");
        httpd_resp_send(req, resultData, HTTPD_RESP_USE_STRLEN);
        MOTOR_setClawServo(SERVO_CLAW_MAX);
        return ESP_OK;
    }    
    else if (strcmp(action, "fwd") == 0) {
        httpd_resp_set_type(req, "text/plain");
        httpd_resp_set_hdr(req, "Connection", "close");
        httpd_resp_send(req, resultData, HTTPD_RESP_USE_STRLEN);
        MOTOR_moveToDistance(10, 300, true);
        return ESP_OK;
    }
    else if (strcmp(action, "rev") == 0) {
        httpd_resp_set_type(req, "text/plain");
        httpd_resp_set_hdr(req, "Connection", "close");
        httpd_resp_send(req, resultData, HTTPD_RESP_USE_STRLEN);
        MOTOR_moveToDistance(10, -300, true);
        return ESP_OK;
    }
    else if (strcmp(action, "left") == 0) {
        httpd_resp_set_type(req, "text/plain");
        httpd_resp_set_hdr(req, "Connection", "close");
        httpd_resp_send(req, resultData, HTTPD_RESP_USE_STRLEN);
        MOTOR_incrementSteerServo(-20);
        return ESP_OK;
    }
    else if (strcmp(action, "right") == 0) {
        httpd_resp_set_type(req, "text/plain");
        httpd_resp_set_hdr(req, "Connection", "close");
        httpd_resp_send(req, resultData, HTTPD_RESP_USE_STRLEN);
        MOTOR_incrementSteerServo(20);
        return ESP_OK;
    }
    else if (strcmp(action, "distance") == 0) {
        int32_t distance = DISTANCE_get();
        snprintf(resultData, sizeof(resultData), "%ld", distance);
        httpd_resp_set_type(req, "text/plain");
        httpd_resp_set_hdr(req, "Connection", "close");
        httpd_resp_send(req, resultData, HTTPD_RESP_USE_STRLEN);
    } 
    else if (strcmp(action, "stop") == 0) {
        MOTOR_stopAll();
        return api_logs_handler(req);
    }
    else {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid action");
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "text/plain");
    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_send(req, resultData, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// ---------------- HTTP Server Init ----------------
void HTTP_SERVER_stop(){
    if (cmd_httpd) httpd_stop(cmd_httpd);
    if (stream_httpd) httpd_stop(stream_httpd);
}

void HTTP_SERVER_init()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 16; 
    config.stack_size = 8192;    
    config.server_port = 80;
    config.ctrl_port = 32768;
    config.task_priority = 4;
    // CRITICAL: Reduce the timeout so Port 80 closes the instant the command is sent
    config.recv_wait_timeout = 2; 
    config.send_wait_timeout = 2;
    // Don't let Port 80 hoard sockets
    config.max_open_sockets = 2;

    httpd_uri_t index_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = index_handler,
        .user_ctx = NULL
    };

    httpd_uri_t capture_uri = {
        .uri = "/capture",
        .method = HTTP_GET,
        .handler = capture_handler,
        .user_ctx = NULL
    };

    httpd_uri_t api_ota_uri = {
        .uri = "/api/ota",
        .method = HTTP_POST,
        .handler = api_ota_handler,
        .user_ctx = NULL
    };
    
    httpd_uri_t api_logs = {
        .uri = "/api/logs",
        .method = HTTP_GET,
        .handler = api_logs_handler,
        .user_ctx = NULL
    };

    httpd_uri_t api_cmd = {
        .uri = "/api/cmd",
        .method = HTTP_GET,
        .handler = api_cmd_handler,
        .user_ctx = NULL
    };

    httpd_uri_t api_wiki = {
        .uri = "/api/",
        .method = HTTP_GET,
        .handler = api_wiki_handler,
        .user_ctx = NULL
    };

    if (httpd_start(&cmd_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(cmd_httpd, &index_uri);
        httpd_register_uri_handler(cmd_httpd, &capture_uri);
        httpd_register_uri_handler(cmd_httpd, &api_ota_uri);
        httpd_register_uri_handler(cmd_httpd, &api_logs);
        httpd_register_uri_handler(cmd_httpd, &api_cmd);
        httpd_register_uri_handler(cmd_httpd, &api_wiki);
    }

    httpd_uri_t stream_uri = {
        .uri = "/stream",
        .method = HTTP_GET,
        .handler = stream_handler,
        .user_ctx = NULL
    };

    config.server_port += 1; // Now 81
    config.ctrl_port += 1;   // Now 32769
    config.task_priority = 6; // Set this significantly HIGHER than Port 80
    config.max_open_sockets = 1;     
    // Give the stream more time to survive Wi-Fi congestion
    config.recv_wait_timeout = 30;
    config.send_wait_timeout = 30;

    if (httpd_start(&stream_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(stream_httpd, &stream_uri);
    }
}
