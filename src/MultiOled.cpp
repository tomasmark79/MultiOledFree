// MIT License
// Copyright (c) 2024-2025 Tomáš Mark

#include <MultiOled/MultiOled.hpp>
#include <Logger/Logger.hpp>
#include <Utils/Utils.hpp>

extern "C" {
#include <bcm2835.h>
#include <raspi_gpio_hal.h>
#include <u8g2.h>
}

#include <xbitmaps/dotname.h>
#include <xbitmaps/vortex.h>

#include <bitset>

// Multiplexer TCA9548A address

#define TCA9548A_ADDR 0x70
#define OLEDS 8

namespace dotname {

  MultiOled::MultiOled () {
    LOG_D_STREAM << libName_ << " ...constructed" << std::endl;
  }
  MultiOled::MultiOled (const std::filesystem::path& assetsPath) : MultiOled () {
    assetsPath_ = assetsPath;
    if (!assetsPath_.empty ()) {
      LOG_D_STREAM << "Assets path: " << assetsPath_ << std::endl;
      this->oledTester ();
    } else {
      LOG_D_STREAM << "Assets path is empty" << std::endl;
    }
  }
  MultiOled::~MultiOled () {
    LOG_D_STREAM << libName_ << " ...destructed" << std::endl;
  }

  void MultiOled::oledTester () {
    u8g2_t* oled = new u8g2_t[OLEDS];
    char buf[32], buf2[32];
    int counter = 0;

    std::string getCurrentTimeStr;

    while (true) {
      for (int i = 0; i < OLEDS; i++) {
        // Switch to channel 0 and initialize the first OLED display
        selectI2CChannel (i);

        if (i == 0 || i == 1)
          u8g2_Setup_ssd1306_i2c_128x64_noname_f (&oled[i], U8G2_R0, u8x8_byte_sw_i2c,
                                                  u8x8_gpio_and_delay_raspi_gpio_hal);
        else
          u8g2_Setup_ssd1306_i2c_128x64_noname_f (&oled[i], U8G2_R2, u8x8_byte_sw_i2c,
                                                  u8x8_gpio_and_delay_raspi_gpio_hal);

        // Set I2C pins - GPIO 3 and 2
        u8x8_SetPin (u8g2_GetU8x8 (&oled[i]), U8X8_PIN_I2C_CLOCK, 3);
        u8x8_SetPin (u8g2_GetU8x8 (&oled[i]), U8X8_PIN_I2C_DATA, 2);

        sprintf (buf, "Oled: %d step: %d", i + 1, counter);

        u8g2_InitDisplay (&oled[i]);
        u8g2_SetPowerSave (&oled[i], 0);
        u8g2_ClearBuffer (&oled[i]);

        // Random invert the display in current iteration
        int r = rand () % 2;
        if (r == 0)
          u8g2_SetDrawColor (&oled[i], 1);
        else
          u8g2_SetDrawColor (&oled[i], 0);

        // change the bitmap
        int r2 = rand () % 2;
        if (r2 == 0)
          u8g2_DrawXBMP (&oled[i], 0, 0, vortex_width, vortex_height, (const uint8_t*)vortex_bits);
        else
          u8g2_DrawXBMP (&oled[i], 0, 0, dotname_width, dotname_height,
                         (const uint8_t*)dotname_bits);

        u8g2_SetFont (&oled[i], u8g2_font_6x10_tf);
        u8g2_DrawStr (&oled[i], 0, 11, buf);

        getCurrentTimeStr = this->getCurrentTimeStr ();
        u8g2_SetFont (&oled[i], u8g2_font_6x10_tf);
        u8g2_DrawStr (&oled[i], 0, 58, getCurrentTimeStr.c_str ());

        u8g2_SendBuffer (&oled[i]);

        LOG_D_STREAM << "Displaying on OLED " << i;
        LOG_D_STREAM << " Counter: " << counter;
        LOG_D_STREAM << " I2C Channel: " << i;
        LOG_D_STREAM << " I2C Address: " << std::hex << TCA9548A_ADDR;
        LOG_D_STREAM << " I2C Data: " << std::hex << (1 << i);
        LOG_D_STREAM << " I2C Data (binary): " << std::bitset<8> (1 << i);
        LOG_D_STREAM << " I2C Data (decimal): " << std::dec << (1 << i);
        LOG_D_STREAM << " I2C Data (hex): " << std::hex << (1 << i);
        LOG_D_STREAM << " I2C Data (octal): " << std::oct << (1 << i);
        LOG_D_STREAM << std::endl;

        counter++;
      }
    }

    delete[] oled;
  }

  // TCA9548A
  int MultiOled::selectI2CChannel (int channel) {
    if (channel < 0 || channel > 7) {
      LOG_E_STREAM << "Invalid I2C channel: " << channel << std::endl;
      return 1;
    }

    uint8_t data = 1 << channel;

    if (bcm2835_init () == 0) {
      LOG_E_STREAM << "Error: bcm2835 initialization failed!" << std::endl;
      return 1;
    }

    if (bcm2835_i2c_begin () == 0) {
      LOG_E_STREAM << "Error: Cannot start I2C! (Root Access is required)" << std::endl;
      bcm2835_close ();
      return 1;
    }

    bcm2835_i2c_setSlaveAddress (TCA9548A_ADDR);
    bcm2835_i2c_set_baudrate (100000);

    if (bcm2835_i2c_write ((char*)&data, 1) != 0) {
      LOG_E_STREAM << "Error: Cannot set multiplexer to channel " << channel << std::endl;
      return 1;
    }

    bcm2835_delay (2); // Short delay for stabilization

    return 0;
  }

  std::string MultiOled::getCurrentTimeStr () {
    time_t now = time (0);
    tm* ltm = localtime (&now);
    // Format the time as a string
    std::string timeStr = std::to_string (ltm->tm_hour) + ":" + std::to_string (ltm->tm_min) + ":"
                          + std::to_string (ltm->tm_sec);
    return timeStr;
  }

} // namespace dotname