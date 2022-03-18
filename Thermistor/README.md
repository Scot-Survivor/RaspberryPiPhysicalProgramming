# Thermistor Project
# ==========================
# Introduction
At the time of conducting this project, I was going through my A-Level Physics course, thus one of the required practicals 
is to use a thermistor, this [Physics and Maths tutor shows the brief](https://www.physicsandmathstutor.com/pdf-pages/?pdf=https%3A%2F%2Fpmt.physicsandmathstutor.com%2Fdownload%2FPhysics%2FA-level%2FNotes%2FEdexcel%2FPractical-Skills%2FCP%252012%2520-%2520Calibrate%2520a%2520Thermistor%2520in%2520a%2520Potential%2520Divider%2520Circuit%2520as%2520a%2520Thermostat.pdf)
at the current time of writing I am unable to be able to achieve the temperature range the brief states (0 to 100 degrees), 
as such I had to go with what I could create on my desk, which was room temperature, which is around 20 degrees, to 40C heated from my Pi's CPU. 

# Design
## Circuit Diagram
Overall Circuit design came from the Tutorial on Freenove:
![image](https://user-images.githubusercontent.com/40865296/159094267-a916b430-9e5d-45b4-a9cb-77c78cfa48a5.png)
The GPIO extension board, labelling my exact pins made this whole process much easier. 
The exact ADC I used was the ADS7830, the [spec sheet](https://www.ti.com/lit/ds/symlink/ads7830.pdf?ts=1647530844817&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FADS7830%253FkeyMatch%253DADS7830%2526tisearch%253Dsearch-everything%2526usecase%253DGPN)
became vital in debugging while I was trying to find out the maths. When conducting this project, I only wanted to use the tutorial, for the 
circuit diagrams to stop me from damaging my components by miss wiring them. Thus, the maths I had to use my own research for. 

## References
### Links
- [ADS7830 Datasheet](https://www.ti.com/lit/ds/symlink/ads7830.pdf?ts=1647530844817&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FADS7830%253FkeyMatch%253DADS7830%2526tisearch%253Dsearch-everything%2526usecase%253DGPN)
- [Thermistor Datasheet](https://en.wikipedia.org/wiki/Thermistor)
- [Thermistor Equations (Part 1)](https://e2e.ti.com/blogs_/archives/b/precisionhub/posts/it-s-in-the-math-how-to-convert-adc-code-to-a-voltage-part-1)
- [Thermistor Equations (Part 2)](https://e2e.ti.com/blogs_/archives/b/precisionhub/posts/it-39-s-in-the-math-how-to-convert-an-adc-code-to-a-voltage-part-2)

### Images of Physical Circuit
![IMG_5753](https://user-images.githubusercontent.com/40865296/159095016-b25625d7-f759-4eb7-a87a-3e6e0a7b1040.jpg)


## Maths and Calculations
The most interesting part of this project was the maths, as I had to use my own research to find the correct values for the thermistor. 
Turns out to get the value of the temperature, I had to find several constants from spec sheets. In my code these are called:
T0, B, R0. Now, R0 was referencing that resistor we see in the above image. A 10k Ohm resistor. Thus, R0 = 10000. 
B constant means "Beta", this is an important value to get right, as it is a scaling factor for the temperature. Through the spec sheet 
for the resistor, I found this to be a value of 3950. Thus, B = 3950. Lastly, T0 is the temperature at which the thermistor is at its 
lowest resistance, which is around 25 degrees. Thus, T0 = 25.
The formula for the temperature is:
- ![Equation for temperature](https://latex.codecogs.com/svg.image?T_%7B1%7D=%20%5Cfrac%7B1%7D%7B(%5Cfrac%7B1%7D%7BT_%7B0%7D%7D%20&plus;%20%5Cfrac%7Blog(%5Cfrac%7BRt%7D%7BR0%7D)%7D%7BB%7D)%7D)

However, as you can see above Rt, is undefined, to work this out we use:
- ![Equation for Rt](https://latex.codecogs.com/png.image?%5Cdpi%7B110%7DV%20=%20Val%20%5Ctimes%20LSB%20%5C%5C%5C%5CR_%7Bt%7D%20=%20R_%7B0%7D%20%5Ctimes%20%5Cfrac%7BV%7D%7BV_%7Bmax%7D%20-%20V%20%7D%20%5C%5C)

Where Val is the measured Value from the ADC & LSB is the "least significant bit", essentially the minimum value that the ADC can measure.

## Mock Code
```pseudocode
SUBROUITE main
    INPUT adc_value
    
    set MAX_VOLTAGE = 5.0
    SET LSB = MAX_VOLTAGE / 2**8
    SET ABS_ZERO = 273.15
    SET R0 = 10000
    SET B = 3950
    SET T0 = 25 + ABS_ZERO
    
    SET voltage = adc_value * LSB
        
    SET Voltage = adc_value * LSB
    SET Rt = R0 * voltage / (MAX_VOLTAGE - voltage)
    SET T = 1 / (1 / T0 + 1 / B * log(Rt / R0)) - ABS_ZERO
    write_to_file(T)
END
```