from machine import ADC, Pin
import time

# Initialize ADC on pin 26 (ADC0)
adc1 = ADC(Pin(26))
adc2 = ADC(Pin(27))
counter = 0



# Function to read voltage
def read_voltage():
    # ADC.read_u16() returns a value from 0 to 65535
    if (adc1.read_u16() < 36000):
        raw1 = 0
    else:
        raw1 = 1
    
    if (adc2.read_u16() < 49500):
        raw2 = 0
    else:
        raw2 = 1
    
    
    return raw1,raw2

time.sleep(1)
grayCodeOrder = [[0,0],
                 [0,1],
                 [1,1],
                 [1,0]]
initRead1, initRead2 = read_voltage()
lastIndex = 0
for i in range(4):
    if (initRead1 == grayCodeOrder[i][0] and initRead2 == grayCodeOrder[i][1]):
        lastIndex = i

# Main loop
while True:
    v1, v2 = read_voltage()
    currentIndex = 0
    for i in range(4):
        if (v1 == grayCodeOrder[i][0] and v2 == grayCodeOrder[i][1]):
            currentIndex = i
            
    rotation = currentIndex-lastIndex
    
    if (rotation == 1 or rotation == -3):
        counter += 1
        
    elif(rotation == -1 or rotation == 3):
        counter -= 1
            
   
    lastIndex = currentIndex
    print(counter)
    
 
    time.sleep(0.2)  # read every 1 second
