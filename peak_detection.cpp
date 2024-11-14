#include <math.h>
#include "types.h"
#include "Arduino.h"

// Constants for peak detection parameters
const float diffThreshold = 50.0;
const int minPeakDistance = 15;
const float accelWeight = 0.85;
const float gyroWeight = 0.1;
const float differenceWeightFactor = 2.0;
const int peakWindow = 5;

// Global variables for real-time detection
SensorData window[WINDOW_SIZE];  // Sliding window to store recent sensor data
float magnitudes[WINDOW_SIZE];  // Array to store magnitudes
int currentIndex = 0;  // Index to keep track of the current position in the window
bool windowFull = false;  // Flag to indicate when the window is full
int peakCount = 0;
int lastPeakIndex = -WINDOW_SIZE;

// Function to print sensor data for debugging
void printSensorData(SensorData d) {
  String output = "{ax:" + String(d.ax) + 
                  ",ay:" + String(d.ay) + 
                  ",az:" + String(d.az) + 
                  ",gx:" + String(d.gx) + 
                  ",gy:" + String(d.gy) + 
                  ",gz:" + String(d.gz) + "}";
  Serial.println(output);
}

// Function to calculate the weighted magnitude of differences
float calculateWeightedDifferenceMagnitude(SensorData current, SensorData previous) {
    float ax_diff = current.ax - previous.ax;
    float ay_diff = current.ay - previous.ay;
    float az_diff = current.az - previous.az;
    float gx_diff = current.gx - previous.gx;
    float gy_diff = current.gy - previous.gy;
    float gz_diff = current.gz - previous.gz;

    // Apply weights to accelerometer and gyroscope differences
    float accelMagnitude = accelWeight * sqrt(ax_diff * ax_diff + ay_diff * ay_diff + az_diff * az_diff);
    float gyroMagnitude = gyroWeight * sqrt(gx_diff * gx_diff + gy_diff * gy_diff + gz_diff * gz_diff);

    // Calculate the total magnitude and apply a dynamic difference weight
    float totalMagnitude = accelMagnitude + gyroMagnitude;
    float differenceWeight = 1 + (differenceWeightFactor * totalMagnitude);  // Dynamic weight

    return totalMagnitude * differenceWeight;
}

// Function to check if the current point is a local maximum within a window
bool isLocalMaximum(int index) {
    for (int j = 1; j <= peakWindow; j++) {
        int leftIndex = (index - j + WINDOW_SIZE) % WINDOW_SIZE;
        int rightIndex = (index + j) % WINDOW_SIZE;

        if (magnitudes[index] <= magnitudes[leftIndex] || magnitudes[index] <= magnitudes[rightIndex]) {
            return false;
        }
    }
    return true;
}

// Function to update the sliding window and detect peaks
bool detectMotion(SensorData newData) {
  // Update the sliding window
  int previousIndex = (currentIndex == 0) ? WINDOW_SIZE - 1 : currentIndex - 1;
  window[currentIndex] = newData;
  bool result = false;

  // Calculate the weighted difference magnitude
  if (windowFull) {
      magnitudes[currentIndex] = calculateWeightedDifferenceMagnitude(window[currentIndex], window[previousIndex]);
  }

  // Check for peaks if the window is full
  if (windowFull) {
      //Serial.println("Magnitude:"+String(magnitudes[currentIndex]));
      //Serial.println("Threshold:"+String(diffThreshold));
      // Check if the current point is a local maximum in the peakWindow range
      if (isLocalMaximum(currentIndex) && magnitudes[currentIndex] > diffThreshold) {
        if ((currentIndex - lastPeakIndex + WINDOW_SIZE) % WINDOW_SIZE >= minPeakDistance) {
          peakCount++;
          lastPeakIndex = currentIndex;
          Serial.print("Repetition detected! Total repetitions: ");
          Serial.println(peakCount);
          result = true;
        }
      }
  }

  // Move to the next index
  currentIndex = (currentIndex + 1) % WINDOW_SIZE;
  if (currentIndex == 0) {
      windowFull = true;  // Mark the window as full once we've cycled through all indices
  }

  return result;
}

// Function to get accumulated sensor data from the circular buffer
SensorData* getAccumulatedSensorData() {
  // Create a new array to hold the accumulated sensor data
  static SensorData accumulatedData[MODEL_SAMPLE_SIZE];
  
  // Traverse backward from the current_index, wrapping around if necessary
  for (int i = 0; i < MODEL_SAMPLE_SIZE; i++) {
    // Calculate the index in the circular buffer, wrapping around using modulo
    int bufferIndex = (currentIndex - i + WINDOW_SIZE) % WINDOW_SIZE;

    // Copy the data from the circular buffer into the accumulatedData array
    accumulatedData[i] = window[bufferIndex];
  }
  
  // Return the array containing the accumulated sensor data
  return accumulatedData;
}