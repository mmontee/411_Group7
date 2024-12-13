/**

//#include <Arduino.h>
#include "HX711.h"
#include "esp_camera.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"

// Pin Definitions
#define HX711_DOUT 4  // Define your DOUT pin for HX711
#define HX711_SCK 5   // Define your SCK pin for HX711
#define LED_PIN 3     // GPIO4 for LED

// Define board/camera model
#define CAMERA_MODEL_XIAO_ESP32S3  // Has PSRAM

// HX711 and Camera Setup
HX711 scale;
bool picture_taken = false;
unsigned long start_time = 0;
bool above_zero_for_10_sec = false;

// Camera Pins - Adjust according to your model
#include "camera_pins.h"


// Save pictures to SD card
void photo_save(const char * fileName) {
  // Take a photo
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Failed to get camera frame buffer");
    return;
  }
  // Save photo to file
  writeFile(SD, fileName, fb->buf, fb->len);
  
  // Release image buffer
  esp_camera_fb_return(fb);

  Serial.println("Photo saved to file");
}

// SD card write file
void writeFile(fs::FS &fs, const char * path, uint8_t * data, size_t len){
    Serial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.write(data, len) == len){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void setup() {
  //Camera setup
  {
    Serial.begin(115200);
    while (!Serial)
      ;  // When the serial monitor is turned on, the program starts to execute

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
    config.frame_size = FRAMESIZE_UXGA;
    config.pixel_format = PIXFORMAT_JPEG;  // for streaming
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = 12;
    config.fb_count = 1;

    // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
    //                      for larger pre-allocated frame buffer.
    if (config.pixel_format == PIXFORMAT_JPEG) {
      if (psramFound()) {
        config.jpeg_quality = 10;
        config.fb_count = 2;
        config.grab_mode = CAMERA_GRAB_LATEST;
      } else {
        // Limit the frame size when PSRAM is not available
        config.frame_size = FRAMESIZE_SVGA;
        config.fb_location = CAMERA_FB_IN_DRAM;
      }
    } else {
      // Best option for face detection/recognition
      config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
      config.fb_count = 2;
#endif
    }

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
      Serial.printf("Camera init failed with error 0x%x", err);
      return;
    }

    camera_sign = true;  // Camera initialization check passes

    // Initialize SD card
    if (!SD.begin(21)) {
      Serial.println("Card Mount Failed");
      return;
    }
    uint8_t cardType = SD.cardType();

    // Determine if the type of SD card is available
    if (cardType == CARD_NONE) {
      Serial.println("No SD card attached");
      return;
    }

    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC) {
      Serial.println("MMC");
    } else if (cardType == CARD_SD) {
      Serial.println("SDSC");
    } else if (cardType == CARD_SDHC) {
      Serial.println("SDHC");
    } else {
      Serial.println("UNKNOWN");
    }

    sd_sign = true;  // sd initialization check passes
  }
}

void loop() {
  // Read HX711
  long weight = scale.get_units();
  Serial.println(weight);

  // Turn LED on or off
  if (weight > 400) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }

  // Check if weight is greater than 0 for 10 seconds
  if (weight > 0) {
    if (!above_zero_for_10_sec) {
      if (start_time == 0) {
        start_time = millis();
      } else if (millis() - start_time > 10000) {
        above_zero_for_10_sec = true;
      }
    }
  } else {
    start_time = 0;
    above_zero_for_10_sec = false;
  }

  // Take and save a picture if above_zero_for_10_sec
  if (above_zero_for_10_sec && !picture_taken) {
    takePicture();
    picture_taken = true;  // Ensure we only take one picture
  } else if (!above_zero_for_10_sec) {
    picture_taken = false;  // Reset when condition no longer met
  }

  delay(100);
}

void takePicture() {
  Serial.println("Taking picture...");
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  // Save to SD Card
  String path = "/picture.jpg";
  File file = SD_MMC.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("File open failed");
  } else {
    file.write(fb->buf, fb->len);  // Save picture
    file.close();
        Serial.println("Picture saved to "/" + path");
  }
  esp_camera_fb_return(fb);  // Return frame buffer
}

**/
#include <Arduino.h>
#include "HX711.h"
#include "esp_camera.h"
#include <SD_MMC.h>

// Pin Definitions
#define HX711_DOUT  5  // Define your DOUT pin for HX711
#define HX711_SCK   6  // Define your SCK pin for HX711
#define LED_PIN     4  // GPIO4 for LED

// HX711 and Camera Setup
HX711 scale;
bool picture_taken = false;
unsigned long start_time = 0;
bool above_zero_for_10_sec = false;
unsigned long reset_timer = 0;
int image_count = 1;

// Define camera/model
#define CAMERA_MODEL_XIAO_ESP32S3 // Has PSRAM
// Camera Pins - Adjust according to your model
#include "camera_pins.h"

void setup() {
    Serial.begin(115200);

    // Initialize HX711
    scale.begin(HX711_DOUT, HX711_SCK);

    // Initialize GPIO4 for LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // Initialize the camera
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
    config.pixel_format = PIXFORMAT_JPEG;

    // Frame size: Use your preference (e.g., FRAMESIZE_UXGA for 1600x1200)
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 10;  // 1-63, lower number means higher quality
    config.fb_count = 1;

    if (!esp_camera_init(&config)) {
        Serial.println("Camera init failed!");
        return;
    }

    // Initialize SD card
    if (!SD_MMC.begin()) {
        Serial.println("SD Card initialization failed!");
        return;
    }
    Serial.println("SD Card initialized.");

    reset_timer = millis();
}

void loop() {
    // Read HX711
    long weight = scale.get_units();
    Serial.println(weight);

    // Reset conditions every minute
    if (millis() - reset_timer > 60000) {
        start_time = 0;
        above_zero_for_10_sec = false;
        picture_taken = false;
        reset_timer = millis();
    }

    // Turn LED on or off
    if (weight > 400) {
        digitalWrite(LED_PIN, HIGH);
    } else {
        digitalWrite(LED_PIN, LOW);
    }

    // Check if weight is greater than 0 for 10 seconds
    if (weight > 0) {
        if (!above_zero_for_10_sec) {
            if (start_time == 0) {
                start_time = millis();
            } else if (millis() - start_time > 10000) {
                above_zero_for_10_sec = true;
            }
        }
    } else {
        start_time = 0;
        above_zero_for_10_sec = false;
    }

    // Take and save a picture if above_zero_for_10_sec
    if (above_zero_for_10_sec && !picture_taken) {
        char filename[32];
        sprintf(filename, "/image%d.jpg", image_count);
        takePicture(filename);
        picture_taken = true;  // Ensure we only take one picture
        image_count++;
    }

    delay(100);
}

void takePicture(const char *filename) {
    Serial.println("Taking picture...");
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        return;
    }

    // Save to SD Card
    File file = SD_MMC.open(filename, FILE_WRITE);
    if (!file) {
        Serial.println("File open failed");
    } else {
        file.write(fb->buf, fb->len);  // Save picture
        file.close();
        Serial.printf("Picture saved to %s\n", filename);
    }
    esp_camera_fb_return(fb);  // Return frame buffer
}


