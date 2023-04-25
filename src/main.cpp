#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <MAX30105.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "display.hpp"
#include "biometry.hpp"
#include "encoder.hpp"

Adafruit_SSD1306 display(DISPLAY_WIDTH, DISPLAY_HEIGHT, MOSI_PIN, SCLK_PIN, DC_PIN, RST_PIN, CS_PIN);
MAX30105 beat_sensor;
Biometry biometry(std::move(beat_sensor));
Encoder encoder(ENCODER_SW, ENCODER_DT, ENCODER_CLK);

bool bpm_on = true;
bool o2_on = true;

void setup()
{
    Serial.begin(115200);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3d))
    {
        Serial.println(F("SSD1306 allocation error"));
        for (;;)
            ;
    }
    biometry.begin();
    encoder.begin();
    display.clearDisplay();
}

void display_measurement_biometry(bool status)
{
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.printf("BPM: %d\n", biometry.get_bpm());
    display.printf("O2: %d\n", biometry.get_o2());

    if (status) {
        display.setTextSize(1);
        display.setCursor(0, 56);

        int stop = 21 * biometry.get_progress();
        for (auto i = 0; i < stop; i++)
        {
            display.printf("#");
        }
        display.println();
    }
    display.display();
}

void display_calibration_status(bool status) {
    static std::string loading[] = {"|", "/", "\\"};
    static int pos = 0;

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(30, 5);
    display.printf("calibrating %s\n", loading[pos].c_str());

    if (status) {
        display.setCursor(30, 32);
        display.setTextSize(2);
        display.printf("%d %%\n", (int)(biometry.get_progress()*100));
    }

    display.display();
    pos = (pos + 1) % 3;
}

void display_setting(Setting &setting) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 5);
    display.printf("%s", setting.label.c_str());

    if (setting.use_value)
    {
        display.printf(": %d\n", setting.value);
    }
    else {
        for (size_t i = 0; i < setting.options.size(); i++)
        {
            if (i == setting.active_option)
            {
                display.printf("> %s\n", setting.options[i]);
            }
            else {
                display.printf("  %s\n", setting.options[i]);
            }
        } 
    }

    display.display();
}

void loop()
{

    EncoderSettings settings;
    settings.bpm_on = bpm_on;
    settings.brightness = biometry.get_brightness();
    settings.o2_on = o2_on;
    settings.sample_average = biometry.get_sample_average();

    encoder.event_loop_tick(settings);

    if (encoder.has_result())
    {
        auto result = encoder.get_result();
        biometry.set_brightness(result.brightness);
        biometry.set_sample_average(result.sample_average);
        bpm_on = result.bpm_on;
        o2_on = result.o2_on;
        encoder.reset();
    }
    else if (encoder.is_configuration_in_progress()) {
        auto config_setting = encoder.get_current_setting();
        display_setting(config_setting);
        biometry.reset();
        return;
    }

    biometry.event_loop_tick();


    if (!biometry.has_finger()) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(30, 5);
        display.println("Please place");
        display.setCursor(30, 15);
        display.println("your finger");
        display.display();
    }
    else if (biometry.get_state() == State::MEASURING && biometry.has_data())
    {
        display_measurement_biometry(true);
    }
    else if (biometry.get_state() == State::CALIBRATING)
    {
        display_calibration_status(true);
    }
    else if (biometry.get_state() == State::MEASURING_POP) {
        display_measurement_biometry(false);
    }
}
