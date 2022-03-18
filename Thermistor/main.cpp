//
// Created by scot on 16/03/2022.
//
#include <cmath>
#include <ADCDevice.hpp>
#include <vector>
#include <tuple>

#define MAX_TO_AVG 2
#define MAX_SAMPLES 5000000
const char *VoltageLabel = "Voltage (V)";
const char *TemperatureLabel = "Temperature (C)";
const char *ResistanceLabel = "Resistance (kOhms)";
const char *CurrentLabel = "Current (uA)";
const char *PowerLabel = "Power (uW)";
const char *UncertaintyLabel = "Uncertainty (%)";
const char *PADDING = "    ";

// My Full scale input range is 5V

struct CurrentData {
    double voltage;
    double temperature;
    double current;
    double resistance;
    double power;
    double uncertainty;
};


struct arg_struct {
    CurrentData *data;
    int index;
    bool running;
    bool is_ready;
};


void write_header() {
    FILE *file = fopen("thermistor.data", "w");
    if (file != nullptr) {
        fprintf(file, "# %s%s%s%s%s%s%s%s%s%s%s\n", VoltageLabel, PADDING, TemperatureLabel, PADDING, ResistanceLabel, PADDING, CurrentLabel, PADDING, PowerLabel, PADDING, UncertaintyLabel);
    } else {
        perror("Failed to open file\n");
        exit(EXIT_FAILURE);
    }
    fclose(file);
}


// Writes output to file
void write_thread(void *arguments) {
   auto *args = static_cast<arg_struct *>(arguments);
    auto data = args->data;
    FILE *file = fopen("thermistor.data", "a");
    while (args->running && args->index < MAX_SAMPLES) {
        if (file != nullptr) {
            if (args->is_ready) {
                fprintf(file, "%f%s%f%s%f%s%f%s%f%s%f\n", data->voltage, PADDING, data->temperature, PADDING, data->resistance, PADDING, data->current, PADDING, data->power, PADDING, data->uncertainty);
                args->is_ready = false;
                args->index++;
            }
        } else {
            perror("Failed to open file\n");
            exit(EXIT_FAILURE);
        }
    }
    fclose(file);
}


void write_gnuplot_script() {
    FILE *file = fopen("thermistor.gnuplot", "w");
    if (file != nullptr) {
        fprintf(file, "set output \"thermistor.png\"\n");
        fprintf(file, "set title \"Thermistor\"\n");
        fprintf(file, "set xlabel \"%s\"\n", VoltageLabel);
        fprintf(file, "set ylabel \"%s\"\n", TemperatureLabel);
        fprintf(file, "set xrange [0:5]\n");
        fprintf(file, "set yrange [-40:125]\n");
        fprintf(file, "f(x) = a*x + b\n");
        fprintf(file, "fit f(x) \"thermistor.data\" using 1:2 via a,b\n");
        fprintf(file, "set grid\n");
        fprintf(file, "plot f(x) title 'Best fit'\n");
        fprintf(file, "plot \"thermistor.data\" using 1:2 title 'Data'\n");
    } else {
        perror("Failed to open file\n");
        exit(EXIT_FAILURE);
    }
    fclose(file);
}


std::tuple<double, double, double, double, double, double> get_average(std::vector<std::tuple<double, double, double, double, double, double>> data_vector) {
    double sum_1 = 0;
    double sum_2 = 0;
    double sum_3 = 0;
    double sum_4 = 0;
    double sum_5 = 0;
    double sum_6 = 0;
    for (auto &data : data_vector) {
        sum_1 += std::get<0>(data);
        sum_2 += std::get<1>(data);
        sum_3 += std::get<2>(data);
        sum_4 += std::get<3>(data);
        sum_5 += std::get<4>(data);
        sum_6 += std::get<5>(data);
    }
    return std::make_tuple(sum_1 / data_vector.size(), sum_2 / data_vector.size(), sum_3 / data_vector.size(),
                           sum_4 / data_vector.size(), sum_5 / data_vector.size(), sum_6 / data_vector.size());
}


int main(int argc, char *argv[]) {
    setbuf(stdout, nullptr);
    auto adc = new ADCDevice();
    if (adc->detectI2C(0x4b)) {
        delete adc;
        adc = new ADS7830();
        printf("ADS7830 detected\n");
        printf("Operating Temp Range: -40C to 125C\n");
    } // ic2Detect shows the address for our ADC module is 4b.
    //file setup
    write_header();
    write_gnuplot_script();

    // Variables
    double maximum_voltage = 5; // 5V
    double lsb_size = maximum_voltage/pow(2, 8); // 5V / 2**8 = 0.01953125V
    double abs_zero = 273.15; // 273.15K
    double B = 3950; // B = 3950
    double T0 = 25 + abs_zero; // 25C
    double R0 = 10000; // 10k
    printf("The values below are all for across the ADC device.\n");
    double val, voltage, current, power, Rt, temperature, percentage_uncertainty;
    std::vector<std::tuple<double, double, double, double, double, double>> data_vector;
    data_vector.reserve(MAX_TO_AVG);
    int tries;
    auto *data = new CurrentData();
    data->voltage = 0;
    data->temperature = 0;
    data->resistance = 0;
    data->current = 0;
    data->power = 0;
    data->uncertainty = 0;

    //thread setup
    pthread_t thread;
    arg_struct args{};
    args.data = data;
    args.running = true;
    args.is_ready = false;
    args.index = 0;
    pthread_create(&thread, nullptr, (void *(*)(void *)) &write_thread, &args);

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
        temperature = 1 / (1 / (T0) + log(Rt / R0) / B);
        percentage_uncertainty = (lsb_size / voltage)*100;
        data_vector.emplace_back(voltage, temperature, Rt/1000, current*pow(10, 6), power*1000, percentage_uncertainty);
        if (tries++ > MAX_TO_AVG) {
            if (MAX_TO_AVG == 1) {
                data->temperature = temperature;
                data->voltage = voltage;
                data->resistance = Rt;
                data->current = current;
                data->power = power;
                data->uncertainty = percentage_uncertainty;
                args.is_ready = true;
            }
            else {
                args.is_ready = true;
                std::tuple<double, double, double, double, double, double> average = get_average(data_vector);
                data->voltage = std::get<0>(average);
                data->temperature = std::get<1>(average)-abs_zero;
                data->resistance = std::get<2>(average);
                data->current = std::get<3>(average);
                data->power = std::get<4>(average);
                data->uncertainty = std::get<5>(average);
                tries = 0;
                data_vector.clear();
            }
        }
        printf("\rVoltage: %.3fV Current: %.3fuA Power: %.3fmW Resistance: %.3fkOhms Temperature: %.3fC Uncertainty: %.3f%%",
               voltage,
               current*pow(10, 6),
               power * 1000,
               Rt/1000,
               temperature - abs_zero,
               percentage_uncertainty);
        fflush(stdout);
        if (args.index > MAX_SAMPLES) {
            args.running = false;
            pthread_join(thread, nullptr);
            break;
        }
    }
    delete data;
    return 0;
}
