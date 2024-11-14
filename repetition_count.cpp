#include <math.h>
#include "types.h"
#include "Arduino.h"

// ==================== Adjustable Parameters ==================== //

// Threshold for detecting a repetition based on magnitude
const float MAGNITUDE_THRESHOLD = 1.0;  // Adjust based on data scaling

// Minimum number of samples between repetitions to avoid double-counting
const int MIN_PEAK_DISTANCE = 20;  // Adjust as needed based on sampling rate and exercise speed

// Leading Axis Configuration
// Define the leading axis for each exercise. If an exercise does not have a leading axis, set it to NULL.
const char* LEADING_AXIS = "gX";  // Example: "gX" for chest_fly, "aZ" for arm_curl, NULL for bench_press

// Weight multiplier for the leading axis
const float LEADING_AXIS_WEIGHT = 10.0;  // As per your request

// Low-Pass Filter Parameters (Exponential Moving Average)
const float ALPHA = 0.1;  // Smoothing factor for EMA. Adjust between 0 (no smoothing) and 1 (maximum smoothing)

// Size definitions
const int WINDOW_SIZE = 100;          // Size of the sliding window buffer
const int MODEL_SAMPLE_SIZE = 40;     // Number of samples to accumulate for model input

// ==================== Global Variables ==================== //

// Sliding window to store recent sensor data
SensorData window[WINDOW_SIZE];       // Circular buffer for sensor data
float magnitudes[WINDOW_SIZE];        // Array to store magnitudes
int currentIndex = 0;                 // Current position in the window
bool windowFull = false;              // Flag to indicate when the window is full

// Low-Pass Filter Variable
float filteredMagnitude = 0.0;        // Filtered magnitude using EMA

// Peak Detection Variables
int peakCount = 0;                     // Total number of detected peaks (repetitions)
int lastPeakIndex = -MIN_PEAK_DISTANCE; // Index of the last detected peak

// ==================== Function Definitions ==================== //

/**
 * @brief Prints sensor data for debugging purposes.
 * 
 * @param d SensorData structure containing accelerometer and gyroscope data.
 */
void printSensorData(SensorData d) {
  String output = "{ax:" + String(d.ax) + 
                  ",ay:" + String(d.ay) + 
                  ",az:" + String(d.az) + 
                  ",gx:" + String(d.gx) + 
                  ",gy:" + String(d.gy) + 
                  ",gz:" + String(d.gz) + "}";
  Serial.println(output);
}

/**
 * @brief Calculates the weighted magnitude of differences between current and previous sensor data.
 * 
 * @param current Current sensor data.
 * @param previous Previous sensor data.
 * @return float Weighted magnitude.
 */
float calculateWeightedMagnitude(SensorData current, SensorData previous) {
    // Calculate differences
    float ax_diff = current.ax - previous.ax;
    float ay_diff = current.ay - previous.ay;
    float az_diff = current.az - previous.az;
    float gx_diff = current.gx - previous.gx;
    float gy_diff = current.gy - previous.gy;
    float gz_diff = current.gz - previous.gz;

    // Initialize weights
    float accelWeight = 1.0;
    float gyroWeight = 1.0;

    // Apply higher weight to the leading axis if defined
    if (LEADING_AXIS != NULL) {
        if (strcmp(LEADING_AXIS, "aX") == 0) {
            accelWeight = LEADING_AXIS_WEIGHT;
        }
        else if (strcmp(LEADING_AXIS, "aY") == 0) {
            accelWeight = LEADING_AXIS_WEIGHT;
        }
        else if (strcmp(LEADING_AXIS, "aZ") == 0) {
            accelWeight = LEADING_AXIS_WEIGHT;
        }
        else if (strcmp(LEADING_AXIS, "gX") == 0) {
            gyroWeight = LEADING_AXIS_WEIGHT;
        }
        else if (strcmp(LEADING_AXIS, "gY") == 0) {
            gyroWeight = LEADING_AXIS_WEIGHT;
        }
        else if (strcmp(LEADING_AXIS, "gZ") == 0) {
            gyroWeight = LEADING_AXIS_WEIGHT;
        }
    }

    // Calculate weighted magnitudes
    float accelMagnitude = accelWeight * sqrt(ax_diff * ax_diff + ay_diff * ay_diff + az_diff * az_diff);
    float gyroMagnitude = gyroWeight * sqrt(gx_diff * gx_diff + gy_diff * gy_diff + gz_diff * gz_diff);

    // Total magnitude
    float totalMagnitude = accelMagnitude + gyroMagnitude;

    return totalMagnitude;
}

