#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// --- Pin Definitions ---
#define DHTPIN 2
#define DHTTYPE DHT22
#define HEATER_LED 9

// --- Temperature Thresholds (in °C) ---
#define TEMP_LOW 24.0
#define TEMP_TARGET 30.0
#define TEMP_HIGH 35.0

// --- Enum for System States ---
enum HeaterState {
  IDLE,
  HEATING,
  STABILIZING,
  TARGET_REACHED,
  OVERHEAT
};

// --- Object Instantiations ---
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Change to 0x3F if screen remains blank
HeaterState currentState = IDLE;

void setup() {
  Serial.begin(9600);
  dht.begin();

  lcd.begin(16, 2, LCD_5x8DOTS);         // Required to initialize LCD
  lcd.backlight();     // Turn on LCD backlight

  pinMode(HEATER_LED, OUTPUT);
  digitalWrite(HEATER_LED, LOW);
}

void loop() {
  delay(2000);  // Read every 2 seconds

  float temp = dht.readTemperature();

  // --- Check for sensor failure ---
  if (isnan(temp)) {
    currentState = IDLE;
    digitalWrite(HEATER_LED, LOW);
    displayStatus("Sensor Error", "State: IDLE");
    Serial.println("Error reading temperature!");
    return;
  }

  // --- State Transitions Based on Temp ---
  if (temp < TEMP_LOW) {
    currentState = HEATING;
    digitalWrite(HEATER_LED, HIGH);
  } else if (temp >= TEMP_LOW && temp < TEMP_TARGET) {
    currentState = STABILIZING;
    digitalWrite(HEATER_LED, HIGH);
  } else if (temp >= TEMP_TARGET && temp <= TEMP_HIGH) {
    currentState = TARGET_REACHED;
    digitalWrite(HEATER_LED, LOW);
  } else if (temp > TEMP_HIGH) {
    currentState = OVERHEAT;
    digitalWrite(HEATER_LED, LOW);
  }

  // --- Show State & Temp on Serial and LCD ---
  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print(" C | State: ");
  Serial.println(getStateName(currentState));

  displayStatus("Temp: " + String(temp, 1) + " C", getStateName(currentState));
}

// --- Helper: Return state as String ---
String getStateName(HeaterState state) {
  switch (state) {
    case IDLE: return "IDLE";
    case HEATING: return "HEATING";
    case STABILIZING: return "STABILIZING";
    case TARGET_REACHED: return "TARGET OK";
    case OVERHEAT: return "OVERHEAT";
    default: return "UNKNOWN";
  }
}

// --- Helper: Display lines on LCD ---
void displayStatus(String line1, String line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}
