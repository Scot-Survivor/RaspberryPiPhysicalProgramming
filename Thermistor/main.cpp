//
// Created by scot on 16/03/2022.
//
#include <cmath>
#include <ADCDevice.hpp>


// My Full scale input range is 5V,


int main() {
    setbuf(stdout, nullptr);
    auto adc = new ADCDevice();
    if (adc->detectI2C(0x4b)) {
        delete adc;
        adc = new ADS7830();
        printf("ADS7830 detected\n");
        printf("Operating Temp Range: -40C to 125C\n");
    } // ic2Detect shows the address for our ADC module is 4b.

    double maximum_voltage = 5; // 5V
    double lsb_size = maximum_voltage/pow(2, 8); // 5V / 2**8 = 0.01953125V
    double B = 3950; // B = 3950
    double T0 = 25; // 25C
    double R0 = 10; // 10k
    double abs_zero = 273.15; // 273.15K
    printf("The values below are all for across the ADC device.\n");
    double val, voltage, current, power, Rt, temperature, percentage_uncertainty;
    while (adc) {
        val = adc->analogRead(0);
        if (val < 0) {
            printf("Error reading ADC\n");
            break;
        }
        voltage = val * lsb_size;
        current = (voltage / R0);
        power = current * voltage;
        Rt = R0 * voltage / (maximum_voltage - voltage);
        temperature = 1 / (1 / (abs_zero + T0) + log(Rt / R0) / B);
        percentage_uncertainty = (lsb_size / voltage)*100;
        printf("\rVoltage: %.3fV Current: %.3fA Power: %.3fmW Resistance: %.3fOhms Temperature: %.3fC Uncertainty: %.3f%%",
               voltage,
               current,
               power * 1000,
               Rt,
               temperature - abs_zero,
               percentage_uncertainty);
        fflush(stdout);
    }
}
