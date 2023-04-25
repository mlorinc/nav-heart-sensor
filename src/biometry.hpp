#pragma once
#include <SPI.h>
#include <Wire.h>
#include <MAX30105.h>
#include <spo2_algorithm.h>

enum State
{
    WAITING_FOR_FINGER,
    CALIBRATING_INIT,
    CALIBRATING,
    MEASURING_INIT,
    MEASURING,
    MEASURING_POP
};

class Biometry
{
private:
    MAX30105 &beat_sensor;
    State state;
    uint32_t ir_buffer[100];
    uint32_t red_buffer[100];
    int32_t buffer_length = 100;
    int32_t spo2;
    int8_t validSPO2;
    int32_t bpm;
    int8_t valid_bpm;
    uint8_t brightness = 0x0a;
    uint8_t sample_average = 4;
    int iterator;

    void wait_for_finger();
    void calibrate();
    void measure();
    bool take_samples();
    void pop_front(int count, int size);

public:
    Biometry(MAX30105 &&beat_sensor) : state(State::WAITING_FOR_FINGER), beat_sensor(beat_sensor) {}
    void reset();
    void begin();
    void event_loop_tick();
    bool has_data();
    bool has_progress();
    bool has_finger();
    float get_progress();
    State get_state();
    decltype(brightness) get_brightness();
    void set_brightness(decltype(brightness) value);
    decltype(sample_average) get_sample_average();
    void set_sample_average(decltype(sample_average) value);
    decltype(bpm) get_bpm();
    decltype(spo2) get_o2();
};
