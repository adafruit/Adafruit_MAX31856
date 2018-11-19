/*!
 * @file Adafruit_MAX31856.cpp
 *
 * @mainpage Adafruit MAX31856 thermocouple reader
 *
 * @section intro_sec Introduction
 *
 * This is the documentation for Adafruit's MAX31856 driver for the
 * Arduino platform.  It is designed specifically to work with the
 * Adafruit MAX31856 breakout: https://www.adafruit.com/products/3263
 *
 * These sensors use SPI to communicate, 4 pins are required to  
 *  interface with the breakout.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * @section dependencies Dependencies
 *
 * This library depends on <a href="https://github.com/adafruit/Adafruit_Sensor">
 * Adafruit_Sensor</a> being present on your system. Please make sure you have
 * installed the latest version before using this library.
 *
 * @section author Author
 *
 * Written by ladyada for Adafruit Industries.
 *
 * @section license License
 *
 * BSD license, all text here must be included in any redistribution.
 *
 */

#include "Adafruit_MAX31856.h"
#ifdef __AVR
  #include <avr/pgmspace.h>
#elif defined(ESP8266)
  #include <pgmspace.h>
#endif

#include <stdlib.h>
#include <SPI.h>

static SPISettings max31856_spisettings = SPISettings(500000, MSBFIRST, SPI_MODE1);


/**************************************************************************/
/*!
    @brief  Instantiate MAX31856 object and use software SPI pins
    @param  spi_cs Bitbang SPI Chip Select
    @param  spi_mosi Bitbang SPI MOSI
    @param  spi_miso Bitbang SPI MISO
    @param  spi_clk Bitbang SPI Clock
*/
/**************************************************************************/
Adafruit_MAX31856::Adafruit_MAX31856(int8_t spi_cs, int8_t spi_mosi, int8_t spi_miso, int8_t spi_clk) {
  _sclk = spi_clk;
  _cs = spi_cs;
  _miso = spi_miso;
  _mosi = spi_mosi;

}

/**************************************************************************/
/*!
    @brief  Instantiate MAX31856 object and use hardware SPI
    @param  spi_cs Any pin for SPI Chip Select
*/
/**************************************************************************/
Adafruit_MAX31856::Adafruit_MAX31856(int8_t spi_cs) {
  _cs = spi_cs;
  _sclk = _miso = _mosi = -1;
}

/**************************************************************************/
/*!
    @brief  Initialize MAX31856 attach/set pins or SPI device, default to K thermocouple
    @returns Always returns true at this time (no known way of detecting chip ID)
*/
/**************************************************************************/
boolean Adafruit_MAX31856::begin(void) {
  pinMode(_cs, OUTPUT);
  digitalWrite(_cs, HIGH);

  if (_sclk != -1) {
    //define pin modes
    pinMode(_sclk, OUTPUT); 
    pinMode(_mosi, OUTPUT); 
    pinMode(_miso, INPUT);
  } else {
    //start and configure hardware SPI
    SPI.begin();
  }

  // assert on any fault
  writeRegister8(MAX31856_MASK_REG, 0x0);
  
  writeRegister8(MAX31856_CR0_REG, MAX31856_CR0_OCFAULT0);
  setThermocoupleType(MAX31856_TCTYPE_K);

  return true;
}

/**************************************************************************/
/*!
    @brief  Set which kind of Thermocouple (K, J, T, etc) to detect & decode
    @param type The enumeration type of the thermocouple
*/
/**************************************************************************/
void Adafruit_MAX31856::setThermocoupleType(max31856_thermocoupletype_t type) {
  uint8_t t = readRegister8(MAX31856_CR1_REG);
  t &= 0xF0; // mask off bottom 4 bits
  t |= (uint8_t)type & 0x0F;
  writeRegister8(MAX31856_CR1_REG, t);
}

/**************************************************************************/
/*!
    @brief  Get which kind of Thermocouple (K, J, T, etc) we are using
    @returns The enumeration type of the thermocouple
*/
/**************************************************************************/
max31856_thermocoupletype_t Adafruit_MAX31856::getThermocoupleType(void) {
  uint8_t t = readRegister8(MAX31856_CR1_REG);
  t &= 0x0F;

  return (max31856_thermocoupletype_t)(t);
}

/**************************************************************************/
/*!
    @brief  Read the fault register (8 bits)
    @returns 8 bits of fault register data
*/
/**************************************************************************/
uint8_t Adafruit_MAX31856::readFault(void) {
  return readRegister8(MAX31856_SR_REG);
}

/**************************************************************************/
/*!
    @brief  Sets the threshhold for internal chip temperature range 
    for fault detection. NOT the thermocouple temperature range!
    @param  low Low (min) temperature, signed 8 bit so -128 to 127 degrees C
    @param  high High (max) temperature, signed 8 bit so -128 to 127 degrees C
*/
/**************************************************************************/
void Adafruit_MAX31856::setColdJunctionFaultThreshholds(int8_t low, int8_t high) {
  writeRegister8(MAX31856_CJLF_REG, low);
  writeRegister8(MAX31856_CJHF_REG, high);
}

/**************************************************************************/
/*!
    @brief  Sets the mains noise filter. Can be set to 50 or 60hz. 
    Defaults to 60hz. You need to call this if you live in a 50hz country.
    @param  noiseFilter One of MAX31856_NOISE_FILTER_50HZ or MAX31856_NOISE_FILTER_60HZ
*/
/**************************************************************************/
void Adafruit_MAX31856::setNoiseFilter(max31856_noise_filter_t noiseFilter) {
  uint8_t t = readRegister8(MAX31856_CR0_REG);
  if(noiseFilter == MAX31856_NOISE_FILTER_50HZ) {
    t|=0x01;
  } else {
    t&=0xfe;
  }
  writeRegister8(MAX31856_CR0_REG, t);
}

