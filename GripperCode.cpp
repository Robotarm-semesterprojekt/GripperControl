#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "pico/multicore.h"
#include <array>

volatile int counter = 0;
volatile bool maxCurrentExceeded = false;

typedef struct {
	uint adc0;
	uint adc1;
} ADCValues;

ADCValues readVoltage();
int GreyCode(int lastIndex);
float checkCurrent();

const float maxCurrent = 0.2f; // Max current in amps

void Core1();

void Core1() {
    
    int grayCodeOrder[4][2] = {
	{0, 0},
	{0, 1},
	{1, 1},
	{1, 0}
	};

	ADCValues initRead = readVoltage();
	int lastIndex = 0;
	for (int i = 0; i < 4; i++)
	{
		if (initRead.adc0 == grayCodeOrder[i][0] && initRead.adc1 == grayCodeOrder[i][1])
		{
			lastIndex = i;
		}
	}
    
    std::array<float, 10> currentReadings{};
    size_t next =0;

    while (true)
    {
        lastIndex = GreyCode(lastIndex);
        currentReadings[next] = checkCurrent();
        next = (next + 1) % currentReadings.size();

        float sumCurrent = 0.0f;
        for (float reading : currentReadings) {
            sumCurrent += reading;
        }
        float averageCurrent = sumCurrent / currentReadings.size();
        if (averageCurrent > maxCurrent) {
            maxCurrentExceeded = true;
        } else {
            maxCurrentExceeded = false;
        }
        if (next % 5 == 0)
        {
           // printf("Average Current: %.3f A\n", averageCurrent);
        }

        sleep_ms(100);
    }
}

int GreyCode(int lastIndex) {
    int grayCodeOrder[4][2] = {
    {0, 0},
    {0, 1},
    {1, 1},
    {1, 0}
    };

    int currentIndex;

    ADCValues values = readVoltage();
    for (int i = 0; i < 4; i++)
    {
        if (values.adc0 == grayCodeOrder[i][0] && values.adc1 == grayCodeOrder[i][1])
        {
            currentIndex = i;
        }
    }

    int rotation = currentIndex - lastIndex;

    if (rotation == 1 || rotation == -3)
    {
        counter++;
    }
    else if (rotation == -1 || rotation == 3)
    {
        counter--;
    }

    return currentIndex;
    //cout << "Counter: " << counter << endl;
    
    //printf("Counter: %d\n", counter);
    sleep_ms(10);


}
ADCValues readVoltage()
{
	ADCValues result;

	adc_select_input(1);        // GPIO 27
	uint raw0 = adc_read();
	result.adc0 = (raw0 < 2600) ? 0 : 1;
    //printf("ADC0: %d -> %d\n", raw0, result.adc0);

	adc_select_input(2);        // GPIO 28
	uint raw1 = adc_read();
	result.adc1 = (raw1 < 2600) ? 0 : 1;
   // printf("ADC1: %d -> %d\n", raw1, result.adc1);

	return result;
}

const int Hbridgepin1 = 21;
const int Hbridgepin2 = 19;
const int HbridgepinOnOff = 22;

const int LDRPin = 20; // On-board LED

const int ADCpin = 26; // ADC0

const int switchIntervalMs = 10000; // 10 seconds



void openGripper();
void closeGripper();

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

void Pause(int timeMS) {
    gpio_put(Hbridgepin1, 0);
    gpio_put(Hbridgepin2, 0);
    gpio_put(HbridgepinOnOff, 0); // OFF
    sleep_ms(timeMS);
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

void PWMUpRamping(int max_speed, int stop) {
    for (int speed = 0; speed <= max_speed; speed += 5) {
        SetPWMDutyCycle(speed);
        sleep_ms(10);
       
       if (counter == stop)
       {
        break;
       }
    }
}

void PWMDownRamping(int max_speed) {
    for (int speed = max_speed; speed >= 0; speed -= 5) {
        SetPWMDutyCycle(speed);
       
        sleep_ms(10);
    }
}

void DriveMotor(int direction, int speed, int encoderStop, bool encoderHigh) {
    SetPWMDutyCycle(0);
    sleep_ms(50);

    HbridgeControl(direction);

    PWMUpRamping(speed, encoderStop);
    if (encoderHigh == true) 
    {
        while (counter < encoderStop)
        {
            sleep_ms(10);
            
        }
    }
    else 
    {
      while (counter > encoderStop && !maxCurrentExceeded)
      {
            sleep_ms(10);
            
        }
        /*if (!encoderHigh)
        {
            sleep_ms(200);
        }*/
        
     if (maxCurrentExceeded)
     {
     	printf("Current Exceeded");
     	Pause(5000);
     }
    }
    //PWMDownRamping(speed);

    Pause(100);
}

float checkCurrent() {
    adc_select_input(0); // Select ADC channel 0 (GPIO 26)
    uint adcValue = adc_read();
    float current = adcValue * 3.3f / 4095.0f; // Since shunt resistor is 1 ohm, voltage across it equals current in amps
    /*if (current > 0.13f)
    {
        printf("Current: %.2f A\n", current);
    }*/
    return current;
    
}

void openGripper() 
{
    DriveMotor(1, 255, 2, true);
}

void closeGripper()
{
    DriveMotor(-1, 255, 0, false);
}


int main () {
    stdio_init_all();
    sleep_ms(2000);

    gpio_init(0);
    gpio_set_dir(0, GPIO_OUT);
    gpio_put(0,1); // Turn on the on-board LED¨


	adc_init(); 
    adc_gpio_init(26); //
	adc_gpio_init(27); //z pin26
	adc_gpio_init(28); //Intializes pin27
	

    sleep_ms(3000);

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

    multicore_launch_core1(Core1);


    while (true) {
        int c = getchar_timeout_us(0);

        if (c != PICO_ERROR_TIMEOUT) {

            if (c == '1') {
                closeGripper();
                printf("IN_POSITION_CLOSED\n");
                
            }

            else if (c == '0') {
                openGripper();
                printf("IN_POSITION_OPEN\n");
            }
           else if (c == '3') {
                DriveMotor(-1, 255, counter-1, false);
                printf("IN_POSITION_CLOSED\n");
                
            }
        }

        sleep_ms(1);

    }
}
