#define WINDOW_SIZE 100    // Number of previous samples to use in the rolling window
#define MODEL_SAMPLE_SIZE 28

// Struct to store sensor data
typedef struct {
    float ax, ay, az, gx, gy, gz;
} SensorData;

typedef unsigned short int short_i;