#include "camera.h"
#include "distance.h"
#include "http_client.h"
#include "motor.h"
#include "ui_api_html.h"
#include "ui_home_html.h"
#include <Arduino.h>
#include <esp_http_server.h>

bool isStreaming = false;

typedef struct {
  httpd_req_t *req;
  size_t len;
} jpg_chunking_t;

#define PART_BOUNDARY "123456789000000000000987654321"
static const char *_STREAM_CONTENT_TYPE =
    "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: "
                                  "%u\r\nX-Timestamp: %d.%06d\r\n\r\n";

httpd_handle_t stream_httpd = NULL;
httpd_handle_t command_httpd = NULL;

typedef struct {
  size_t size;  // number of values used for filtering
  size_t index; // current value index
  size_t count; // value count
  int sum;
  int *values; // array to be filled with values
} ra_filter_t;

static ra_filter_t ra_filter;

static ra_filter_t *ra_filter_init(ra_filter_t *filter, size_t sample_size) {
  memset(filter, 0, sizeof(ra_filter_t));

  filter->values = (int *)malloc(sample_size * sizeof(int));
  if (!filter->values) {
    return NULL;
  }
  memset(filter->values, 0, sample_size * sizeof(int));

  filter->size = sample_size;
  return filter;
}

static esp_err_t capture_handler(httpd_req_t *req) {
  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;

  // if(!CAM_isInitialized()){
  //   return ESP_FAIL;
  // }

  fb = esp_camera_fb_get();
  if (!fb) {
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  httpd_resp_set_type(req, "image/jpeg");
  httpd_resp_set_hdr(req, "Content-Disposition",
                     "inline; filename=capture.jpg");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

  char ts[32];
  snprintf(ts, 32, "%lld.%06ld", fb->timestamp.tv_sec, fb->timestamp.tv_usec);
  httpd_resp_set_hdr(req, "X-Timestamp", (const char *)ts);

  res = httpd_resp_send(req, (const char *)fb->buf, fb->len);

  esp_camera_fb_return(fb);
  return res;
}

static esp_err_t stream_handler(httpd_req_t *req) {
  camera_fb_t *fb = NULL;
  struct timeval _timestamp;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t *_jpg_buf = NULL;
  char *part_buf[128];

  if (!CAM_isInitialized()) {
    return ESP_FAIL;
  }

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if (res != ESP_OK) {
    return res;
  }

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_set_hdr(req, "X-Framerate", "60");

  while (true) {
    fb = CAM_Capture();
    if (!fb) {
      res = ESP_FAIL;
    } else {
      _timestamp.tv_sec = fb->timestamp.tv_sec;
      _timestamp.tv_usec = fb->timestamp.tv_usec;
      if (fb->format != PIXFORMAT_JPEG) {
        bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
        esp_camera_fb_return(fb);
        fb = NULL;
        if (!jpeg_converted) {
          res = ESP_FAIL;
        }
      } else {
        _jpg_buf_len = fb->len;
        _jpg_buf = fb->buf;
      }
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY,
                                  strlen(_STREAM_BOUNDARY));
    }
    if (res == ESP_OK) {
      size_t hlen = snprintf((char *)part_buf, 128, _STREAM_PART, _jpg_buf_len,
                             _timestamp.tv_sec, _timestamp.tv_usec);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if (fb) {
      CAM_Dispose(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if (_jpg_buf) {
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if (res != ESP_OK) {
      break;
    }
  }

  return res;
}

static esp_err_t parse_get(httpd_req_t *req, char **obuf) {
  char *buf = NULL;
  size_t buf_len = 0;

  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (buf_len > 1) {
    buf = (char *)malloc(buf_len);
    if (!buf) {
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
      *obuf = buf;
      return ESP_OK;
    }
    free(buf);
  }
  httpd_resp_send_404(req);
  return ESP_FAIL;
}

static esp_err_t config_handler(httpd_req_t *req) {
  char *buf = NULL;
  char variable[32];
  char value[32];

  if (parse_get(req, &buf) != ESP_OK) {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                        "ERR: Missing query parameters");
    return ESP_FAIL;
  }
  if (httpd_query_key_value(buf, "var", variable, sizeof(variable)) != ESP_OK ||
      httpd_query_key_value(buf, "val", value, sizeof(value)) != ESP_OK) {
    free(buf);
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                        "ERR: Missing query parameters");
    return ESP_FAIL;
  }
  free(buf);

  int val = atoi(value);
  sensor_t *s = esp_camera_sensor_get();
  int response = 0;

  if (!strcmp(variable, "framesize")) {
    if (s->pixformat == PIXFORMAT_JPEG) {
      response = s->set_framesize(s, (framesize_t)val);
    }
  } else if (!strcmp(variable, "quality")) {
    response = s->set_quality(s, val);
  } else if (!strcmp(variable, "contrast")) {
    response = s->set_contrast(s, val);
  } else if (!strcmp(variable, "brightness")) {
    response = s->set_brightness(s, val);
  } else if (!strcmp(variable, "saturation")) {
    response = s->set_saturation(s, val);
  } else if (!strcmp(variable, "gainceiling")) {
    response = s->set_gainceiling(s, (gainceiling_t)val);
  } else if (!strcmp(variable, "colorbar")) {
    response = s->set_colorbar(s, val);
  } else if (!strcmp(variable, "awb")) {
    response = s->set_whitebal(s, val);
  } else if (!strcmp(variable, "agc")) {
    response = s->set_gain_ctrl(s, val);
  } else if (!strcmp(variable, "aec")) {
    response = s->set_exposure_ctrl(s, val);
  } else if (!strcmp(variable, "hmirror")) {
    response = s->set_hmirror(s, val);
  } else if (!strcmp(variable, "vflip")) {
    response = s->set_vflip(s, val);
  } else if (!strcmp(variable, "awb_gain")) {
    response = s->set_awb_gain(s, val);
  } else if (!strcmp(variable, "agc_gain")) {
    response = s->set_agc_gain(s, val);
  } else if (!strcmp(variable, "aec_value")) {
    response = s->set_aec_value(s, val);
  } else if (!strcmp(variable, "aec2")) {
    response = s->set_aec2(s, val);
  } else if (!strcmp(variable, "dcw")) {
    response = s->set_dcw(s, val);
  } else if (!strcmp(variable, "bpc")) {
    response = s->set_bpc(s, val);
  } else if (!strcmp(variable, "wpc")) {
    response = s->set_wpc(s, val);
  } else if (!strcmp(variable, "raw_gma")) {
    response = s->set_raw_gma(s, val);
  } else if (!strcmp(variable, "lenc")) {
    response = s->set_lenc(s, val);
  } else if (!strcmp(variable, "special_effect")) {
    response = s->set_special_effect(s, val);
  } else if (!strcmp(variable, "wb_mode")) {
    response = s->set_wb_mode(s, val);
  } else if (!strcmp(variable, "ae_level")) {
    response = s->set_ae_level(s, val);
  } else {
    response = -1;
  }

  if (response < 0) {
    return httpd_resp_send_500(req);
  }

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, NULL, 0);
}

static esp_err_t process_motor_command(const char *action, char *response_msg) {
  if (!strcmp(action, "distance")) {
    int32_t distance = DISTANCE_get();
    sprintf(response_msg, "%ld", distance);
  } else if (!strcmp(action, "grab")) {
    MOTOR_grabBall();
    strcpy(response_msg, "OK");
  } else if (!strcmp(action, "release")) {
    MOTOR_setClawServo(SERVO_CLAW_MAX);
    strcpy(response_msg, "OK");
  } else if (strcmp(action, "fwd") == 0) {
    MOTOR_moveToDistance(1000, 200, true);
    strcpy(response_msg, "OK");
  } else if (strcmp(action, "rev") == 0) {
    MOTOR_moveToDistance(1000, -200, true);
    strcpy(response_msg, "OK");
  } else if (strcmp(action, "left") == 0) {
    MOTOR_incrementSteerServo(-180);
    strcpy(response_msg, "OK");
  } else if (strcmp(action, "right") == 0) {
    MOTOR_incrementSteerServo(180);
    strcpy(response_msg, "OK");
  } else if (strcmp(action, "stop") == 0) {
    MOTOR_stopAll();
    strcpy(response_msg, "OK");
  } else {
    return ESP_FAIL;
  }
  return ESP_OK;
}

static esp_err_t command_handler(httpd_req_t *req) {
  char *buf = NULL;
  char action[32] = {0};
  char response_msg[64] = {0};

  if (HTTPC_fwUpdateInProgress()) {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "OTA in progress");
    return ESP_FAIL;
  }

  if (parse_get(req, &buf) != ESP_OK) {
    return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                               "ERR: Missing query parameters");
  }

  if (httpd_query_key_value(buf, "action", action, sizeof(action)) != ESP_OK) {
    free(buf);
    return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                               "ERR: Missing action parameter");
  }
  free(buf);

  httpd_resp_set_type(req, "text/plain");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

  if (process_motor_command(action, response_msg) != ESP_OK) {
    return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                               "ERR: Unsupported command");
  }

  return httpd_resp_send(req, response_msg, strlen(response_msg));
}

