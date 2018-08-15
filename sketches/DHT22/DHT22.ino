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

//TODO: increase send period to 5 minutes
//TODO: average as many samples as possible per send period
//TODO: transmit the samples_count and sample_errors as variables too

#include <UbidotsMicroESP8266.h>
#include <ErriezDHT22.h>
#include <ErriezTimestamp.h>
#include <Esp.h>
#include "private.h"

#define UBIDOTS_UPDATE_INTERVAL_MS  15000 // 15 seconds

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

    // Initialize sensor
    sensor.begin();

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

    timestampDelta = timestamp.delta();

    // Once every UBIDOTS_UPDATE_INTERVAL_MS
    if (timestampDelta >= UBIDOTS_UPDATE_INTERVAL_MS) {

        // Check minimum interval of 2000 ms between sensor reads
        if (sensor.available()) {

            // Read temperature from sensor (blocking)
            temperature = sensor.readTemperature();

            // Print temperature (can handle a failed read)
            printTemperature(temperature);

            // Correct it to 0 on failed reads
            if (temperature == ~(int16_t)0) {
                temperature = 0;
            }

            // Read humidity from sensor (blocking)
            humidity = sensor.readHumidity();

            // Print humidity (can handle a failed read)
            printHumidity(humidity);

            // Correct it to 0 on failed reads
            if (humidity == ~(int16_t)0) {
                humidity = 0;
            }
        }

        // Convert the sensor values to floats with a single decimal
        sendTemperature = (float)temperature / 10;
        sendHumidity = (float)humidity / 10;

        // Add the variables to the Ubidots data
        client.add("temperature", sendTemperature);
        client.add("humidity", sendHumidity);

        // Transmit
        client.sendAll(true);
        Serial.println("");

        // Re-start timestamp
        timestamp.start();
    }

    // Keep track of free memory and print any changes
    UpdateFreeMemory();
}