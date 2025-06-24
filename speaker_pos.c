#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

// Environment dimensions
typedef struct {
    float width;
    float length;
    float depth;
} environment;

// Speaker types
typedef enum {
    SPEAKER_TOP,
    SPEAKER_SUB,
    SPEAKER_MONITOR,
    SPEAKER_FILL,
    SPEAKER_ARRAY
} SpeakerType;

// Speaker specifications
typedef struct {
    char model[32];
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

void get_ellipse_axes(
    float height,
    float tilt_deg,
    float v_disp_deg,
    float h_disp_deg,
    float *major_axis,
    float *minor_axis
) {
    float alpha = tilt_deg * M_PI / 180.0f;
    float theta = v_disp_deg * M_PI / 180.0f;
    float phi   = h_disp_deg * M_PI / 180.0f;

    if (major_axis != NULL) {
        *major_axis = height * (tanf(alpha + theta/2) - tanf(alpha - theta/2));
    }
    if (minor_axis != NULL) {
        *minor_axis = 2.0f * height * tanf(phi / 2.0f);
    }
}

float compute_tilt_for_end_distance(float height, float vert_disp_deg, float target_distance) {
    float theta = vert_disp_deg * M_PI / 180.0f;
    float alpha = atanf(target_distance / height) - theta / 2.0f;
    return alpha * 180.0f / M_PI;
}

int main() {
    environment hall = { .width = 30.0f, .length = 15.0f, .depth = 50.0f };

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
        .num_lf = 2, .num_mf = 4, .num_hf = 2
    };

    float base_height = hall.length - 1.0f;
    float speaker_spacing = mk2.box_height;
    float target_floor = 0.1f;
    float overlap_factor = 0.5f;
    float top_minor = 0.0f;

    float test_major = 0.0f;
    float test_pitch = compute_tilt_for_end_distance(base_height, mk2.vert_dispersion_deg, hall.depth);
    get_ellipse_axes(base_height, test_pitch, mk2.vert_dispersion_deg, mk2.horiz_dispersion_deg, &test_major, &top_minor);

    float step = top_minor * overlap_factor;
    int num_arrays = (int)ceilf(hall.width / step);
    float start_x = (hall.width - (num_arrays - 1) * step) / 2.0f;

    printf("\n== Venue Setup ==\n");
    printf("Venue Width: %.2f m, Depth: %.2f m\n", hall.width, hall.depth);
    printf("\n== Main Arrays (%.2f m spacing) ==\n\n", step);

    for (int row = 0; row < 10; row++) {
        for (int a = 0; a < num_arrays; a++) {
            printf("     |");
        }
        printf("\n");
    }
    for (int a = 0; a < num_arrays; a++) {
        printf("[Array %d]", a + 1);
    }
    printf("\n\n");

    for (int a = 0; a < num_arrays; a++) {
        float array_x = start_x + a * step;
        float target_end = hall.depth;

        printf("Array %d @ x = %.2f m\n", a + 1, array_x);

        for (int i = 0; ; i++) {
            float current_height = base_height - i * speaker_spacing;
            if (current_height <= target_floor) break;

            float pitch = compute_tilt_for_end_distance(current_height, mk2.vert_dispersion_deg, target_end);
            float pitch_rad = pitch * M_PI / 180.0f;
            float vertical_drop = mk2.box_height * sinf(pitch_rad);
            float backshift     = mk2.box_height * (1.0f - cosf(pitch_rad));
            float mouth_height  = current_height - vertical_drop;

            float major = 0.0f, minor = 0.0f;
            get_ellipse_axes(mouth_height, pitch, mk2.vert_dispersion_deg, mk2.horiz_dispersion_deg, &major, &minor);
            float target_start = target_end - major;
            if (target_start <= 0.0f) break;

            printf("  Speaker %d - Height: %.2f m, Pitch: %.2f°, Covers: %.2f → %.2f m, Width: %.2f m\n",
                   i + 1, current_height, pitch, target_start, target_end, minor);

            target_end = target_start;
        }
        printf("\n");
    }

    printf("== Fill Arrays ==\n\n");
    float fill_x1 = start_x + step / 2;
    float fill_x2 = start_x + (num_arrays - 1) * step - step / 2;

    printf("Fill 1 @ x = %.2f m → Covers 10–25 m\n", fill_x1);
    printf("Fill 2 @ x = %.2f m → Covers 10–25 m\n", fill_x2);

    return 0;
}