/**
 * @brief Applies an Exponential Moving Average (EMA) as a low-pass filter to smooth the magnitude.
 * 
 * @param magnitude Current magnitude to be filtered.
 */
void applyLowPassFilter(float magnitude) {
    filteredMagnitude = ALPHA * magnitude + (1 - ALPHA) * filteredMagnitude;
}

/**
 * @brief Checks if the current point is a local maximum within a specified window.
 * 
 * @param index Current index in the magnitudes array.
 * @return true If it's a local maximum.
 * @return false Otherwise.
 */
bool isLocalMaximum(int index) {
    // Define the window range around the current index
    int windowRange = 5;  // Number of samples to check on each side

    // Iterate through the window range
    for (int i = 1; i <= windowRange; i++) {
        int leftIndex = (index - i + WINDOW_SIZE) % WINDOW_SIZE;
        int rightIndex = (index + i) % WINDOW_SIZE;

        // Compare magnitudes
        if (filteredMagnitude <= magnitudes[leftIndex] || filteredMagnitude <= magnitudes[rightIndex]) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Updates the sliding window with new sensor data and detects repetitions.
 * 
 * @param newData New sensor data to be added to the window.
 * @return true If a repetition is detected.
 * @return false Otherwise.
 */
bool detectRepetition(SensorData newData) {
    // Calculate the previous index
    int previousIndex = (currentIndex == 0) ? WINDOW_SIZE - 1 : currentIndex - 1;

    // Update the sliding window with new data
    window[currentIndex] = newData;

    // Calculate weighted magnitude if the window is full
    if (windowFull) {
        magnitudes[currentIndex] = calculateWeightedMagnitude(window[currentIndex], window[previousIndex]);
        
        // Apply low-pass filter
        applyLowPassFilter(magnitudes[currentIndex]);

        // Check for peak (repetition)
        if (isLocalMaximum(currentIndex) && filteredMagnitude > MAGNITUDE_THRESHOLD) {
            // Ensure enough distance from the last detected peak
            if ((currentIndex - lastPeakIndex + WINDOW_SIZE) % WINDOW_SIZE >= MIN_PEAK_DISTANCE) {
                peakCount++;
                lastPeakIndex = currentIndex;
                Serial.print("Repetition detected! Total repetitions: ");
                Serial.println(peakCount);
                // Optionally, you can perform additional actions here when a repetition is detected
                return true;
            }
        }
    }

    // Move to the next index
    currentIndex = (currentIndex + 1) % WINDOW_SIZE;

    // Check if the window is now full
    if (currentIndex == 0) {
        windowFull = true;
    }

    return false;
}

/**
 * @brief Retrieves accumulated sensor data from the sliding window.
 * 
 * @return SensorData* Pointer to the accumulated sensor data array.
 */
SensorData* getAccumulatedSensorData() {
    // Create a new array to hold the accumulated sensor data
    static SensorData accumulatedData[MODEL_SAMPLE_SIZE];
    
    // Traverse backward from the currentIndex, wrapping around if necessary
    for (int i = 0; i < MODEL_SAMPLE_SIZE; i++) {
        // Calculate the index in the circular buffer, wrapping around using modulo
        int bufferIndex = (currentIndex - i - 1 + WINDOW_SIZE) % WINDOW_SIZE;
        accumulatedData[i] = window[bufferIndex];
    }
    
    // Return the array containing the accumulated sensor data
    return accumulatedData;
}
