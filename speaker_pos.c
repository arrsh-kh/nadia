#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

// Environment dimensions
typedef struct {
    float width;
    float length;
    float depth;
} Environment;

// Speaker specifications
typedef struct {
    char model[64];  // Increased size for longer model names
    float yaw_deg;
    float spl_peak_dB;
    float bandwidth_low_Hz;
    float bandwidth_high_Hz;
    float horiz_dispersion_deg;
    float vert_dispersion_deg;
    float max_throw_m;
    float weight_kg;
    float box_height;
    int num_lf;
    int num_mf;
    int num_hf;
} Speaker;

// Structure to hold speaker position and configuration
typedef struct {
    int speaker_id;
    float height;
    float pitch_deg;
    float coverage_start;
    float coverage_end;
    float coverage_width;
    float x_position;
    int array_id;
} SpeakerConfig;

// Function prototypes
void get_ellipse_axes(float height, float tilt_deg, float v_disp_deg, 
                     float h_disp_deg, float *major_axis, float *minor_axis);
float compute_tilt_for_end_distance(float height, float vert_disp_deg, float target_distance);
void print_venue_layout(const Environment *env, float step, int num_arrays);
void calculate_array_configuration(const Environment *env, const Speaker *speaker, 
                                 int array_id, float array_x, float base_height, 
                                 float target_floor, SpeakerConfig configs[], int *config_count);
void print_speaker_summary(const SpeakerConfig configs[], int config_count);
bool validate_inputs(const Environment *env, const Speaker *speaker);

void get_ellipse_axes(
    float height,
    float tilt_deg,
    float v_disp_deg,
    float h_disp_deg,
    float *major_axis,
    float *minor_axis
) {
    // Input validation
    if (height <= 0 || major_axis == NULL || minor_axis == NULL) {
        if (major_axis) *major_axis = 0.0f;
        if (minor_axis) *minor_axis = 0.0f;
        return;
    }
    
    float alpha = tilt_deg * M_PI / 180.0f;
    float theta = v_disp_deg * M_PI / 180.0f;
    float phi = h_disp_deg * M_PI / 180.0f;

    *major_axis = height * (tanf(alpha + theta/2.0f) - tanf(alpha - theta/2.0f));
    *minor_axis = 2.0f * height * tanf(phi / 2.0f);
    
    // Ensure non-negative values
    *major_axis = fmaxf(0.0f, *major_axis);
    *minor_axis = fmaxf(0.0f, *minor_axis);
}

float compute_tilt_for_end_distance(float height, float vert_disp_deg, float target_distance) {
    if (height <= 0 || target_distance <= 0 || vert_disp_deg <= 0) {
        return 0.0f;
    }
    
    float theta = vert_disp_deg * M_PI / 180.0f;
    float alpha = atanf(target_distance / height) - theta / 2.0f;
    return alpha * 180.0f / M_PI;
}

bool validate_inputs(const Environment *env, const Speaker *speaker) {
    if (!env || !speaker) {
        printf("Error: Null pointer passed to validate_inputs\n");
        return false;
    }
    
    if (env->width <= 0 || env->length <= 0 || env->depth <= 0) {
        printf("Error: Environment dimensions must be positive\n");
        return false;
    }
    
    if (speaker->box_height <= 0 || speaker->vert_dispersion_deg <= 0 || 
        speaker->horiz_dispersion_deg <= 0) {
        printf("Error: Speaker parameters must be positive\n");
        return false;
    }
    
    return true;
}

void print_venue_layout(const Environment *env, float step, int num_arrays) {
    printf("\n== Venue Setup ==\n");
    printf("Venue: %.1f m (W) × %.1f m (L) × %.1f m (D)\n", 
           env->width, env->length, env->depth);
    printf("Number of arrays: %d\n", num_arrays);
    printf("Array spacing: %.2f m\n\n", step);
    
    // ASCII visualization
    printf("== Main Arrays Layout ==\n");
    for (int row = 0; row < 8; row++) {
        printf("Stage  ");
        for (int a = 0; a < num_arrays; a++) {
            printf("   |   ");
        }
        printf("\n");
    }
    
    printf("       ");
    for (int a = 0; a < num_arrays; a++) {
        printf("[A%d]  ", a + 1);
    }
    printf("\n");
    
    // Show audience area
    for (int row = 0; row < 5; row++) {
        printf("  ~    ");
        for (int a = 0; a < num_arrays; a++) {
            printf("  ~   ");
        }
        printf("  (Audience)\n");
    }
    printf("\n");
}

