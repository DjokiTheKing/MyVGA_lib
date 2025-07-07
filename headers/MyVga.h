#ifndef MY_VGA_H
#define MY_VGA_H

#include <iostream>
#include <cstdint>
#include <string>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/structs/bus_ctrl.h"

// ---------- PIOS ----------

// ---------- SYNC's ----------

// 1080p base
#include "hsync_1080p.pio.h"
#include "vsync_1080p.pio.h"

// 900p base
#include "hsync_900p.pio.h"
#include "vsync_900p.pio.h"

// 768p_16x9 base
#include "hsync_768p_16x9.pio.h"
#include "vsync_768p_16x9.pio.h"

// 720p base
#include "hsync_720p.pio.h"
#include "vsync_720p.pio.h"

// 768p base
#include "hsync_768p.pio.h"
#include "vsync_768p.pio.h"

// 600p base
#include "hsync_600p.pio.h"
#include "vsync_600p.pio.h"

// 480p base
#include "hsync_480p.pio.h"
#include "vsync_480p.pio.h"

// ---------- RGB's ----------
// ---- 4 CLOCKS PER PIXEL ---

//    4 BITS PER PIXEL
#include "rgb_x4_4bpp.pio.h" // 4 bits per pixel x1 speed
#include "rgb_x4_4bpp_05x.pio.h" // 0.5x speed
#include "rgb_x4_4bpp_2x.pio.h" // 2x speed
#include "rgb_x4_4bpp_4x.pio.h" // 4x speed

//     1 BIT PER PIXEL
#include "rgb_x4_1bpp.pio.h" // x1 speed
#include "rgb_x4_1bpp_05x.pio.h" // 0.5x speed
#include "rgb_x4_1bpp_2x.pio.h" // 2x speed
#include "rgb_x4_1bpp_4x.pio.h" // 4x speed

// ---- 3 CLOCKS PER PIXEL ---

//    4 BITS PER PIXEL
#include "rgb_x3_4bpp.pio.h" // 3 clocks per pixel 4 bits per pixel
#include "rgb_x3_4bpp_3x.pio.h" // 4 clocks per pixel 1 bit  per pixel 3x clock speed fix
#include "rgb_x3_4bpp_05x.pio.h" // 4 clocks per pixel 1 bit  per pixel 3x clock speed fix

//     1 BIT PER PIXEL
#include "rgb_x3_1bpp.pio.h" // 3 clocks per pixel 1 bit  per pixel
#include "rgb_x3_1bpp_3x.pio.h" // 4 clocks per pixel 1 bit  per pixel 3x clock speed fix
#include "rgb_x3_1bpp_05x.pio.h" // 4 clocks per pixel 1 bit  per pixel 3x clock speed fix


// ---------- PIOS END ----------

#ifdef FREERTOS_CONFIG_H
    #include "FreeRTOS.h"
    #include "task.h"
    #include "semphr.h"
#endif

#ifndef MAX_DISPLAY_RES
    #define MAX_DISPLAY_RES MAX_RES_1920X1080
#endif

#include "Font.h"

template<int _bits_per_pixel>
struct Color;


template<>
struct Color<1> {
    uint8_t r, g, b;

    uint8_t __not_in_flash_func(return_color)(){
        if(r > 126 || g > 126 || b > 126) return 1;
        else return 0;
    }
};

template<>
struct Color<4> {
    uint8_t r, g, b;

    uint8_t __not_in_flash_func(return_color)() {
        uint8_t res_color = 0;
        if(r > 126) res_color |= 1 << 3;
        if(b > 126) res_color |= 1 << 2;
        if(g > 25 && g <= 95) res_color |= 1;
        else if(g > 95 && g <= 171) res_color |= 1 << 1;
        else if(g > 171) res_color |= 3;
        return res_color;
    }
};

enum MaxDisplayResolutions{
            MAX_RES_640X480=0,
            MAX_RES_800X600,
            MAX_RES_1024X768,
            MAX_RES_1280X720,
            MAX_RES_1366X768,
            MAX_RES_1600X900,
            MAX_RES_1920X1080
};

