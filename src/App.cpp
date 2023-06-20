#include "App.hpp"

App::App()
	: sensor(DHT(SENSOR_PIN, SENSOR_TYPE))
	, lcd(LiquidCrystal(4, 6, 10, 11, 12, 13))
	, dhtData({NAN, NAN})
	, heatIndex(NAN)
	, timeOfLastUpdate(NAN)
	, heatActive(false)
	, humidifierActive(false)
	, timesHeatOn(0)
{}

static double secs() {
	return static_cast<double>(millis()) / 1000.0;
}

void App::setup() {
	Serial.begin(9600);
	sensor.begin();

	lcd.setBacklightPin(LCD_BACKLIGHT_PIN, POSITIVE);
	lcd.begin(16,2);
	lcd.createChar(0, TEMPERATURE_SYMBOL);
	lcd.createChar(1, HEATINDEX_SYMBOL);

	pinMode(HEAT_PIN, OUTPUT);
	digitalWrite(HEAT_PIN, LOW);

	pinMode(HUMIDIFIER_PIN, OUTPUT);
	digitalWrite(HUMIDIFIER_PIN, LOW);
}


void App::loop() {
	const double now = secs();

	if(now > 5000) {
		asm("jmp 0x0000");
	}

	if(now - timeOfLastUpdate < UPDATE_INTERVAL) {
		return;
	}

	updateSensorInfo();
	updateHeatIndex();
	updateCount();

	printTemperature();

	updateHeaterRelay();
	updateHumidifierRelay();

	updateDisplay();

	timeOfLastUpdate = secs();
}

App::HexNumber App::toHex(unsigned int input) {
	static char HEXADECIMAL[] = "0123456789ABCDF";

	const unsigned int firstDigit = input % 16;
	const unsigned int secondDigit = (input / 16) % 16;

	return App::HexNumber { .number = {HEXADECIMAL[secondDigit], HEXADECIMAL[firstDigit]} };
}

void App::updateDisplay() {
	lcd.backlight(); // Controlled externally
	lcd.clear();

	lcd.setCursor(0,0);
	lcd.write("T");
	lcd.print(dhtData.temperature);
	lcd.write(static_cast<uint8_t>(0));
	lcd.print("C|");
	lcd.print("H");
	lcd.print(dhtData.humidity);
	lcd.print("%");

	lcd.setCursor(0,1);
	lcd.print("T ");
	lcd.print(heatActive? "On":"Off");
	lcd.setCursor(5,1);
	lcd.print("|H ");
	lcd.print(humidifierActive? "On":"Off");

	lcd.setCursor(11,1);

	lcd.print("|0x");
	lcd.print(toHex(timesHeatOn).number);
}

void App::updateHeaterRelay() {
	if (heatActive) {
		bool turnOffHeat = HEAT_OFF_THRESHOLD <= dhtData.temperature;
		if(turnOffHeat) {
			Serial.println("Heat not active");
			digitalWrite(HEAT_PIN, LOW);
			heatActive = false;
		}
	} else {
		bool turnOnHeat = dhtData.temperature <= HEAT_ON_THRESHOLD;
		if(turnOnHeat) {
			Serial.println("Heat active");
			digitalWrite(HEAT_PIN, HIGH);
			heatActive = true;
		}
	}
}

void App::updateHumidifierRelay() {
	if (humidifierActive) {
		bool turnOffHumidifier = HUMIDIFIER_OFF_THRESHOLD <= dhtData.humidity;
		if(turnOffHumidifier) {
			Serial.println("Humidifier not active");
			digitalWrite(HUMIDIFIER_PIN, LOW);
			humidifierActive = false;
		}
	} else {
		bool turnOnHumidifier = dhtData.humidity <= HUMIDIFIER_ON_THRESHOLD;
		if(turnOnHumidifier) {
			Serial.println("Humidifier active");
			digitalWrite(HUMIDIFIER_PIN, HIGH);
			humidifierActive = true;
		}
	}
}

void App::updateCount() {
	if(heatActive) {
		timesHeatOn++;
	}
}

double App::heatIndexApproximation(double temperature, double humidity) {
	const double c1 = -8.78469475556;
	const double c2 = 1.61139411;
	const double c3 = 2.33854883889;
	const double c4 = -0.14611605;
	const double c5 = -0.012308094;
	const double c6 = -0.0164248277778;
	const double c7 = 2.211732e-3;
	const double c8 = 7.2546e-4;
	const double c9 = -3.582e-6;
	return c1 + c2 * temperature
	          + c3 * humidity
			  + c4 * temperature * humidity
			  + c5 * temperature * temperature
			  + c6 * humidity * humidity
			  + c7 * temperature * temperature * humidity
			  + c8 * temperature * humidity * humidity
			  + c9 * temperature * temperature * humidity * humidity;
}

void App::updateSensorInfo() {
	const float temperature = sensor.readTemperature();
	const float humidity = sensor.readHumidity();

	if(isnan(temperature) || isnan(humidity)) {
		Serial.println("Error reading temperature and humidity");
	}

	dhtData = SensorInfo {
		.temperature = temperature,
		.humidity = humidity
	};
}

void App::updateHeatIndex() {
	heatIndex = heatIndexApproximation(dhtData.temperature, dhtData.humidity);
}

void App::printTemperature() const {
	Serial.print("[");
	Serial.print(secs());
	Serial.print("] Temperature\t");
	Serial.print(dhtData.temperature);
	Serial.println("°C");
	Serial.print("[");
	Serial.print(secs());
	Serial.print("] Humidity   \t");
	Serial.print(dhtData.humidity);
	Serial.println("%");
	Serial.print("[");
	Serial.print(secs());
	Serial.print("] Heat index \t");
	Serial.print(heatIndex);
	Serial.println("°C");
}