/**************************************************************************/
/*!
    @brief  Sets the threshhold for thermocouple temperature range 
    for fault detection. NOT the internal chip temperature range!
    @param  flow Low (min) temperature, floating point
    @param  fhigh High (max) temperature, floating point
*/
/**************************************************************************/
void Adafruit_MAX31856::setTempFaultThreshholds(float flow, float fhigh) {
  int16_t low, high;

  flow *= 16;
  low = flow;

  fhigh *= 16;
  high = fhigh;

  writeRegister8(MAX31856_LTHFTH_REG, high >> 8);
  writeRegister8(MAX31856_LTHFTL_REG, high);

  writeRegister8(MAX31856_LTLFTH_REG, low >> 8);
  writeRegister8(MAX31856_LTLFTL_REG, low);
}

/**************************************************************************/
/*!
    @brief  Begin a one-shot (read temperature only upon request) measurement.
    Value must be read later, not returned here!
*/
/**************************************************************************/
void Adafruit_MAX31856::oneShotTemperature(void) {
  writeRegister8(MAX31856_CJTO_REG, 0x0);

  uint8_t t = readRegister8(MAX31856_CR0_REG);

  t &= ~MAX31856_CR0_AUTOCONVERT; // turn off autoconvert!
  t |= MAX31856_CR0_1SHOT;

  writeRegister8(MAX31856_CR0_REG, t);

  delay(250); // MEME FIX autocalculate based on oversampling
}

/**************************************************************************/
/*!
    @brief  Start a one-shot measurement and return internal chip temperature
    @returns Floating point temperature of chip in Celsius
*/
/**************************************************************************/
float Adafruit_MAX31856::readCJTemperature(void) {
  oneShotTemperature();

  int16_t temp16 = readRegister16(MAX31856_CJTH_REG);
  float tempfloat = temp16;
  tempfloat /= 256.0;

  return tempfloat;
}

/**************************************************************************/
/*!
    @brief  Start a one-shot measurement and return thermocouple tip temperature
    @returns Floating point temperature at end of thermocouple in Celsius
*/
/**************************************************************************/
float Adafruit_MAX31856::readThermocoupleTemperature(void) {
  oneShotTemperature();

  int32_t temp24 = readRegister24(MAX31856_LTCBH_REG);
  if (temp24 & 0x800000) {
    temp24 |= 0xFF000000;  // fix sign
  }

  temp24 >>= 5;  // bottom 5 bits are unused

  float tempfloat = temp24;
  tempfloat *= 0.0078125;

  return tempfloat;
}

/**********************************************/

uint8_t Adafruit_MAX31856::readRegister8(uint8_t addr) {
  uint8_t ret = 0;
  readRegisterN(addr, &ret, 1);

  return ret;
}

uint16_t Adafruit_MAX31856::readRegister16(uint8_t addr) {
  uint8_t buffer[2] = {0, 0};
  readRegisterN(addr, buffer, 2);

  uint16_t ret = buffer[0];
  ret <<= 8;
  ret |=  buffer[1];
  
  return ret;
}

uint32_t Adafruit_MAX31856::readRegister24(uint8_t addr) {
  uint8_t buffer[3] = {0, 0, 0};
  readRegisterN(addr, buffer, 3);

  uint32_t ret = buffer[0];
  ret <<= 8;
  ret |=  buffer[1];
  ret <<= 8;
  ret |=  buffer[2];
  
  return ret;
}


void Adafruit_MAX31856::readRegisterN(uint8_t addr, uint8_t buffer[], uint8_t n) {
  addr &= 0x7F; // make sure top bit is not set

  if (_sclk == -1)
    SPI.beginTransaction(max31856_spisettings);
  else 
    digitalWrite(_sclk, HIGH);

  digitalWrite(_cs, LOW);

  spixfer(addr);

  //Serial.print("$"); Serial.print(addr, HEX); Serial.print(": ");
  while (n--) {
    buffer[0] = spixfer(0xFF);
    //Serial.print(" 0x"); Serial.print(buffer[0], HEX);
    buffer++;
  }
  //Serial.println();

  if (_sclk == -1)
    SPI.endTransaction();

  digitalWrite(_cs, HIGH);
}


void Adafruit_MAX31856::writeRegister8(uint8_t addr, uint8_t data) {
  addr |= 0x80; // make sure top bit is set

  if (_sclk == -1)
    SPI.beginTransaction(max31856_spisettings);
  else 
    digitalWrite(_sclk, HIGH);

  digitalWrite(_cs, LOW);

  spixfer(addr);
  spixfer(data);

  //Serial.print("$"); Serial.print(addr, HEX); Serial.print(" = 0x"); Serial.println(data, HEX);

  if (_sclk == -1)
    SPI.endTransaction();

  digitalWrite(_cs, HIGH);
}



uint8_t Adafruit_MAX31856::spixfer(uint8_t x) {
  if (_sclk == -1)
    return SPI.transfer(x);

  // software spi
  //Serial.println("Software SPI");
  uint8_t reply = 0;
  for (int i=7; i>=0; i--) {
    reply <<= 1;
    digitalWrite(_sclk, LOW);
    digitalWrite(_mosi, x & (1<<i));
    digitalWrite(_sclk, HIGH);
    if (digitalRead(_miso))
      reply |= 1;
  }
  return reply;
}
