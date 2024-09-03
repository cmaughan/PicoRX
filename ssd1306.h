/*
MIT License

Copyright (c) 2021 David Schramm

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/** 
* @file ssd1306.h
* 
* simple driver for ssd1306 displays
*/

#ifndef _inc_ssd1306
#define _inc_ssd1306
#include <pico/stdlib.h>
#include <hardware/i2c.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
*	@brief defines commands used in ssd1306
*/
typedef enum {
    SET_CONTRAST = 0x81,
    SET_ENTIRE_ON = 0xA4,
    SET_NORM_INV = 0xA6,
    SET_DISP = 0xAE,
    SET_MEM_ADDR = 0x20,
    SET_COL_ADDR = 0x21,
    SET_PAGE_ADDR = 0x22,
    SET_DISP_START_LINE = 0x40,
    SET_SEG_REMAP = 0xA0,
    SET_MUX_RATIO = 0xA8,
    SET_COM_OUT_DIR = 0xC0,
    SET_DISP_OFFSET = 0xD3,
    SET_COM_PIN_CFG = 0xDA,
    SET_DISP_CLK_DIV = 0xD5,
    SET_PRECHARGE = 0xD9,
    SET_VCOM_DESEL = 0xDB,
    SET_CHARGE_PUMP = 0x8D
} ssd1306_command_t;

/**
*	@brief holds the configuration
*/
typedef struct {
    uint8_t width; 		/**< width of display */
    uint8_t height; 	/**< height of display */
    uint8_t pages;		/**< stores pages of display (calculated on initialization*/
    uint8_t address; 	/**< i2c address of display*/
    i2c_inst_t *i2c_i; 	/**< i2c connection instance */
    bool external_vcc; 	/**< whether display uses external vcc */ 
    uint8_t *buffer;	/**< display buffer */
    size_t bufsize;		/**< buffer size */
	uint8_t disp_col_offset; /**< 0 for ssd1306, 2 for sh1106 */
} ssd1306_t;

/**
*	@brief initialize display
*
*	@param[in] p : pointer to instance of ssd1306_t
*	@param[in] width : width of display
*	@param[in] height : heigth of display
*	@param[in] address : i2c address of display
*	@param[in] i2c_instance : instance of i2c connection
*	
* 	@return bool.
*	@retval true for Success
*	@retval false if initialization failed
*/
bool ssd1306_init(ssd1306_t *p, uint16_t width, uint16_t height, uint8_t address, i2c_inst_t *i2c_instance);

/**
*	@brief deinitialize display
*
*	@param[in] p : instance of display
*
*/
void ssd1306_deinit(ssd1306_t *p);

/**
*	@brief turn off display
*
*	@param[in] p : instance of display
*
*/
void ssd1306_poweroff(ssd1306_t *p);

/**
	@brief turn on display

	@param[in] p : instance of display

*/
void ssd1306_poweron(ssd1306_t *p);

/**
	@brief set contrast of display

	@param[in] p : instance of display
	@param[in] val : contrast

*/
void ssd1306_contrast(ssd1306_t *p, uint8_t val);

/**
	@brief set invert display

	@param[in] p : instance of display
	@param[in] inv : inv==0: disable inverting, inv!=0: invert

*/
void ssd1306_invert(ssd1306_t *p, uint8_t inv);

/**
	@brief set  display flip (180deg)

	@param[in] p : instance of display
	@param[in] flip : inv==0: disable flipping, inv!=0: flip

*/
void ssd1306_flip(ssd1306_t *p, uint8_t flip);

/**
	@brief set  display type ssd1306, sh1106

	@param[in] p : instance of display
	@param[in] type : ssd1306==0:  sh1106 !=0: type

*/
void ssd1306_type(ssd1306_t *p, uint8_t type);

/**
	@brief display buffer, should be called on change

	@param[in] p : instance of display

*/
void ssd1306_show(ssd1306_t *p);

