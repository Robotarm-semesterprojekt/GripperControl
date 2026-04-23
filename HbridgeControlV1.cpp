/*
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

int main() {
    stdio_init_all();
    sleep_ms(2000);

    gpio_init(20);
    gpio_set_dir(20, GPIO_OUT);
    gpio_put(20, 1); // Turn on the LED

    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);

    while (true) {
        gpio_put(20, 1); // Turn on the LED
        uint16_t result = adc_read();
        float voltage = result * 3.3f / 4095.0f;
        printf("ADC raw: %d, Voltage: %.2f V\n", result, voltage);
        sleep_ms(500);
    }
}

*/

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"



const int Hbridgepin1 = 17;
const int Hbridgepin2 = 18;
const int HbridgepinOnOff = 16;

const int LDRPin = 20; // On-board LED

const int ADCpin = 26; // ADC0

const int switchIntervalMs = 5000; // 10 seconds
const int printIntervalMs = 10000; // 10 seconds

void HbridgeControl(int direction) {
    if (direction == 1) {
        gpio_put(Hbridgepin1, 1);
        gpio_put(Hbridgepin2, 0);
    } 
    else if (direction == -1) {
        gpio_put(Hbridgepin1, 0);
        gpio_put(Hbridgepin2, 1);
    } 
    else {
        gpio_put(Hbridgepin1, 0);
        gpio_put(Hbridgepin2, 0);
    }
}

void Pause() {
    gpio_put(Hbridgepin1, 0);
    gpio_put(Hbridgepin2, 0);
    gpio_put(HbridgepinOnOff, 0); // OFF
    sleep_ms(switchIntervalMs);
}


void InitPWM() {
    gpio_set_function(HbridgepinOnOff, GPIO_FUNC_PWM);

    uint slice = pwm_gpio_to_slice_num(HbridgepinOnOff);
    pwm_set_wrap(slice, 255);
    pwm_set_enabled(slice, true);
}

void SetPWMDutyCycle(int duty_cycle) {
    uint slice = pwm_gpio_to_slice_num(HbridgepinOnOff);
    uint channel = pwm_gpio_to_channel(HbridgepinOnOff);
    pwm_set_chan_level(slice, channel, duty_cycle);
}

void PWMUpRamping(int max_speed) {
    for (int speed = 0; speed <= max_speed; speed += 5) {
        SetPWMDutyCycle(speed);
        sleep_ms(100);
    }
}

void PWMDownRamping(int max_speed) {
    for (int speed = max_speed; speed >= 0; speed -= 5) {
        SetPWMDutyCycle(speed);
        sleep_ms(100);
    }
}

void DriveMotor(int direction, int speed) {
    SetPWMDutyCycle(0);
    sleep_ms(50);

    HbridgeControl(direction);

    PWMUpRamping(speed);
    sleep_ms(2000);
    PWMDownRamping(speed);

    Pause();
}



void RunMotorPWMTest(int PwmSpeed) {
    DriveMotor(1, PwmSpeed); // Forward
    sleep_ms(switchIntervalMs);
    DriveMotor(-1, PwmSpeed); // Reverse
    sleep_ms(switchIntervalMs);
}

int main () {
    stdio_init_all();
    sleep_ms(2000);

    gpio_init(Hbridgepin1);
    gpio_init(Hbridgepin2);
    gpio_init(HbridgepinOnOff);

    gpio_set_dir(Hbridgepin1, GPIO_OUT);
    gpio_set_dir(Hbridgepin2, GPIO_OUT);
    gpio_set_dir(HbridgepinOnOff, GPIO_OUT);

    gpio_init(LDRPin);
    gpio_set_dir(LDRPin, GPIO_OUT);
    gpio_put(LDRPin, 1); 

    InitPWM();

    while (true) {
        DriveMotor(1, 255); // Forward at full speed
        sleep_ms(switchIntervalMs);
        DriveMotor(-1, 255); // Reverse at full speed
        sleep_ms(switchIntervalMs);
    }

    return 0;
}



