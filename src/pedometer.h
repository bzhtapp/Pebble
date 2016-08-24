const uint32_t s_step_key = 1;
const uint32_t s_samples_per_update = 5;
const uint16_t s_step_min_diff = 400;
const uint16_t s_step_timeStamp_diff_min = 150;
const uint16_t s_step_threshold = 550;
const AccelSamplingRate s_acelSamplingRate = ACCEL_SAMPLING_10HZ;

static void update_step(int steps);
static void accel_data_handler(AccelData *data, uint32_t num_samples);



