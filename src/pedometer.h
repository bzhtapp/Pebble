const uint32_t s_step_key = 1;
const uint32_t s_samples_per_update = 4;
const uint16_t s_step_min_diff = 300;
const uint16_t s_step_threshold = 600;
const AccelSamplingRate s_acelSamplingRate = ACCEL_SAMPLING_10HZ;

static void update_step(int steps);
static void accel_data_handler(AccelData *data, uint32_t num_samples);