static esp_err_t ws_handler(httpd_req_t *req) {
  if (req->method == HTTP_GET) {
    return ESP_OK;
  }

  httpd_ws_frame_t ws_pkt;
  uint8_t *buf = NULL;
  memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
  ws_pkt.type = HTTPD_WS_TYPE_TEXT;

  esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
  if (ret != ESP_OK) {
    return ret;
  }

  if (ws_pkt.len) {
    buf = (uint8_t *)calloc(1, ws_pkt.len + 1);
    if (buf == NULL) {
      return ESP_ERR_NO_MEM;
    }
    ws_pkt.payload = buf;
    ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
    if (ret != ESP_OK) {
      free(buf);
      return ret;
    }

    char response_msg[64] = {0};
    if (process_motor_command((char *)ws_pkt.payload, response_msg) == ESP_OK) {
      httpd_ws_frame_t ws_resp;
      memset(&ws_resp, 0, sizeof(httpd_ws_frame_t));
      ws_resp.payload = (uint8_t *)response_msg;
      ws_resp.len = strlen(response_msg);
      ws_resp.type = HTTPD_WS_TYPE_TEXT;
      httpd_ws_send_frame(req, &ws_resp);
    }
    free(buf);
  }
  return ESP_OK;
}

static int print_reg(char *p, sensor_t *s, uint16_t reg, uint32_t mask) {
  return sprintf(p, "\"0x%x\":%u,", reg, s->get_reg(s, reg, mask));
}

