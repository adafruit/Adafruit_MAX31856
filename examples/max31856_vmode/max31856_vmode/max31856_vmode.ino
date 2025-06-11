// This example demonstrates doing a one-shot measurement "manually" in VMODE8 or VMODE32 mode. 
// Separate calls are made to trigger the conversion and then check
// for conversion complete. While this typically only takes a couple
// 100 milliseconds, that times is made available by separating these
// two steps.

#include <Adafruit_MAX31856.h>

// Use software SPI: CS, DI, DO, CLK
//Adafruit_MAX31856 maxthermo = Adafruit_MAX31856(10, 11, 12, 13);
// use hardware SPI, just pass in the CS pin
Adafruit_MAX31856 maxthermo = Adafruit_MAX31856(10);

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  Serial.println("MAX31856 thermocouple test in Voltage Gain mode");

  if (!maxthermo.begin()) {
    Serial.println("Could not initialize thermocouple.");
    while (1) delay(10);
  }

  maxthermo.setThermocoupleType(MAX31856_VMODE_G8); // or VMODE_G32, other modes will return NA values

  Serial.print("Thermocouple type: ");
  switch (maxthermo.getThermocoupleType() ) {
    case MAX31856_TCTYPE_B: Serial.println("B Type"); break;
    case MAX31856_TCTYPE_E: Serial.println("E Type"); break;
    case MAX31856_TCTYPE_J: Serial.println("J Type"); break;
    case MAX31856_TCTYPE_K: Serial.println("K Type"); break;
    case MAX31856_TCTYPE_N: Serial.println("N Type"); break;
    case MAX31856_TCTYPE_R: Serial.println("R Type"); break;
    case MAX31856_TCTYPE_S: Serial.println("S Type"); break;
    case MAX31856_TCTYPE_T: Serial.println("T Type"); break;
    case MAX31856_VMODE_G8: Serial.println("Voltage x8 Gain mode"); break;
    case MAX31856_VMODE_G32: Serial.println("Voltage x32 Gain mode"); break;
    default: Serial.println("Unknown"); break;
  }

  maxthermo.setConversionMode(MAX31856_ONESHOT_NOWAIT);
}

void loop() {
  // trigger a conversion, returns immediately
  maxthermo.triggerOneShot();

  //
  // here's where you can do other things
  //
  delay(500); // replace this with whatever
  //
  //

  // check for conversion complete and read temperature
  if (maxthermo.conversionComplete()) {
    // Voltage is read in µV
    // Any other mode than MAX31856_VMODE_G8 or MAX31856_VMODE_G32 will output NAN values with the readThermocoupleVoltage function
    Serial.println(maxthermo.readThermocoupleVoltage());
  } else {
    Serial.println("Conversion not complete!");
  }
}
