#pragma once
#include <vector>
#include <stdint.h>
#include <string>

constexpr int8_t ENCODER_SW = 34;
constexpr int8_t ENCODER_DT = 36;
constexpr int8_t ENCODER_CLK = 39;

struct EncoderSettings {
    bool bpm_on;
    bool o2_on;
    uint8_t brightness;
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
        const int8_t sw;
        const int8_t dt;
        const int8_t clk;
        int last_encoder_state;
        int value;
        EncoderSettings settings;
        EncoderState state;
        Setting current_setting;

    public:
        Encoder(int8_t sw, int8_t dt, int8_t clk) : sw(sw), dt(dt), clk(clk), state(EncoderState::LISTENING) {}
        void begin();
        void event_loop_tick(EncoderSettings biometry_settings);
        bool has_result();
        bool is_configuration_in_progress();
        void reset();
        EncoderSettings get_result();
        Setting get_current_setting();
};