static esp_err_t status_handler(httpd_req_t *req) {
  static char json_response[1024];

  sensor_t *s = esp_camera_sensor_get();
  char *p = json_response;
  *p++ = '{';

  if (s->id.PID == OV5640_PID || s->id.PID == OV3660_PID) {
    for (int reg = 0x3400; reg < 0x3406; reg += 2) {
      p += print_reg(p, s, reg, 0xFFF); // 12 bit
    }
    p += print_reg(p, s, 0x3406, 0xFF);

    p += print_reg(p, s, 0x3500, 0xFFFF0); // 16 bit
    p += print_reg(p, s, 0x3503, 0xFF);
    p += print_reg(p, s, 0x350a, 0x3FF);  // 10 bit
    p += print_reg(p, s, 0x350c, 0xFFFF); // 16 bit

    for (int reg = 0x5480; reg <= 0x5490; reg++) {
      p += print_reg(p, s, reg, 0xFF);
    }

    for (int reg = 0x5380; reg <= 0x538b; reg++) {
      p += print_reg(p, s, reg, 0xFF);
    }

    for (int reg = 0x5580; reg < 0x558a; reg++) {
      p += print_reg(p, s, reg, 0xFF);
    }
    p += print_reg(p, s, 0x558a, 0x1FF); // 9 bit
  } else if (s->id.PID == OV2640_PID) {
    p += print_reg(p, s, 0xd3, 0xFF);
    p += print_reg(p, s, 0x111, 0xFF);
    p += print_reg(p, s, 0x132, 0xFF);
  }

  p += sprintf(p, "\"xclk\":%u,", s->xclk_freq_hz / 1000000);
  p += sprintf(p, "\"pixformat\":%u,", s->pixformat);
  p += sprintf(p, "\"framesize\":%u,", s->status.framesize);
  p += sprintf(p, "\"quality\":%u,", s->status.quality);
  p += sprintf(p, "\"brightness\":%d,", s->status.brightness);
  p += sprintf(p, "\"contrast\":%d,", s->status.contrast);
  p += sprintf(p, "\"saturation\":%d,", s->status.saturation);
  p += sprintf(p, "\"sharpness\":%d,", s->status.sharpness);
  p += sprintf(p, "\"special_effect\":%u,", s->status.special_effect);
  p += sprintf(p, "\"wb_mode\":%u,", s->status.wb_mode);
  p += sprintf(p, "\"awb\":%u,", s->status.awb);
  p += sprintf(p, "\"awb_gain\":%u,", s->status.awb_gain);
  p += sprintf(p, "\"aec\":%u,", s->status.aec);
  p += sprintf(p, "\"aec2\":%u,", s->status.aec2);
  p += sprintf(p, "\"ae_level\":%d,", s->status.ae_level);
  p += sprintf(p, "\"aec_value\":%u,", s->status.aec_value);
  p += sprintf(p, "\"agc\":%u,", s->status.agc);
  p += sprintf(p, "\"agc_gain\":%u,", s->status.agc_gain);
  p += sprintf(p, "\"gainceiling\":%u,", s->status.gainceiling);
  p += sprintf(p, "\"bpc\":%u,", s->status.bpc);
  p += sprintf(p, "\"wpc\":%u,", s->status.wpc);
  p += sprintf(p, "\"raw_gma\":%u,", s->status.raw_gma);
  p += sprintf(p, "\"lenc\":%u,", s->status.lenc);
  p += sprintf(p, "\"hmirror\":%u,", s->status.hmirror);
  p += sprintf(p, "\"vflip\":%u,", s->status.vflip);
  p += sprintf(p, "\"dcw\":%u,", s->status.dcw);
  p += sprintf(p, "\"colorbar\":%u", s->status.colorbar);
  *p++ = '}';
  *p++ = 0;
  httpd_resp_set_type(req, "application/json");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, json_response, strlen(json_response));
}

