#include "esp_camera.h"
#include <WiFi.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "soc/soc.h"             // disable brownout problems
#include "soc/rtc_cntl_reg.h"    // disable brownout problems
#include "esp_http_server.h"

// Replace with your network credentials
const char* ssid = "Riaz";
const char* password = "34563456";

#define PART_BOUNDARY "123456789000000000000987654321"

#define CAMERA_MODEL_AI_THINKER

#if defined(CAMERA_MODEL_AI_THINKER)
  #define PWDN_GPIO_NUM     32
  #define RESET_GPIO_NUM    -1
  #define XCLK_GPIO_NUM      0
  #define SIOD_GPIO_NUM     26
  #define SIOC_GPIO_NUM     27
  
  #define Y9_GPIO_NUM       35
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       39
  #define Y6_GPIO_NUM       36
  #define Y5_GPIO_NUM       21
  #define Y4_GPIO_NUM       19
  #define Y3_GPIO_NUM       18
  #define Y2_GPIO_NUM        5
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     23
  #define PCLK_GPIO_NUM     22
#else
  #error "Camera model not selected"
#endif

#define MOTOR_1_PIN_1    15   // Left Motor IN1
#define MOTOR_1_PIN_2    13   // Left Motor IN2
#define MOTOR_2_PIN_1     14  // Right Motor IN3
#define MOTOR_2_PIN_2    12    // Right Motor IN4
#define LED_PIN          4    // Flash LED

String detectionServerIP = "10.133.183.84:5000";  // default detection server IP

static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t camera_httpd = NULL;
httpd_handle_t stream_httpd = NULL;

static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<html>
  <head>
    <title>ESP32-CAM Robot</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      body { font-family: Arial; text-align: center; margin:0px auto; padding-top: 30px;}
      table { margin-left: auto; margin-right: auto; }
      td { padding: 8px; }
      .button {
        background-color: #2f4468;
        border: none;
        color: white;
        padding: 10px 20px;
        text-align: center;
        font-size: 18px;
        margin: 6px 3px;
        cursor: pointer;
      }
      img { width: auto; max-width: 100%; height: auto; }
    </style>
  </head>
  <body>
    <h1>Agri Bot</h1>
    <img src="" id="photo" style="display:none;">
    <br>
    <button class="button" onclick="toggleVideo()">Toggle Video</button>
    <button class="button" onclick="toggleLED()">Toggle LED</button>
    <button onclick="detectLeaf()">Detect Disease</button>

    <div style="margin-top:20px;">
      <h3>Detection Server IP</h3>
      <input type="text" id="server-ip" placeholder="Enter server IP (e.g. 192.168.1.100:5000)">
      <button onclick="saveIP()">Save IP</button>
      <p>Current IP: <span id="current-ip">Not set</span></p>
    </div>

    <div id="result-tab" style="margin-top:20px; border:1px solid #ccc; padding:10px; display:block;">
      <h2>Result</h2>
      <div id="result-content">No result yet.</div>
    </div>

    </br>
    <table>
      <tr><td colspan="3" align="center"><button class="button" onmousedown="toggleCheckbox('forward');" onmouseup="toggleCheckbox('stop');">Forward</button></td></tr>
      <tr>
        <td align="center"><button class="button" onmousedown="toggleCheckbox('left');" onmouseup="toggleCheckbox('stop');">Left</button></td>
        <td align="center"><button class="button" onmousedown="toggleCheckbox('stop');">Stop</button></td>
        <td align="center"><button class="button" onmousedown="toggleCheckbox('right');" onmouseup="toggleCheckbox('stop');">Right</button></td>
      </tr>
      <tr><td colspan="3" align="center"><button class="button" onmousedown="toggleCheckbox('backward');" onmouseup="toggleCheckbox('stop');">Backward</button></td></tr>                   
    </table>

   <script>
     let streamOn = false;
     let ledOn = false;

     function toggleCheckbox(x) {
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/action?go=" + x, true);
        xhr.send();
     }

     function toggleVideo() {
        const img = document.getElementById("photo");
        if (!streamOn) {
          img.src = window.location.href.slice(0, -1) + ":81/stream";
          img.style.display = "block";
        } else {
          img.src = "";
          img.style.display = "none";
        }
        streamOn = !streamOn;
     }

     function saveIP() {
        const ip = document.getElementById("server-ip").value;
        fetch("/set_ip?ip=" + ip)
          .then(() => {
            document.getElementById("current-ip").innerText = ip;
          });
     }

     function detectLeaf() {
        fetch("/get_ip")
          .then(res => res.text())
          .then(ip => {
            return fetch("/capture", { method: "GET" })
              .then(response => response.blob())
              .then(imageBlob => {
                const formData = new FormData();
                formData.append("image", imageBlob, "leaf.jpg");
                return fetch("http://" + ip + "/detect", {
                  method: "POST",
                  body: formData
                });
              });
          })
          .then(response => response.json())
          .then(data => {
            let html = "";
            if (data.detections && data.detections.length > 0) {
              data.detections.forEach(det => {
                html += `Disease: ${det.disease}<br>Confidence: ${det.confidence}<br><br>`;
              });
            } else {
              html = "No disease detected.";
            }
            document.getElementById("result-content").innerHTML = html;
          })
          .catch(error => {
            document.getElementById("result-content").innerHTML = "Detection failed: " + error;
            console.error("Detection failed:", error);
          });
     }

     function toggleLED() {
        toggleCheckbox(ledOn ? 'led_off' : 'led_on');
        ledOn = !ledOn;
     }

     window.onload = () => {
        fetch("/get_ip")
          .then(res => res.text())
          .then(ip => {
            document.getElementById("current-ip").innerText = ip;
            document.getElementById("server-ip").value = ip;
          });
        document.getElementById("photo").src = window.location.href.slice(0, -1) + ":81/stream";
     }
   </script>
  </body>
