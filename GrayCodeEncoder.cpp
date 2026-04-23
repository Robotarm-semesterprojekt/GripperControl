#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

using namespace std;

typedef struct {
	uint16_t adc0;
	uint16_t adc1;
} ADCValues;

ADCValues readVoltage();

int main() 
{
	stdio_init_all();
	sleep_ms(2000);

	adc_init(); 
	adc_gpio_init(26); //Intializes pin26
	adc_gpio_init(27); //Intializes pin27
	

	int counter = 0;
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
	int currentIndex = 0;
	while (true)
	{
		ADCValues values = readVoltage();
		currentIndex = 0;
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

		lastIndex = currentIndex;

		sleep_ms(200);
	
	}

	return 0;
}

ADCValues readVoltage()
{
	ADCValues result;

	adc_select_input(0);        // GPIO 26
	uint16_t raw0 = adc_read();
	result.adc0 = (raw0 < 3000) ? 0 : 1;


	adc_select_input(1);        // GPIO 27
	uint16_t raw1 = adc_read();
	result.adc1 = (raw1 < 3000) ? 0 : 1;

	return result;
}