/**
	@brief clear display buffer

	@param[in] p : instance of display

*/
void ssd1306_clear(ssd1306_t *p, bool colour);

/**
	@brief draw pixel on buffer

	@param[in] p : instance of display
	@param[in] x : x position
	@param[in] y : y position
	@param[in] colour : 1=white, 0=black
*/
void ssd1306_draw_pixel(ssd1306_t *p, uint32_t x, uint32_t y, bool colour);

/**
	@brief draw pixel on buffer

	@param[in] p : instance of display
	@param[in] x1 : x position of starting point
	@param[in] y1 : y position of starting point
	@param[in] x2 : x position of end point
	@param[in] y2 : y position of end point
	@param[in] colour : 1=white, 0=black
*/
void ssd1306_draw_line(ssd1306_t *p, int32_t x1, int32_t y1, int32_t x2, int32_t y2, bool colour);

/**
	@brief draw filled square at given position with given size

	@param[in] p : instance of display
	@param[in] x : x position of starting point
	@param[in] y : y position of starting point
	@param[in] width : width of square
	@param[in] height : height of square
	@param[in] colour : 1=white, 0=black
*/
void ssd1306_draw_square(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool colour);

/**
	@brief draw empty square at given position with given size

	@param[in] p : instance of display
	@param[in] x : x position of starting point
	@param[in] y : y position of starting point
	@param[in] width : width of square
	@param[in] height : height of square
	@param[in] colour : 1=white, 0=black
*/
void ssd13606_draw_empty_square(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool colour);

/**
	@brief draw monochrome bitmap with offset

	@param[in] p : instance of display
	@param[in] data : image data (whole file)
	@param[in] size : size of image data in bytes
	@param[in] x_offset : offset of horizontal coordinate
	@param[in] y_offset : offset of vertical coordinate
*/
void ssd1306_bmp_show_image_with_offset(ssd1306_t *p, const uint8_t *data, const long size, uint32_t x_offset, uint32_t y_offset);

/**
	@brief draw monochrome bitmap

	@param[in] p : instance of display
	@param[in] data : image data (whole file)
	@param[in] size : size of image data in bytes
*/
void ssd1306_bmp_show_image(ssd1306_t *p, const uint8_t *data, const long size);

/**
	@brief draw char with given font

	@param[in] p : instance of display
	@param[in] x : x starting position of char
	@param[in] y : y starting position of char
	@param[in] scale : scale font to n times of original size (default should be 1)
	@param[in] font : pointer to font
	@param[in] c : character to draw
	@param[in] colour : 1=white, 0=black
*/
void ssd1306_draw_char_with_font(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, const uint8_t *font, char c, bool colour);

/**
	@brief draw char with builtin font

	@param[in] p : instance of display
	@param[in] x : x starting position of char
	@param[in] y : y starting position of char
	@param[in] scale : scale font to n times of original size (default should be 1)
	@param[in] c : character to draw
	@param[in] colour : 1=white, 0=black
*/
void ssd1306_draw_char(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, char c, bool colour);

/**
	@brief draw string with given font

	@param[in] p : instance of display
	@param[in] x : x starting position of text
	@param[in] y : y starting position of text
	@param[in] scale : scale font to n times of original size (default should be 1)
	@param[in] font : pointer to font
	@param[in] s : text to draw
	@param[in] colour : 1=white, 0=black

*/
void ssd1306_draw_string_with_font(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, const uint8_t *font, const char *s, bool colour);

/**
	@brief draw string with builtin font

	@param[in] p : instance of display
	@param[in] x : x starting position of text
	@param[in] y : y starting position of text
	@param[in] scale : scale font to n times of original size (default should be 1)
	@param[in] s : text to draw
	@param[in] colour : 1=white, 0=black
*/
void ssd1306_draw_string(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, const char *s, bool colour);


#ifdef __cplusplus
}
#endif

#endif
