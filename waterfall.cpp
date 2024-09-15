#include "waterfall.h"

#include <cmath>
#include <cstdio>

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "fft_filter.h"
#include "ili934x.h"
#include "font_16x12.h"
#include "font_8x5.h"

waterfall::waterfall()
{
    //using ili9341 library from here:
    //https://github.com/bizzehdee/pico-libs

    #define PIN_MISO 12
    #define PIN_CS   13
    #define PIN_SCK  14
    #define PIN_MOSI 15 
    #define PIN_DC   11
    #define PIN_RST  10
    #define SPI_PORT spi1
    //spi_init(SPI_PORT, 62500000);
    spi_init(SPI_PORT, 40000000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_init(PIN_DC);
    gpio_set_dir(PIN_DC, GPIO_OUT);
    gpio_init(PIN_RST);
    gpio_set_dir(PIN_RST, GPIO_OUT);
    display = new ILI934X(SPI_PORT, PIN_CS, PIN_DC, PIN_RST, 240, 320, MIRRORED90DEG);
    display->reset();
    display->init();
    display->clear();

    //draw borders

    //Horizontal
    display->drawLine(31, 135, 288, 135, display->colour565(255,255,255));
    display->drawLine(31, 122, 288, 122, display->colour565(255,255,255));
    display->drawLine(31, 20,   288, 20, display->colour565(255,255,255));
    display->drawLine(31, 239, 288, 239, display->colour565(255,255,255));

    //Vertical
    display->drawLine(31,  20, 31,  122, display->colour565(255,255,255));
    display->drawLine(288, 20, 288, 122, display->colour565(255,255,255));
    display->drawLine(31,  135, 31,  239, display->colour565(255,255,255));
    display->drawLine(288, 135, 288, 239, display->colour565(255,255,255));

    //5kHz ticks
    for(uint16_t fbin=0; fbin<256; ++fbin)
    {
      if((fbin-128)%41==0)
      {
        display->drawLine(32+fbin, 122, 32+fbin, 123, COLOUR_WHITE);
      }
    }
    display->drawString(29,  127, font_8x5, "-15", COLOUR_WHITE, COLOUR_BLACK);
    display->drawString(70,  127, font_8x5, "-10", COLOUR_WHITE, COLOUR_BLACK);
    display->drawString(111, 127, font_8x5, "-5", COLOUR_WHITE, COLOUR_BLACK);
    display->drawString(154, 127, font_8x5, "-0", COLOUR_WHITE, COLOUR_BLACK);
    display->drawString(199, 127, font_8x5, "5", COLOUR_WHITE, COLOUR_BLACK);
    display->drawString(238, 127, font_8x5, "10", COLOUR_WHITE, COLOUR_BLACK);
    display->drawString(279, 127, font_8x5, "15", COLOUR_WHITE, COLOUR_BLACK);
}

waterfall::~waterfall()
{
    delete display;
}

uint16_t waterfall::heatmap(uint8_t value, bool blend, bool highlight)
{
    uint8_t section = ((uint16_t)value*6)>>8;
    uint8_t fraction = ((uint16_t)value*6)&0xff;

    //blend colour e.g. for cursor
    uint8_t blend_r = 0;
    uint8_t blend_g = 255;
    uint8_t blend_b = 0;

    uint8_t r=0, g=0, b=0;
    switch(section)
    {
      case 0: //black->blue
        r=0;
        g=0;
        b=fraction;
        break;
      case 1: //blue->cyan
        r=0;
        g=fraction;
        b=255;
        break;
      case 2: //cyan->green
        r=0;
        g=255;
        b=255-fraction;
        break;
      case 3: //green->yellow
        r=fraction;
        g=255;
        b=0;
        break;
      case 4: //yellow->red
        r=255;
        g=255-fraction;
        b=0;
        break;
      case 5: //red->white
        r=255;
        g=fraction;
        b=fraction;
        break;
    }
    
    if(blend)
    {
      r = (uint16_t)r-(r>>1) + (blend_r>>1);
      g = (uint16_t)g-(g>>1) + (blend_g>>1);
      b = (uint16_t)b-(b>>1) + (blend_b>>1);
    }

    if(highlight)
    {
      r = (uint16_t)(r>>2) + blend_r - (blend_r>>2);
      g = (uint16_t)(g>>2) + blend_g - (blend_g>>2);
      b = (uint16_t)(b>>2) + blend_b - (blend_b>>2);
    }

    return display->colour565(r,g,b);
}

void waterfall::update_spectrum(rx &receiver, rx_settings &settings, rx_status &status, uint8_t spectrum[], uint8_t dB10)
{

    const uint16_t waterfall_height = 100u;
    const uint16_t waterfall_x = 32u;
    const uint16_t waterfall_y = 136u;
    const uint16_t num_cols = 256u;
    const uint16_t scope_height = 100u;
    const uint16_t scope_x = 32u;
    const uint16_t scope_y = 21u;
    const uint16_t scope_fg = display->colour565(255, 255, 255);

    enum FSM_states{
      update_waterfall,
      draw_waterfall, 
      draw_scope, 
      draw_frequency
    };
    static uint16_t top_row = 0u;
    static uint8_t waterfall_row = 0u;
    static uint8_t scope_col = 0u;
    static FSM_states FSM_state = update_waterfall;

    if(FSM_state == update_waterfall)
    {
      //scroll waterfall
      top_row = top_row?top_row-1:waterfall_height-1;

      //add new line to waterfall
      for(uint16_t col=0; col<num_cols; col++)
      {
        waterfall_buffer[top_row][col] = spectrum[col];
      }

      FSM_state = draw_waterfall;

    } else if(FSM_state == draw_waterfall ){
    
      //draw one line of waterfall
      uint16_t line[num_cols];
      uint16_t row_address = (top_row+waterfall_row)%waterfall_height;
      for(uint16_t col=0; col<num_cols; ++col)
      {
         const int16_t fbin = col-128;
         const bool is_usb_col = (fbin > status.filter_config.start_bin) && (fbin < status.filter_config.stop_bin) && status.filter_config.upper_sideband;
         const bool is_lsb_col = (-fbin > status.filter_config.start_bin) && (-fbin < status.filter_config.stop_bin) && status.filter_config.lower_sideband;
         const bool is_passband = is_usb_col || is_lsb_col;

         uint8_t heat = waterfall_buffer[row_address][col];
         uint16_t colour=heatmap(heat, is_passband, fbin==0);
         line[col] = colour;
      }
      display->writeHLine(waterfall_x, waterfall_row+waterfall_y, num_cols, line);

      if(waterfall_row == waterfall_height-1)
      {
        FSM_state = draw_scope;
        waterfall_row = 0;
      } else {
        ++waterfall_row;
      }

    } else if(FSM_state == draw_scope) {

      //draw scope one bar at a time

      uint8_t data_point = (scope_height * (uint16_t)waterfall_buffer[top_row][scope_col])/270;
      uint16_t vline[scope_height];
  
      const int16_t fbin = scope_col-128;
      const bool is_usb_col = (fbin > status.filter_config.start_bin) && (fbin < status.filter_config.stop_bin) && status.filter_config.upper_sideband;
      const bool is_lsb_col = (-fbin > status.filter_config.start_bin) && (-fbin < status.filter_config.stop_bin) && status.filter_config.lower_sideband;
      const bool is_passband = is_usb_col || is_lsb_col;
      const bool col_is_tick = (fbin%41 == 0) && fbin;


      for(uint8_t row=0; row<scope_height; ++row)
      {
        const bool row_is_tick = (row%(4*scope_height*dB10/270)) == 0;
        if(row < data_point)
        {
          vline[scope_height - 1 - row] = heatmap((uint16_t)row*256/scope_height, is_passband);
        }
        else if(row == data_point)
        {
          vline[scope_height - 1 - row] = scope_fg;
        }
        else
        {
          uint16_t colour = heatmap(0, is_passband, fbin==0);
          colour = col_is_tick?COLOUR_GRAY:colour;
          colour = row_is_tick?COLOUR_GRAY:colour;
          vline[scope_height - 1 - row] = colour;
        }
      }
      display->writeVLine(scope_x+scope_col, scope_y, scope_height, vline);

      if(scope_col == num_cols-1)
      {
        //scroll waterfall
        FSM_state = draw_frequency;
        scope_col = 0;
      } else {
        ++scope_col;
      }

    } else if(FSM_state == draw_frequency) {

      //extract frequency from status
      uint32_t remainder, MHz, kHz, Hz;
      MHz = (uint32_t)settings.tuned_frequency_Hz/1000000u;
      remainder = (uint32_t)settings.tuned_frequency_Hz%1000000u; 
      kHz = remainder/1000u;
      remainder = remainder%1000u; 
      Hz = remainder;

      //update frequency if changed
      static uint32_t lastMHz = 0;
      static uint32_t lastkHz = 0;
      static uint32_t lastHz = 0;
      if(lastMHz!=MHz || lastkHz!=kHz || lastHz!=Hz)
      {
        char buffer[20];
        snprintf(buffer, 20, "%2lu.%03lu.%03lu", MHz, kHz, Hz);
        display->drawString(100, 0, font_16x12, buffer, COLOUR_WHITE, COLOUR_BLACK);
        lastMHz = MHz;
        lastkHz = kHz;
        lastHz = Hz;
      }
      FSM_state = update_waterfall;
    }
}
