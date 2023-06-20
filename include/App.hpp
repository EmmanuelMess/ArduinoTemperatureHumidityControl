#ifndef TEMPERATUREHUMIDITYCONTROL_APP_HPP
#define TEMPERATUREHUMIDITYCONTROL_APP_HPP

#include <Arduino.h>

#include <Wire.h>
#include "LiquidCrystal.h"
#include "DHT.h"

/**
 * Main class app, tries to keep the state small
 */
class App {
public:
	App();

	void setup();
	void loop();

private:
	struct SensorInfo {
		float temperature;
		float humidity;
	};

	struct HexNumber {
		char number[2];
	};

	/**
	 * Temperature symbol for LCD
	 */
	byte TEMPERATURE_SYMBOL[8] = {
		B01110,
		B01010,
		B01110,
		B00000,
		B00000,
		B00000,
		B00000,
		B00000
	};

	/**
	 * Heat index symbol for LCD
	 */
	byte HEATINDEX_SYMBOL[8] = {
		B10100,
		B11100,
		B10100,
		B00000,
		B00111,
		B00010,
		B00111,
		B00000
	};

	/**
	 * In seconds
	 */
	const float UPDATE_INTERVAL = 5*60;

	const uint8_t SENSOR_PIN = 2;
	const int SENSOR_TYPE = DHT11;

	const uint8_t LCD_BACKLIGHT_PIN = 7;

	const uint8_t HEAT_PIN = 8;

	/**
	 * The point at which the heat stops working
	 * In celsius
 	 */
	const float HEAT_OFF_THRESHOLD = 22;

	/**
	 * The point at which the heat starts working
	 * In celsius
	 */
	const float HEAT_ON_THRESHOLD = 21;

	const uint8_t HUMIDIFIER_PIN = 9;

	/**
	 * The point at which the humidifier stops working
	 */
	const float HUMIDIFIER_OFF_THRESHOLD = 45; // in percentage

	/**
	 * The point at which the humidifier starts working
	 */
	const float HUMIDIFIER_ON_THRESHOLD = 40; // in percentage

	DHT sensor;
	LiquidCrystal lcd;
	SensorInfo dhtData;
	double heatIndex;
	double timeOfLastUpdate;
	bool heatActive;
	bool humidifierActive;
	unsigned int timesHeatOn;

	static HexNumber toHex(unsigned int input);
	static double heatIndexApproximation(double temperature, double humidity);

	void updateSensorInfo();
	void updateHeatIndex();
	void updateCount();

	void printTemperature() const;

	void updateHeaterRelay();
	void updateHumidifierRelay();

	void updateDisplay();
};


#endif //TEMPERATUREHUMIDITYCONTROL_APP_HPP
