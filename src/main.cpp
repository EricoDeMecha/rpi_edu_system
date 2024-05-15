#include <cstddef>
#include <cstdio>
#include "mpu6050.h"
#include "bme280.h"
#include "mcp3008.h"
#include "hmc5883l.h"
#include <wiringPi.h>

#define MAXTIMINGS 85
#define DHTPIN 3

int dht11_dat[5] = {0, 0, 0, 0, 0};
double dht_temp;
double dht_humidity;

int read_dht11_dat()
{
    uint8_t laststate = HIGH;
    uint8_t counter = 0;
    uint8_t j = 0, i;

    dht11_dat[0] = dht11_dat[1] = dht11_dat[2] = dht11_dat[3] = dht11_dat[4] = 0;

    pinMode(DHTPIN, OUTPUT);
    digitalWrite(DHTPIN, LOW);
    delay(18);
    digitalWrite(DHTPIN, HIGH);
    delayMicroseconds(40);
    pinMode(DHTPIN, INPUT);

    for (i = 0; i < MAXTIMINGS; i++)
    {
        counter = 0;
        while (digitalRead(DHTPIN) == laststate)
        {
            counter++;
            delayMicroseconds(1);
            if (counter == 255)
                break;
        }
        laststate = digitalRead(DHTPIN);

        if (counter == 255)
            break;

        if ((i >= 4) && (i % 2 == 0))
        {
            dht11_dat[j / 8] <<= 1;
            if (counter > 64)
                dht11_dat[j / 8] |= 1;
            j++;
        }
    }

    if ((j >= 40) && (dht11_dat[4] == ((dht11_dat[0] + dht11_dat[1] + dht11_dat[2] + dht11_dat[3]) & 0xFF)))
    {
        dht_temp = dht11_dat[2] + (dht11_dat[3] / 10);
        dht_humidity = dht11_dat[0] + (dht11_dat[1] / 10);
        return (1);
    }
    else
        return (0);
}

int main()
{
    float roll, pitch, yaw;
    float ax, ay, az;
    // Create an instance of the BME280 class
    MPU6050 mpu6050(0x68, true);

    // Create an instance of the BME280 class
    BME280 sensor(0x76);
    // Call the measure method to get the measurements
    BME280::Measurement_t measurements = sensor.measure();

    // Create an instance of the MCP3008 class
    MCP3008Lib::MCP3008 mcp3008(0, 0, 1350000, SPI_MODE_0);

    mcp3008.connect();

    // Create an instance of the HMC5883L class
    HMC5883L compass = NULL;

    int ret = 0;

    if (wiringPiSetup() == -1)
        // exit(1);
        printf("WirinPi setup failed\n");

    // Now you can access the measurements

    while (1)
    {
        float temperature = measurements.temperature; // in degrees Celsius
        float pressure = measurements.pressure;       // in hPa
        float humidity = measurements.humidity;       // in %
        float altitude = measurements.altitude;       // in meters

        mpu6050.getGyro(&roll, &pitch, &yaw);
        mpu6050.getAccel(&ax, &ay, &az);

        unsigned short channel1 = mcp3008.read(0, MCP3008Lib::Mode::SINGLE);
        printf("Roll: %f, Pitch: %f, Yaw: %f\n", roll, pitch, yaw);
        printf("Ax: %f, Ay: %f, Az: %f\n\n", ax, ay, az);

        read_dht11_dat();

        double heading = getCurrentHeading(compass);


        printf("Heading: %f\n", heading);
        delay(100);

        printf("Humidity: %.1f%%\nTemperature: %.1f C\n", dht_humidity, dht_temp);

        printf("Temperature: %.2f C Pressure: %.2f hPa Humidity: %.2f %% Altitude: %.2f m\n\n", temperature, pressure, humidity, altitude);

        printf("Channel 1: %d\n\n", channel1);
    }
}