/*
 * This file is part of the ESP8266Ubidots distribution
 * (https://github.com/baskapteijn/ESP8266Ubidots).
 * Copyright (c) 2018 Bas Kapteijn.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdint.h>
#include <stdbool.h>
#include <UbidotsMicroESP8266.h>
#include <ErriezDHT22.h>
#include <ErriezTimestamp.h>
#include <Esp.h>
#include "private.h"

// 5 minutes
#define UBIDOTS_UPDATE_INTERVAL_MS  (5ul * 60ul * 1000ul)
#define UBIDOTS_RETRIES             10

// 1 second is enough time for the sensor readout
#define DHT22_SAMPLE_TIME_MS        1000ul

// Connect DTH22 DAT pin to Arduino DIGITAL pin
#if defined(ARDUINO_ESP8266_NODEMCU)
#define DHT22_PIN      D2
#else
#error "May work, but not tested on this target"
#endif

// Create DHT22 sensor object
DHT22 sensor = DHT22(DHT22_PIN);

// Create Ubidots client object
Ubidots client(TOKEN);

// Create timestamp with milliseconds resolution
TimestampMillis timestamp;

// Globals
static uint32_t freeMemoryMin = UINT32_MAX;
static uint32_t freeMemoryMax = 0;

static uint64_t temperatureSum = 0;
static uint64_t humiditySum = 0;
static uint64_t samples = 0;
static uint64_t sampleErrors = 0;

// Function prototypes
static void UpdateFreeMemory(void);
static void printTemperature(int16_t temperature);
static void printHumidity(int16_t humidity);

static void UpdateFreeMemory(void)
{
    bool updateConsole = false;
    uint32_t tmpFreeMemory = ESP.getFreeHeap();

    if (tmpFreeMemory < freeMemoryMin) {
        freeMemoryMin = tmpFreeMemory;
        updateConsole = true;
    }

    if (tmpFreeMemory > freeMemoryMax) {
        freeMemoryMax = tmpFreeMemory;
        updateConsole = true;
    }

    // Only update on change
    if (updateConsole == true) {
        Serial.print("Free RAM ");
        Serial.println(tmpFreeMemory);
        Serial.print("Free RAM min ");
        Serial.println(freeMemoryMin);
        Serial.print("Free RAM max ");
        Serial.println(freeMemoryMax);
        Serial.print("\n");
    }
}

static void printTemperature(int16_t temperature)
{
    // Check valid temperature value
    if (temperature == ~0) {
        // Temperature error (Check hardware connection)
        Serial.println("Temperature: Error");
    } else {
        // Print temperature
        Serial.print("Temperature: ");
        Serial.print(temperature / 10);
        Serial.print(".");
        Serial.print(temperature % 10);

        // Print degree Celsius symbol

        // Print *C characters which are displayed correctly in the serial
        // terminal of the Arduino IDE
        Serial.println(F(" *C"));
    }
}

static void printHumidity(int16_t humidity)
{
    // Check valid humidity value
    if (humidity == ~0) {
        // Humidity error (Check hardware connection)
        Serial.println(F("Humidity: Error"));
    } else {
        // Print humidity
        Serial.print(F("Humidity: "));
        Serial.print(humidity / 10);
        Serial.print(F("."));
        Serial.print(humidity % 10);
        Serial.println(F(" %"));
    }

    Serial.println();
}

void setup()
{
    // Start timestamp
    timestamp.start();

    // Initialize serial port
    Serial.begin(115200);
    while (!Serial) { ;
    }
    Serial.println("Ubidots DHT22 temperature and humidity example\n");

    // Keep track of free memory and print any changes
    UpdateFreeMemory();

    // Initialize sensor, max 2 retries (so 3 tries) per read, no averaging
    // Many retries might heat-up the sensor, but we do not expect them to happen very often
    sensor.begin(2, 0);

    // Initialize WIFI connection
    client.wifiConnection(SSID, PASS);
}

void loop()
{
    unsigned long timestampDelta = 0;
    int16_t temperature = 0;
    int16_t humidity = 0;
    float sendTemperature = 0;
    float sendHumidity = 0;
    bool skipSample = false;
    uint8_t retries = 0;

    timestampDelta = timestamp.delta();

    // Once every UBIDOTS_UPDATE_INTERVAL_MS
    if (timestampDelta >= UBIDOTS_UPDATE_INTERVAL_MS) {

        // Re-start timestamp immediately to maintain the interval
        timestamp.start();

        // Store the average of the summed sensor values
        if (samples > 0) { // Protect against divide by 0
            temperatureSum /= samples;
            humiditySum /= samples;
        } else {
            temperatureSum = 0;
            humiditySum = 0;
        }

        // Convert the sensor values to floats with a single decimal
        sendTemperature = (float)temperatureSum / 10;
        sendHumidity = (float)humiditySum / 10;

        Serial.print("avg temperature: ");
        Serial.println(sendTemperature, 1);
        Serial.print("avg humidity: ");
        Serial.println(sendHumidity, 1);

        // Add the variables to the Ubidots data
        client.add("temperature", sendTemperature);
        client.add("humidity", sendHumidity);
        client.add("samples", samples);
        client.add("sample_errors", sampleErrors);

        // Transmit
        retries = 0;
        // Unlike what the sendAll doxygen states, false is returned on success
        while (client.sendAll(true) != false) {
            delay(1000);
            if (retries++ > UBIDOTS_RETRIES) {
                // All hope is lost, restart the MCU
                ESP.restart();
            }
        }
        Serial.println("");

        // Reset data
        temperatureSum = 0;
        humiditySum = 0;
        samples = 0;
        sampleErrors = 0;
    } else if (timestampDelta <
               (UBIDOTS_UPDATE_INTERVAL_MS - DHT22_SAMPLE_TIME_MS)) {
        // There is still more than enough time to get sensor values

        // Check minimum interval of 2000 ms between sensor reads
        if (sensor.available()) {

            // Read temperature from sensor (blocking)
            temperature = sensor.readTemperature();
            // If there were retries, make sure to store them
            sampleErrors += sensor.getNumRetriesLastConversion();

            // Print temperature (can handle a failed read)
            printTemperature(temperature);

            // Correct it to 0 on failed reads
            if (temperature == ~(int16_t)0) {
                temperature = 0;
                skipSample = true;
            }

            // Read humidity from sensor (blocking)
            humidity = sensor.readHumidity();
            // If there were retries, make sure to store them
            sampleErrors += sensor.getNumRetriesLastConversion();

            // Print humidity (can handle a failed read)
            printHumidity(humidity);

            // Correct it to 0 on failed reads
            if (humidity == ~(int16_t)0) {
                humidity = 0;
                skipSample = true;
            }

            // Add the samples to their respective sums (if valid)
            if (skipSample == false) {
                temperatureSum += temperature;
                humiditySum += humidity;
                samples++;
            } // else just skip it, it's been registered in sampleErrors
        }
    }

    // Keep track of free memory and print any changes
    UpdateFreeMemory();
}
