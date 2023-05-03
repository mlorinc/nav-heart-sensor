#include "biometry.hpp"

void Biometry::wait_for_finger()
{
    if (!has_finger())
    {
        return;
    }

    state = State::CALIBRATING_INIT;
}

bool Biometry::has_finger()
{
    return beat_sensor.getIR() >= 7000;
}

void Biometry::calibrate()
{
    if (take_samples())
    {
        maxim_heart_rate_and_oxygen_saturation(ir_buffer, buffer_length, red_buffer, &spo2, &validSPO2, &bpm, &valid_bpm);
        state = State::MEASURING_POP;
    }
}

void Biometry::measure()
{
    if (take_samples())
    {
        // After gathering 25 new samples recalculate HR and SP02
        maxim_heart_rate_and_oxygen_saturation(ir_buffer, buffer_length, red_buffer, &spo2, &validSPO2, &bpm, &valid_bpm);
        state = State::MEASURING_POP;
    }
}

bool Biometry::take_samples()
{
    Serial.println(iterator);
    if (iterator >= buffer_length)
    {
        return true;
    }

    if (beat_sensor.available() == false)
    {
        beat_sensor.check();
        return false;
    }

    red_buffer[iterator] = beat_sensor.getRed();
    ir_buffer[iterator] = beat_sensor.getIR();
    beat_sensor.nextSample();
    iterator++;
    return false;
}

void Biometry::pop_front(int count, int size)
{
    for (auto i = count; i < size; i++)
    {
        red_buffer[i - count] = red_buffer[i];
        ir_buffer[i - count] = ir_buffer[i];
    }
}

void Biometry::begin()
{
    // byte led_brightness = 0x0a; // Option: 0=Off to 255=50mA
    //  byte sample_average = 4;    // Option: 1, 2, 4, 8, 16, 32
    byte led_mode = 2;     // Option: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
    int sample_rate = 100; // Option: 50, 100, 200, 400, 800, 1000, 1600, 3200
    int pulse_width = 411; // Option: 69, 118, 215, 411
    int adc_range = 4096;  // Option: 2048, 4096, 8192, 16384

    beat_sensor.begin(Wire, I2C_SPEED_FAST);
    beat_sensor.setup(brightness, sample_average, led_mode, sample_rate, pulse_width, adc_range);
    beat_sensor.setPulseAmplitudeRed(0x0a);
    beat_sensor.setPulseAmplitudeGreen(0);
}

void Biometry::event_loop_tick()
{
    switch (state)
    {
    case State::WAITING_FOR_FINGER:
        wait_for_finger();
        break;
    case State::CALIBRATING_INIT:
        iterator = 0;
        state = State::CALIBRATING;
        break;
    case State::CALIBRATING:
        calibrate();
        break;
    case State::MEASURING_POP:
        pop_front(25, buffer_length);
        state = State::MEASURING_INIT;
        break;
    case State::MEASURING_INIT:
        iterator = 75;
        state = State::MEASURING;
        break;
    case State::MEASURING:
        measure();
        break;
    default:
        Serial.println("error: unknown state");
        for (;;)
        {
        }
        break;
    }

    if (!has_finger())
    {
        state = State::WAITING_FOR_FINGER;
    }
}

bool Biometry::has_data()
{
    return state == State::MEASURING && valid_bpm && validSPO2;
}

decltype(Biometry::bpm) Biometry::get_bpm()
{
    return bpm;
}

decltype(Biometry::spo2) Biometry::get_o2()
{
    return spo2;
}

bool Biometry::has_progress()
{
    return state == State::MEASURING || state == State::CALIBRATING;
}

float Biometry::get_progress()
{
    if (state == State::MEASURING)
    {
        return (float)(iterator - 75) / (float)(buffer_length - 75);
    }
    else if (state == State::CALIBRATING)
    {
        return (float)(iterator) / (float)(buffer_length);
    }
    else
    {
        return -1;
    }
}

State Biometry::get_state()
{
    return state;
}

decltype(Biometry::brightness) Biometry::get_brightness()
{
    return brightness;
}

void Biometry::set_brightness(decltype(Biometry::brightness) value)
{
    brightness = value;
    beat_sensor.setPulseAmplitudeRed(value);
}

decltype(Biometry::sample_average) Biometry::get_sample_average()
{
    return sample_average;
}

void Biometry::set_sample_average(decltype(Biometry::sample_average) value)
{
    sample_average = value;
}

void Biometry::reset() {
    state = State::WAITING_FOR_FINGER;
}