static int parse_get_var(char *buf, const char *key, int def) {
  char _int[16];
  if (httpd_query_key_value(buf, key, _int, sizeof(_int)) != ESP_OK) {
    return def;
  }
  return atoi(_int);
}

static esp_err_t index_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, index_html, strlen_P(index_html));
}

static esp_err_t fw_update_handler(httpd_req_t *req) {
  if (req->method != HTTP_POST) {
    httpd_resp_send_404(req);
    return ESP_FAIL;
  }

  /* Reject if OTA already running */
  if (HTTPC_fwUpdateInProgress()) {
    return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                               "Firmware update already in progress");
  }

  /* Read firmware URL from POST body */
  char firmwareUrl[512] = {0};
  int ret = httpd_req_recv(req, firmwareUrl, sizeof(firmwareUrl) - 1);
  if (ret <= 0) {
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                        "Failed to read firmware download URL");
    return ESP_FAIL;
  }
  firmwareUrl[ret] = '\0';

  /* Optional: get log callback URL from header */
  char logUrl[255] = {0};
  if (httpd_req_get_hdr_value_len(req, "X-Log-Callback") > 0) {
    httpd_req_get_hdr_value_str(req, "X-Log-Callback", logUrl, sizeof(logUrl));
  }

  /* Start OTA asynchronously */
  HTTPC_fwUpdateRequest(firmwareUrl, logUrl[0] != '\0' ? logUrl : nullptr);

  httpd_resp_send(req, "Firmware update started", HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

void HTTPSRV_stop() {
  if (stream_httpd)
    httpd_stop(stream_httpd);
  if (command_httpd)
    httpd_stop(command_httpd);
}

void HTTPSRV_init() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.max_uri_handlers = 16;

  httpd_uri_t index_uri = {.uri = "/",
                           .method = HTTP_GET,
                           .handler = index_handler,
                           .user_ctx = NULL};

  httpd_uri_t status_uri = {.uri = "/status",
                            .method = HTTP_GET,
                            .handler = status_handler,
                            .user_ctx = NULL};

  httpd_uri_t cfg_uri = {.uri = "/config",
                         .method = HTTP_GET,
                         .handler = config_handler,
                         .user_ctx = NULL};

  httpd_uri_t cmd_uri = {.uri = "/command",
                         .method = HTTP_GET,
                         .handler = command_handler,
                         .user_ctx = NULL};

  httpd_uri_t capture_uri = {.uri = "/capture",
                             .method = HTTP_GET,
                             .handler = capture_handler,
                             .user_ctx = NULL};

  httpd_uri_t stream_uri = {.uri = "/stream",
                            .method = HTTP_GET,
                            .handler = stream_handler,
                            .user_ctx = NULL};

  httpd_uri_t fw_update_uri = {.uri = "/api/ota",
                               .method = HTTP_POST,
                               .handler = fw_update_handler,
                               .user_ctx = NULL};

  httpd_uri_t ws_uri = {.uri = "/ws",
                        .method = HTTP_GET,
                        .handler = ws_handler,
                        .user_ctx = NULL,
                        .is_websocket = true};

  ra_filter_init(&ra_filter, 20);

  if (httpd_start(&command_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(command_httpd, &index_uri);
    httpd_register_uri_handler(command_httpd, &cfg_uri);
    httpd_register_uri_handler(command_httpd, &cmd_uri);
    httpd_register_uri_handler(command_httpd, &status_uri);
    httpd_register_uri_handler(command_httpd, &capture_uri);
    httpd_register_uri_handler(command_httpd, &fw_update_uri);
    httpd_register_uri_handler(command_httpd, &ws_uri);
  }

  config.server_port += 1;
  config.ctrl_port += 1;
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
  }
}
