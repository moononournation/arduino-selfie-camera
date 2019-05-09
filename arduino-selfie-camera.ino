#include <esp_camera.h>
#include "ST7789.h"
#include <SPI.h>
#include <SD.h>
#include <FS.h>

#define PREVIEW_SIZE FRAMESIZE_HQVGA

#define PWDN_GPIO_NUM       -1
#define RESET_GPIO_NUM      -1
#define XCLK_GPIO_NUM       4
#define SIOD_GPIO_NUM       18
#define SIOC_GPIO_NUM       23

#define Y9_GPIO_NUM         36
#define Y8_GPIO_NUM         37
#define Y7_GPIO_NUM         38
#define Y6_GPIO_NUM         39
#define Y5_GPIO_NUM         35
#define Y4_GPIO_NUM         26
#define Y3_GPIO_NUM         13
#define Y2_GPIO_NUM         34
#define VSYNC_GPIO_NUM      5
#define HREF_GPIO_NUM       27
#define PCLK_GPIO_NUM       25

#define SDCARA_CS           0

ST7789 tft = ST7789();
sensor_t *s;
int i = 0;
int file_idx = 1;
char buff[256];
camera_fb_t *fb = NULL;

void setup()
{
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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;

  //init with high specs to pre-allocate larger buffers
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  config.frame_size = FRAMESIZE_UXGA;
  config.jpeg_quality = 1;
  config.fb_count = 2;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
  }

  s = esp_camera_sensor_get();
  s->set_brightness(s, 2);
  s->set_contrast(s, 1);
  s->set_saturation(s, 1);
  s->set_sharpness(s, 2);
  s->set_gainceiling(s, GAINCEILING_128X);
  s->set_aec2(s, 1);
  s->set_denoise(s, 1);
  //s->set_hmirror(s, 1);
  //s->set_vflip(s, 1);
  s->set_framesize(s, PREVIEW_SIZE);
  s->set_pixformat(s, PIXFORMAT_RGB565);

  SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);

  tft.begin();
  tft.invertDisplay(true);
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(1);
  tft.drawString("ESP32 Digital Camera", 70, 0);

  if (!SD.begin(SDCARA_CS)) {
    tft.drawString("SD Init Fail!", 0, 208);
    Serial.println("SD Init Fail!");
  } else {
    snprintf(buff, sizeof(buff), "SD Init Pass Type: %d Size: %lu", SD.cardType(), SD.cardSize() / 1024 / 1024);
    tft.drawString(buff, 0, 212);
    Serial.println(buff);

    File file = SD.open("/DCIM");
    if (!file) {
      Serial.println("Create /DCIM");
      SD.mkdir("/DCIM");
    } else {
      Serial.println("Found /DCIM");
      file.close();
    }
    file = SD.open("/DCIM/100ESPDC");
    if (!file) {
      Serial.println("Create /DCIM/100ESPDC");
      SD.mkdir("/DCIM/100ESPDC");
    } else {
      Serial.println("Found /DCIM/100ESPDC");
      file.close();
    }
  }
}

void findNextFileIdx() {
  File file;
  while (true) {
    snprintf(buff, sizeof(buff), "/DCIM/100ESPDC/DSC%05D.JPG\n", file_idx);
    file = SD.open(buff);
    if (file) {
      //Serial.printf("Found %s\n", buff);
      file.close();
      file_idx++;
    } else {
      Serial.printf("Next file: %s\n", buff);
      return;
    }
  }
}

void snap() {
  s->set_pixformat(s, PIXFORMAT_JPEG);
  s->set_framesize(s, FRAMESIZE_UXGA);
  s->set_hmirror(s, 0);
  for (uint8_t gs = 0; gs < 250; gs += 10) {
    tft.fillRect(0, 32, 240, 176, tft.color565(gs, gs, gs));
  }
  tft.fillRect(0, 32, 240, 176, TFT_WHITE);
  fb = esp_camera_fb_get();

  if (!fb) {
    Serial.println("Camera capture JPG failed");
  } else {
    findNextFileIdx();
    File file = SD.open(buff, FILE_WRITE);
    if (file.write(fb->buf, fb->len)) {
      snprintf(buff, sizeof(buff), "File written: %luKB\n%s", fb->len / 1024, file.name());
      tft.setTextSize(1);
      tft.drawString(buff, 0, 226);
      Serial.println(buff);
    } else {
      tft.drawString("Write failed!", 0, 218);
      Serial.println("Write failed!");
    }
    file.close();
    esp_camera_fb_return(fb);
  }

  s->set_framesize(s, PREVIEW_SIZE);
  s->set_pixformat(s, PIXFORMAT_RGB565);
  s->set_hmirror(s, 1);
  fb = esp_camera_fb_get();
  esp_camera_fb_return(fb);
  fb = NULL;
}

void loop()
{
  if (i == 1) {
    tft.setTextSize(2);
    tft.drawString("3", 116, 12);
    findNextFileIdx();
  } else if (i == 11) {
    tft.setTextSize(2);
    tft.drawString("2", 116, 12);
  } else if (i == 21) {
    tft.setTextSize(2);
    tft.drawString("1", 116, 12);
  } else if (i == 31) {
    tft.setTextSize(2);
    tft.drawString("Cheeze!", 92, 12);

    snap();

    tft.setTextSize(2);
    tft.drawString("Reset to snap again!", 0, 12);
  } else if (i == 600) {
    tft.end();
    esp_deep_sleep_start();
  } else {
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.printf("Camera capture RGB failed");
    } else {
      tft.pushRect(0, 32, fb->width, fb->height, (uint16_t*)fb->buf);
      esp_camera_fb_return(fb);
      fb = NULL;
    }
  }

  i++;
}