void calculate_array_configuration(const Environment *env, const Speaker *speaker, 
                                 int array_id, float array_x, float base_height, 
                                 float target_floor, SpeakerConfig configs[], int *config_count) {
    float target_end = env->depth;
    int speaker_count = 0;
    
    printf("=== Array %d Configuration (x = %.2f m) ===\n", array_id, array_x);
    printf("Speaker | Height | Pitch  | Coverage Range | Width | Notes\n");
    printf("--------|--------|--------|----------------|-------|-------\n");
    
    for (int i = 0; ; i++) {
        float current_height = base_height - i * speaker->box_height;
        if (current_height <= target_floor) {
            printf("Stopped: Minimum height reached\n");
            break;
        }

        float pitch = compute_tilt_for_end_distance(current_height, speaker->vert_dispersion_deg, target_end);
        
        // Physical calculations
        float pitch_rad = pitch * M_PI / 180.0f;
        float vertical_drop = speaker->box_height * sinf(pitch_rad);
        float mouth_height = current_height - vertical_drop;

        float major = 0.0f, minor = 0.0f;
        get_ellipse_axes(mouth_height, pitch, speaker->vert_dispersion_deg, 
                        speaker->horiz_dispersion_deg, &major, &minor);
        
        float target_start = target_end - major;
        if (target_start <= 0.0f) {
            printf("Stopped: Coverage would extend beyond stage\n");
            break;
        }

        // Store configuration
        if (*config_count < 100) {  // Assuming max 100 speakers
            SpeakerConfig *config = &configs[*config_count];
            config->speaker_id = i + 1;
            config->height = current_height;
            config->pitch_deg = pitch;
            config->coverage_start = target_start;
            config->coverage_end = target_end;
            config->coverage_width = minor;
            config->x_position = array_x;
            config->array_id = array_id;
            (*config_count)++;
        }

        // Determine any warnings
        const char *notes = "";
        if (pitch > 45.0f) notes = "High angle!";
        else if (pitch < -10.0f) notes = "Upward tilt";
        else if (major < 2.0f) notes = "Short throw";

        printf("   %2d   | %6.2f | %6.2f | %5.1f → %5.1f | %5.2f | %s\n",
               i + 1, current_height, pitch, target_start, target_end, minor, notes);

        target_end = target_start;
        speaker_count++;
        
        // Safety limit
        if (speaker_count >= 20) {
            printf("Stopped: Maximum speakers per array reached\n");
            break;
        }
    }
    printf("Total speakers in array: %d\n\n", speaker_count);
}

void print_speaker_summary(const SpeakerConfig configs[], int config_count) {
    if (config_count == 0) return;
    
    printf("=== System Summary ===\n");
    printf("Total speakers configured: %d\n", config_count);
    
    // Calculate totals
    float total_coverage = 0.0f;
    float min_height = configs[0].height;
    float max_height = configs[0].height;
    
    for (int i = 0; i < config_count; i++) {
        total_coverage += (configs[i].coverage_end - configs[i].coverage_start);
        if (configs[i].height < min_height) min_height = configs[i].height;
        if (configs[i].height > max_height) max_height = configs[i].height;
    }
    
    printf("Height range: %.2f m to %.2f m\n", min_height, max_height);
    printf("Total coverage distance: %.1f m\n", total_coverage);
    printf("\n");
}

int main() {
    // Test data
    Environment hall = { 
        .width = 30.0f, 
        .length = 15.0f, 
        .depth = 50.0f 
    };

    Speaker mk2 = {
        .model = "L-Acoustics K2",
        .yaw_deg = 0.0f,
        .spl_peak_dB = 147.0f,
        .bandwidth_low_Hz = 35.0f,
        .bandwidth_high_Hz = 20000.0f,
        .horiz_dispersion_deg = 90.0f,
        .vert_dispersion_deg = 10.0f,
        .max_throw_m = 25.0f,
        .weight_kg = 56.0f,
        .box_height = 0.5f,
        .num_lf = 2, 
        .num_mf = 4, 
        .num_hf = 2
    };

    // Input validation
    if (!validate_inputs(&hall, &mk2)) {
        return 1;
    }

    // Configuration parameters
    const float base_height = hall.length - 1.0f;
    const float target_floor = 0.1f;
    const float overlap_factor = 0.5f;

    // Calculate array spacing and positions
    float test_pitch = compute_tilt_for_end_distance(base_height, mk2.vert_dispersion_deg, hall.depth);
    float major_axis, top_minor;
    get_ellipse_axes(base_height, test_pitch, mk2.vert_dispersion_deg, mk2.horiz_dispersion_deg, 
                    &major_axis, &top_minor);

    float step = top_minor * overlap_factor;
    int num_arrays = (int)ceilf(hall.width / step);
    float start_x = (hall.width - (num_arrays - 1) * step) / 2.0f;

    // Print venue layout
    print_venue_layout(&hall, step, num_arrays);

    // Configure each array
    SpeakerConfig all_configs[100];  // Max 100 speakers total
    int total_config_count = 0;

    for (int a = 0; a < num_arrays; a++) {
        float array_x = start_x + a * step;
        calculate_array_configuration(&hall, &mk2, a + 1, array_x, base_height, 
                                    target_floor, all_configs, &total_config_count);
    }

    // Print summary
    print_speaker_summary(all_configs, total_config_count);

    // Fill speakers section
    printf("=== Fill/Delay Arrays ===\n");
    if (num_arrays >= 2) {
        float fill_x1 = start_x - step / 2;
        float fill_x2 = start_x + (num_arrays - 1) * step + step / 2;
        
        if (fill_x1 > 0) {
            printf("Fill Left  @ x = %6.2f m → Covers 10–25 m (near-field)\n", fill_x1);
        }
        if (fill_x2 < hall.width) {
            printf("Fill Right @ x = %6.2f m → Covers 10–25 m (near-field)\n", fill_x2);
        }
    }
    printf("\nRecommendation: Add delay speakers at 25-35m for far-field coverage\n");

    return 0;
}
