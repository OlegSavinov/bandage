//#include "types.h"
#include "peak_detection.h"

#include <Adafruit_LSM6DS3TRC.h>
#include <TensorFlowLite.h>
#include "Adafruit_TFLite.h"

#include "model.h"

// For SPI mode, we need a CS pin
#define LSM_CS 10
// For software-SPI mode we need SCK/MOSI/MISO pins
#define LSM_SCK 13
#define LSM_MISO 12
#define LSM_MOSI 11

Adafruit_LSM6DS3TRC lsm6ds3trc;

const char* GESTURES[] = {
    "bench_press",
    "chest_fly",
    "arm_curl",
    "rest",
};

#define NUM_GESTURES (sizeof(GESTURES) / sizeof(GESTURES[0]))

const int kTensorAreaSize  (4 * 1024);
Adafruit_TFLite ada_tflite(kTensorAreaSize);

// Setup function for BLE and services
void setup() {
  Serial.begin(9600);
  while (!Serial)
    delay(10);

  if (!lsm6ds3trc.begin_I2C()) {
    // if (!lsm6ds3trc.begin_SPI(LSM_CS)) {
    // if (!lsm6ds3trc.begin_SPI(LSM_CS, LSM_SCK, LSM_MISO, LSM_MOSI)) {
    Serial.println("Failed to find LSM6DS3TR-C chip");
    while (1) {
      delay(10);
    }
  }

  if (! ada_tflite.begin()) {
    Serial.println("Failed to initialize TFLite");
    while (1) yield();
  }

  if (! ada_tflite.loadModel(model)) {
    Serial.println("Failed to load model");
    while (1) yield();
  }

  lsm6ds3trc.configInt1(false, false, true); // accelerometer DRDY on INT1
  lsm6ds3trc.configInt2(false, true, false); // gyro DRDY on INT2
  Serial.println("Setup successful");
}

void loop() {
  // No automatic incrementing, we wait for clicks from the phone
  SensorData data = getSensorData();

  if (detectMotion(data)){
    Serial.println("Motion detected");
    loadAccumulatedDataIntoModel();

    if (runModel()){
      printModelOutput();
    }
  }

  delay(70);
}

void loadAccumulatedDataIntoModel(){
  SensorData* data = getAccumulatedSensorData();

  for (short_i i = 0; i < WINDOW_SIZE; i++){
    ada_tflite.input->data.f[i * 6 + 0] = data[i].ax;
    ada_tflite.input->data.f[i * 6 + 1] = data[i].ay;
    ada_tflite.input->data.f[i * 6 + 2] = data[i].az;
    ada_tflite.input->data.f[i * 6 + 3] = data[i].gx;
    ada_tflite.input->data.f[i * 6 + 4] = data[i].gy;
    ada_tflite.input->data.f[i * 6 + 5] = data[i].gz;
  }
}

bool runModel(){
  TfLiteStatus invoke_status = ada_tflite.interpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
    ada_tflite.error_reporter->Report("Invoke failed");
    return false;
  }
  return true;
}

void printModelOutput(){
  // Loop through the output tensor values from the model
  for (int i = 0; i < NUM_GESTURES; i++) {
    Serial.print(GESTURES[i]);
    Serial.print(": ");
    Serial.println(ada_tflite.output->data.f[i], 6);
  }
  Serial.println();
}

SensorData getSensorData(){
  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t temp;
  lsm6ds3trc.getEvent(&accel, &gyro, &temp);

  SensorData data = {
    accel.acceleration.x,
    accel.acceleration.y,
    accel.acceleration.z,
    gyro.gyro.x,
    gyro.gyro.y,
    gyro.gyro.z,
  };

  return data;
}