</html>
)rawliteral";

static esp_err_t index_handler(httpd_req_t *req){
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}

static esp_err_t stream_handler(httpd_req_t *req){
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if(res != ESP_OK){
    return res;
  }

  while(true){
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
    } else {
      if(fb->width > 400){
        if(fb->format != PIXFORMAT_JPEG){
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if(!jpeg_converted){
            Serial.println("JPEG compression failed");
            res = ESP_FAIL;
          }
        } else {
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
        }
      }
    }
    if(res == ESP_OK){
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if(fb){
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if(_jpg_buf){
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if(res != ESP_OK){
      break;
    }
  }
  return res;
}

static esp_err_t cmd_handler(httpd_req_t *req){
  char*  buf;
  size_t buf_len;
  char variable[32] = {0,};
  
  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (buf_len > 1) {
    buf = (char*)malloc(buf_len);
    if(!buf){
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
      if (httpd_query_key_value(buf, "go", variable, sizeof(variable)) == ESP_OK) {
      } else {
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
      }
    } else {
      free(buf);
      httpd_resp_send_404(req);
      return ESP_FAIL;
    }
    free(buf);
  } else {
    httpd_resp_send_404(req);
    return ESP_FAIL;
  }

  if (!strcmp(variable, "led_on")) {
    digitalWrite(LED_PIN, HIGH);
  }
  else if (!strcmp(variable, "led_off")) {
    digitalWrite(LED_PIN, LOW);
  }
  else if(!strcmp(variable, "forward")) {
    digitalWrite(MOTOR_1_PIN_1, HIGH);
    digitalWrite(MOTOR_1_PIN_2, LOW);
    digitalWrite(MOTOR_2_PIN_1, HIGH);
    digitalWrite(MOTOR_2_PIN_2, LOW);
  }
  else if(!strcmp(variable, "backward")) {
    digitalWrite(MOTOR_1_PIN_1, LOW);
    digitalWrite(MOTOR_1_PIN_2, HIGH);
    digitalWrite(MOTOR_2_PIN_1, LOW);
    digitalWrite(MOTOR_2_PIN_2, HIGH);
  }
  else if(!strcmp(variable, "left")) {
    digitalWrite(MOTOR_1_PIN_1, LOW);
    digitalWrite(MOTOR_1_PIN_2, HIGH);
    digitalWrite(MOTOR_2_PIN_1, HIGH);
    digitalWrite(MOTOR_2_PIN_2, LOW);
  }
  else if(!strcmp(variable, "right")) {
    digitalWrite(MOTOR_1_PIN_1, HIGH);
    digitalWrite(MOTOR_1_PIN_2, LOW);
    digitalWrite(MOTOR_2_PIN_1, LOW);
    digitalWrite(MOTOR_2_PIN_2, HIGH);
  }
  else if(!strcmp(variable, "stop")) {
    digitalWrite(MOTOR_1_PIN_1, LOW);
    digitalWrite(MOTOR_1_PIN_2, LOW);
    digitalWrite(MOTOR_2_PIN_1, LOW);
    digitalWrite(MOTOR_2_PIN_2, LOW);
  }

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, NULL, 0);
}

void startCameraServer(){
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  httpd_uri_t index_uri = {"/", HTTP_GET, index_handler, NULL};
  httpd_uri_t cmd_uri = {"/action", HTTP_GET, cmd_handler, NULL};
  httpd_uri_t stream_uri = {"/stream", HTTP_GET, stream_handler, NULL};

  httpd_uri_t capture_uri = {
    "/capture", HTTP_GET,
    [](httpd_req_t *req) -> esp_err_t {
      camera_fb_t *fb = esp_camera_fb_get();
      if (!fb) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
      }
      httpd_resp_set_type(req, "image/jpeg");
      httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
      esp_err_t res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
      esp_camera_fb_return(fb);
      return res;
    },
    NULL
  };

  // NEW: set_ip and get_ip endpoints
  httpd_uri_t set_ip_uri = {
    "/set_ip", HTTP_GET,
    [](httpd_req_t *req) -> esp_err_t {
      char buf[64];
      size_t buf_len = httpd_req_get_url_query_len(req) + 1;
      if (buf_len > 1 && buf_len < sizeof(buf)) {
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
          char param[64];
          if (httpd_query_key_value(buf, "ip", param, sizeof(param)) == ESP_OK) {
            detectionServerIP = String(param);
            Serial.println("Detection Server IP updated to: " + detectionServerIP);
          }
        }
      }
      return httpd_resp_send(req, "OK", 2);
    },
    NULL
  };

  httpd_uri_t get_ip_uri = {
    "/get_ip", HTTP_GET,
    [](httpd_req_t *req) -> esp_err_t {
      return httpd_resp_send(req, detectionServerIP.c_str(), detectionServerIP.length());
    },
    NULL
  };

  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(camera_httpd, &index_uri);
    httpd_register_uri_handler(camera_httpd, &cmd_uri);
    httpd_register_uri_handler(camera_httpd, &capture_uri);
    httpd_register_uri_handler(camera_httpd, &set_ip_uri);
    httpd_register_uri_handler(camera_httpd, &get_ip_uri);
  }

  config.server_port += 1;
  config.ctrl_port += 1;
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
  }
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(MOTOR_1_PIN_1, OUTPUT);
  pinMode(MOTOR_1_PIN_2, OUTPUT);
  pinMode(MOTOR_2_PIN_1, OUTPUT);
  pinMode(MOTOR_2_PIN_2, OUTPUT);
  
  Serial.begin(115200);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed");
    return;
  }
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("Camera Stream Ready! Go to: http://");
  Serial.println(WiFi.localIP());
  
  startCameraServer();
}

void loop() {
}
