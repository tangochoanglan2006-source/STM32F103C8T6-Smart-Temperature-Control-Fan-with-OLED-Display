/*
 * STM32F103C8T6 Smart Temperature-Controlled Fan
 *
 * MCU: STM32F103C8T6 "Blue Pill"
 * Sensor: DHT22
 * Display: 0.96" SSD1306 I2C OLED
 * Fan: PWM-controlled fan
 *
 * Pin assignment:
 * DHT22 DATA  -> PA1
 * OLED SDA    -> PB7
 * OLED SCL    -> PB6
 * FAN PWM     -> PA8
 *
 * Serial: 115200 baud
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// -----------------------------
// Pin definitions
// -----------------------------

#define DHT_PIN PA1
#define DHT_TYPE DHT22

#define FAN_PWM PA8

// -----------------------------
// OLED configuration
// -----------------------------

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);

// -----------------------------
// DHT22
// -----------------------------

DHT dht(DHT_PIN, DHT_TYPE);

// -----------------------------
// Fan control thresholds
// -----------------------------

#define TEMP_FAN_OFF       25.0
#define TEMP_FAN_LOW       30.0
#define TEMP_FAN_MEDIUM    35.0
#define TEMP_FAN_HIGH      40.0

// -----------------------------
// Convert temperature to fan %
// -----------------------------

int calculateFanSpeed(float temperature)
{
  if (temperature < TEMP_FAN_OFF)
  {
    return 0;
  }

  if (temperature < TEMP_FAN_LOW)
  {
    return 30;
  }

  if (temperature < TEMP_FAN_MEDIUM)
  {
    return 50;
  }

  if (temperature < TEMP_FAN_HIGH)
  {
    return 75;
  }

  return 100;
}

// -----------------------------
// Set fan PWM
// -----------------------------

void setFanSpeed(int percentage)
{
  percentage = constrain(percentage, 0, 100);

  int pwmValue = map(
    percentage,
    0,
    100,
    0,
    255
  );

  analogWrite(FAN_PWM, pwmValue);
}

// -----------------------------
// OLED display
// -----------------------------

void updateDisplay(
  float temperature,
  float humidity,
  int fanSpeed
)
{
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);

  display.println("STM32 COOLING");

  display.setTextSize(2);
  display.setCursor(0, 15);

  display.print(temperature, 1);
  display.println(" C");

  display.setTextSize(1);
  display.setCursor(0, 40);

  display.print("Humidity: ");
  display.print(humidity, 1);
  display.println("%");

  display.setCursor(0, 52);

  display.print("Fan: ");
  display.print(fanSpeed);
  display.println("%");

  display.display();
}

// -----------------------------
// Setup
// -----------------------------

void setup()
{
  Serial.begin(115200);

  pinMode(FAN_PWM, OUTPUT);

  // Start with the fan turned off
  setFanSpeed(0);

  // Initialize DHT22
  dht.begin();

  // Allow the OLED power supply to settle after power-on.
  delay(500);

  // Initialize OLED
  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        OLED_ADDRESS))
  {
    Serial.println("OLED initialization failed!");

    while (1)
    {
      delay(1000);
    }
  }

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(0, 0);

  display.println("STM32");

  display.setTextSize(1);
  display.setCursor(0, 28);

  display.println("Temperature Fan");

  display.setCursor(0, 42);

  display.println("Controller");

  display.display();

  delay(2000);

  Serial.println();
  Serial.println("==============================");
  Serial.println("STM32 Temperature Fan");
  Serial.println("Controller Started");
  Serial.println("==============================");
}

// -----------------------------
// Main loop
// -----------------------------

void loop()
{
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Check sensor reading
  if (isnan(temperature) || isnan(humidity))
  {
    Serial.println("ERROR: DHT22 reading failed.");

    display.clearDisplay();

    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);

    display.setCursor(0, 0);
    display.println("DHT22 ERROR");

    display.setCursor(0, 20);
    display.println("Check sensor");

    display.setCursor(0, 35);
    display.println("and wiring.");

    display.display();

    // Stop the fan if sensor fails
    setFanSpeed(0);

    delay(2000);

    return;
  }

  // Calculate required fan speed
  int fanSpeed = calculateFanSpeed(temperature);

  // Apply PWM
  setFanSpeed(fanSpeed);

  // Send data to Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperature, 1);

  Serial.print(" C | Humidity: ");
  Serial.print(humidity, 1);

  Serial.print(" % | Fan: ");
  Serial.print(fanSpeed);

  Serial.println(" %");

  // Update OLED
  updateDisplay(
    temperature,
    humidity,
    fanSpeed
  );

  // DHT22 should not be sampled too quickly
  delay(2000);
}
