// #include <math.h>
// #include "types.h"
// #include "Arduino.h"

// #define ZSCORE_THRESHOLD 1.4  // Z-score threshold for peak detection

// typedef unsigned short int short_i;

// // Circular buffer to store a rolling window of data for each axis
// float axData[WINDOW_SIZE];
// float ayData[WINDOW_SIZE];
// float azData[WINDOW_SIZE];
// float gxData[WINDOW_SIZE];
// float gyData[WINDOW_SIZE];
// float gzData[WINDOW_SIZE];

// // Index to keep track of where to insert the next value in the buffer
// int bufferIndex = 0;
// int currentSize = 0;

// // Function to compute the mean of sensor axis values over the rolling window
// float computeMean(float* data, int size) {
//     float sum = 0;
//     for (int i = 0; i < size; i++) {
//         sum += data[i];
//     }
//     return sum / size;
// }

// // Function to compute the standard deviation of sensor axis values over the rolling window
// float computeStdDev(float* data, int size, float mean) {
//     float variance = 0;
//     for (int i = 0; i < size; i++) {
//         variance += pow(data[i] - mean, 2);
//     }
//     return sqrt(variance / size);
// }

// // Function to compute Z-score and check for peaks
// bool detectPeak(float* data) {
//     // Calculate mean and standard deviation
//     float mean = computeMean(data, currentSize);
//     float stdDev = computeStdDev(data, currentSize, mean);

//     // Avoid division by zero
//     if (stdDev == 0) return false;

//     // Calculate Z-score
//     float zScore = (data[(bufferIndex - 1 + WINDOW_SIZE) % WINDOW_SIZE] - mean) / stdDev;

//     String p = "zScore:" + String(zScore);
//     Serial.println(p);

//     // Check if Z-score exceeds the threshold
//     return fabs(zScore) > ZSCORE_THRESHOLD;
// }

// SensorData* getAccumulatedSensorData(){
//   SensorData* result = new SensorData[WINDOW_SIZE];

//   for (short_i i = 0; i < (sizeof(axData) / sizeof(axData[0])); i++) {
//     result[i] = { axData[i], ayData[i], azData[i], gxData[i], gyData[i], gzData[i] };
//   }
  
//   return result;
// }

// // Function to add data to the circular buffer for one axis
// void addToBuffer(float* data, float newValue) {
//   data[bufferIndex] = newValue;
// }

// // Main function to detect peaks based on accelerometer and gyroscope data
// bool detectMotionZScore(SensorData currentData) {
//     // Add the new data to the rolling window
//     addToBuffer(axData, currentData.ax);
//     addToBuffer(ayData, currentData.ay);
//     addToBuffer(azData, currentData.az);
//     addToBuffer(gxData, currentData.gx);
//     addToBuffer(gyData, currentData.gy);
//     addToBuffer(gzData, currentData.gz);

//     bool peakDetected = false;

//     // Check for peaks on any axis (accelerometer and gyroscope)
//     //if (currentSize == WINDOW_SIZE) {
//         if (detectPeak(axData) || 
//             detectPeak(ayData) || 
//             detectPeak(azData) ||
//             detectPeak(gxData) || 
//             detectPeak(gyData) || 
//             detectPeak(gzData)) {
//             peakDetected = true;  // Peak detected
//         }
//     //}

//     //Serial.println("currentSize: " + currentSize);

//     // Update buffer index
//     bufferIndex = (bufferIndex + 1) % WINDOW_SIZE;

//     // Increment the current size of the buffer until it's full
//     if (currentSize < WINDOW_SIZE) {
//         currentSize++;
//     }

//     // Clear previous data after a peak is detected
//     if (peakDetected) {
//         currentSize = 0;
//         bufferIndex = 0;
//     }

//     return peakDetected;
// }

// void printSensorData(SensorData d){
//     String output = "Data : { ax: " + String(d.ax) + 
//                   ", ay: " + String(d.ay) + 
//                   ", az: " + String(d.az) + 
//                   ", gx: " + String(d.gx) + 
//                   ", gy: " + String(d.gy) + 
//                   ", gz: " + String(d.gz) + " }";
//   Serial.println(output);
// }