/// @brief VGA class for adding vga functionality to rp2040
/// @tparam _width wanted width of the screen
/// @tparam _height wanted height of the screen
/// @tparam _bits_per_pixel bits per pixel (only 1 and 4 supported currently)
/// @tparam _num_buffers number of buffers (only 1 and 2 supported currently)
template <uint16_t _width=0, uint16_t _height=0, uint16_t _bits_per_pixel=1, uint16_t _num_buffers=1, uint8_t _pio_num=0>
class MyVga{
    public:
        
        /// @brief ColorType for this instance with this many bits per pixel
        using ColorType = Color<_bits_per_pixel>;

        /// @brief Initialize the class with the first pin for vga, pin order: hsync, vsync, green, blue, red; First go the low order bits then high
        /// @param start_pin first of the pins (hsync pin)
        MyVga(uint8_t start_pin);

        /// @brief Initialize dma's, pio's and irq's, and start them. 
        /// @brief If using FreeRTOS call this inside a task, the interrupts will be pinned to that task. 
        void initVga();

        /// @brief Draw a single pixel at specified position with specified color. 
        /// @param x horizontal position of the pixel
        /// @param y vertical position of the pixel
        /// @param color color of the pixel
        void __not_in_flash_func(drawPixel)(uint16_t x, uint16_t y, ColorType color);
        
        /// @brief Draw a horizontal line with the specified (x, y) left start point, length, color and thickness
        /// @param x horizontal position of the left start point of the line
        /// @param y vertical position of the line
        /// @param length length of the line
        /// @param color color of the line 
        /// @param thickness thickness of the line
        void __not_in_flash_func(drawHorizontalLine)(uint16_t x, uint16_t y, uint16_t length, ColorType color, uint16_t thickness=1);

        /// @brief Draw a vertical line with the specified (x, y) top start point, length, color and thickness
        /// @param x horizontal position of the line
        /// @param y vertical position of the top start point of the line
        /// @param length length of the line
        /// @param color color of the line 
        /// @param thickness thickness of the line
        void __not_in_flash_func(drawVerticalLine)(uint16_t x, uint16_t y, uint16_t length, ColorType color, uint16_t thickness=1);

