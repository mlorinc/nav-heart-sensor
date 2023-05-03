#pragma once
#include <vector>
#include <stdint.h>
#include <string>

constexpr int8_t LEFT_BUTTON_PIN = 34;
constexpr int8_t CENTER_BUTTON_PIN = 36;
constexpr int8_t RIGHT_BUTTON_PIN = 35;

struct EncoderSettings {
    bool bpm_on;
    bool o2_on;
    uint16_t brightness;
    uint8_t sample_average;
};

struct Setting {
    std::string label;
    std::vector<std::string> options;
    int value;
    bool use_value;
    int active_option;
};

enum EncoderState {
    LISTENING, BRIGHTNESS, SAMPLE_AVERAGE, BPM_ON, O2_ON, FINALIZE
};

class Encoder {
    private:
        const int8_t left_button_pin, center_button_pin, right_button_pin;
        int last_encoder_state;
        int value;
        EncoderSettings settings;
        EncoderState state;
        Setting current_setting;

    public:
        Encoder(int8_t left, int8_t center, int8_t right) : 
            left_button_pin(left), center_button_pin(center), right_button_pin(right), state(EncoderState::LISTENING) {}
        void begin();
        void event_loop_tick(EncoderSettings biometry_settings);
        bool has_result();
        bool is_configuration_in_progress();
        void reset();
        EncoderSettings get_result();
        Setting get_current_setting();
};