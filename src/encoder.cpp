#include "encoder.hpp"
#include <Arduino.h>
#include <algorithm>

void Encoder::begin()
{
    pinMode(left_button_pin, INPUT_PULLDOWN);
    pinMode(center_button_pin, INPUT_PULLDOWN);
    pinMode(right_button_pin, INPUT_PULLDOWN);
}

// LISTENING, BRIGHTNESS, SAMPLE_AVERAGE, BPM_ON, O2_ON, FINALIZE
void Encoder::event_loop_tick(EncoderSettings biometry_settings)
{
    auto left = digitalRead(left_button_pin);
    auto center = digitalRead(center_button_pin);
    // auto right = digitalRead(right_button_pin);
    
    auto temp_left = left;
    auto temp_center = center;

    while (temp_left || temp_center)
    {
        Serial.printf("%d, %d\n", temp_left, temp_center);
        delay(50);
        temp_left = digitalRead(left_button_pin);
        temp_center = digitalRead(center_button_pin);
        // right = digitalRead(right_button_pin);
    }
    
    int direction;
    if (left)
    {
        direction = -1;
    }
    else if (center) {
        direction = 1;
    }
    else {
        direction = 0;
    }

    auto next = left && center;

    if (next) {
        left = 0;
        center = 0;
        direction = 0;
    }

    switch (state)
    {
    case LISTENING:
        if (next)
        {
            settings = biometry_settings;
            auto val = settings.brightness;
            current_setting.label = "Brightness";
            current_setting.use_value = true;
            current_setting.value = val;
            state = EncoderState::BRIGHTNESS;
        }   
        break;
    case BRIGHTNESS:
        settings.brightness = std::max(0, std::min(settings.brightness + direction * 5, 255));
        current_setting.value = settings.brightness;
        if (next)
        {
            std::vector<std::string> options = {"2", "4", "8", "16", "32"};
            auto it = std::find(options.begin(), options.end(), std::to_string(settings.sample_average));
            current_setting.label =  "Sample average";
            current_setting.options = options;
            current_setting.use_value = false;
            current_setting.active_option = std::distance(options.begin(), it);
            state = EncoderState::SAMPLE_AVERAGE;
        }
        break;
    case SAMPLE_AVERAGE:
        current_setting.active_option = (current_setting.active_option + direction) % (current_setting.options.size());
        settings.sample_average = std::stoi(current_setting.options[current_setting.active_option]);

        if (next) {
            std::vector<std::string> options = {"on", "off"};
            current_setting.label = "BPM status";
            current_setting.options = options;
            current_setting.use_value = false;
            current_setting.active_option = settings.bpm_on ? 0 : 1;
            state = EncoderState::BPM_ON;
        }

        break;
    case BPM_ON:
        current_setting.active_option = (current_setting.active_option + direction) % (current_setting.options.size());
        settings.bpm_on = current_setting.active_option == 0 ? 1 : 0;
        if (next) {
            std::vector<std::string> options = {"on", "off"};
            current_setting.label = "O2 status";
            current_setting.options = options;
            current_setting.use_value = false;
            current_setting.active_option = settings.o2_on ? 0 : 1;
            state = EncoderState::O2_ON;
        }
        break;
    case O2_ON:
        current_setting.active_option = (current_setting.active_option + direction) % (current_setting.options.size()); 
        settings.o2_on = current_setting.active_option == 0 ? 1 : 0;
        if (next) {
            state = EncoderState::FINALIZE;
        }
        break;
    case FINALIZE:
        break;
    default:
        Serial.println("error: unkown state in encoder");
        break;
    }
}

bool Encoder::has_result()
{
    return state == EncoderState::FINALIZE;
}

void Encoder::reset() {
    state = EncoderState::LISTENING;
}

EncoderSettings Encoder::get_result()
{
     state = EncoderState::LISTENING;
     return settings;
}

Setting Encoder::get_current_setting() {
    return current_setting;
}

bool Encoder::is_configuration_in_progress() {
    return state != EncoderState::LISTENING;
}