        /// @brief Draw a rectangle with top left at (x, y) with the specified width, height and color, and walls of the specified thickness
        /// @param x horizontal position of top left corner
        /// @param y vertical position of top left corner
        /// @param width width of the rectangle
        /// @param height height of the rectangle
        /// @param color color of the rectangle 
        /// @param thickness thickness of the walls
        void __not_in_flash_func(drawRect)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, ColorType color, uint16_t thickness=1);

        /// @brief Draw a filled rectangle with top left at (x, y) with the specified width, height and color
        /// @param x horizontal position of top left corner
        /// @param y vertical position of top left corner
        /// @param width width of the rectangle
        /// @param height height of the rectangle
        /// @param color color of the rectangle 
        void __not_in_flash_func(fillRect)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, ColorType color);

        /// @brief Draw a line from x0,y0 to x1,y1 with the specified color and thickness
        /// @param x0 start x
        /// @param y0 start y
        /// @param x1 end x
        /// @param y1 end y
        /// @param color color of the line
        void __not_in_flash_func(drawLine)(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, ColorType color);

        /// @brief Draw a circle with the center at x0 and y0 and specified radius and color
        /// @param x center x
        /// @param y center y 
        /// @param radius radius of the circle
        /// @param color color of the circle
        void __not_in_flash_func(drawCircle)(uint16_t x, uint16_t y, uint16_t radius, ColorType color);

        /// @brief Draw a filled circle with the center at x0 and y0 and specified radius and color
        /// @param x center x
        /// @param y center y 
        /// @param radius radius of the circle
        /// @param color color of the circle
        void __not_in_flash_func(fillCircle)(uint16_t x, uint16_t y, uint16_t radius, ColorType color);

        /// @brief Clears the whole display (sets the color to black)
        void __not_in_flash_func(clearDisplay)();

        /// @brief Waits for vsync trigger from an irq.
        /// @brief If using FreeRTOS call this inside a task.
        void __not_in_flash_func(wait_for_Vsync)();


        /// @brief Copies contents of the backbuffer to the front buffer
        void __not_in_flash_func(copy_back_to_front)();

        /// @brief Set the font 
        /// @param font pointer to a font object
        void setFont(Font* font);

        /// @brief Set the cursor position, cursor is the top left position of the text
        /// @param x topleft x
        /// @param y topleft y
        void setCursor(uint16_t x, uint16_t y);

        /// @brief Set the text color
        /// @param color color
        void setTextColor(ColorType color);

        /// @brief Print the text with the currently set parameters
        /// @param text text
        void __not_in_flash_func(print)(const std::string& text);

        static constexpr uint16_t display_width = (_width >> (_bits_per_pixel == 1 ? 3 : 1))<<(_bits_per_pixel == 1 ? 3 : 1);
        static constexpr uint16_t display_height = _height;

    private: // private Variables
        uint8_t frame_buffer[_height*_width*_bits_per_pixel/8*_num_buffers];
        
        const uint32_t frame_buffer_size = (_height*_width*_bits_per_pixel/8);

        volatile uint8_t *back_buffer, *front_buffer, *tmp_buffer;
        volatile uint8_t *buffer_pointer;

        Font* current_font = nullptr;
        uint16_t cursor_x = 0, cursor_y = 0;
        ColorType text_color = {255,255,255};

        volatile uint16_t currentScanLine;
        volatile bool vsync_ready;

        uint8_t _start_pin;

        PIO pio_vga;
        uint32_t dma_chan_primary, dma_chan_reset;
        uint32_t H_ACTIVEPORCH, V_ACTIVE, line_divisor;

        pio_program hsync_program, vsync_program, rgb_program;
        uint8_t hsync_wrap_target, vsync_wrap_target, rgb_wrap_target;
        uint8_t hsync_wrap, vsync_wrap, rgb_wrap;
        uint8_t hsync_clock_divider, vsync_clock_divider, rgb_clock_divider;

        const uint32_t TXCOUNT = _width*_bits_per_pixel/8;
        const uint32_t RGB_ACTIVE = TXCOUNT-1;

        struct vga_resolution{
            uint32_t width, height, H_ACTIVEPORCH, V_ACTIVE, PIXEL_CLOCK;
        };

        static constexpr uint8_t number_of_supported_resolutions = 7;
        static constexpr vga_resolution supported_base_resolutions[7] = {
            {640,  480,  640 + 16,  480,  25175000},
            {800,  600,  800 + 40,  600,  40000000},
            {1024, 768,  1024+ 24,  768,  65000000},
            {1280, 720,  1280+110,  720,  74250000},
            {1366, 768,  1366+ 14,  768,  67152000},
            {1600, 900,  1600+ 24,  900, 108000000},
            {1920, 1080, 1920+ 88, 1080, 148500000}
        };

        static inline MyVga* self;
        
    private: // private Functions
        static void __isr __not_in_flash_func(dma_IRQ0_handeler)();
        static void __isr __not_in_flash_func(pio_IRQ0_handeler)();

        void __not_in_flash_func(memset_volatile)(volatile unsigned char *s, unsigned char c, unsigned int n);
        void __not_in_flash_func(memset_volatile)(volatile unsigned char *s, unsigned char c, unsigned int n, unsigned int offset);
        void __not_in_flash_func(memcpy_volatile)(volatile unsigned char *d, volatile unsigned char *s, unsigned int n);

        constexpr uint64_t vector_dot(uint64_t x, uint64_t y){ return x*x+y*y; }
        constexpr uint8_t find_best_resolution_match();
        constexpr uint8_t find_best_resolution_divisor(uint8_t closest_match);
        constexpr bool check_if_exact_resolution(uint8_t closest_match, uint8_t closest_divisor);

        constexpr uint8_t find_clock_divisor(uint32_t target_clock);
        constexpr uint8_t find_rgb_clock_divisor(uint8_t clock_divisor, uint8_t divisor);
        constexpr uint8_t find_rgb_clocks_per_pixel(uint8_t clock_divisor, uint8_t divisor);
};

#include "MyVga.hpp"

#endif // MY_VGA_H