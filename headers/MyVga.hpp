#ifndef MY_VGA_IMPL_H
#define MY_VGA_IMPL_H

#include "MyVga.h"

/// @brief Initialize the class with the first pin for vga, pin order: hsync, vsync, green, blue, red; First go the low order bits then high
/// @param start_pin first of the pins (hsync pin)
template <uint16_t _width, uint16_t _height, uint16_t _bits_per_pixel, uint16_t _num_buffers, uint8_t _pio_num>
inline MyVga<_width, _height, _bits_per_pixel, _num_buffers, _pio_num>::MyVga(uint8_t start_pin)
:
currentScanLine(0),
_start_pin(start_pin),
vsync_ready(false)
{
    if constexpr(_pio_num == 0)
    {
        pio_vga = pio0;
    }
    else if constexpr(_pio_num == 1)
    {
        pio_vga = pio1;
    }
    #ifdef PICO_RP2350
    else if constexpr(_pio_num == 2){
        pio_vga = pio2;
    }
    #endif 
    else{
        static_assert(false, "pio chosen doesn't exist");
    }
    constexpr uint8_t closest_res = find_best_resolution_match();
    constexpr uint8_t divisor = find_best_resolution_divisor(closest_res);

    // static_assert(closest_res < 0, "test");
    // static_assert(divisor < 0, "test");

    static_assert(check_if_exact_resolution(closest_res, divisor), "Invalid resolution: must be an exact quotient of a base resolution. (Check MaxDisplayResolutions)");

    constexpr uint8_t clock_divisor = find_clock_divisor(supported_base_resolutions[closest_res].PIXEL_CLOCK);

    static_assert(clock_divisor > 0, "Unsupported system clock: system clock must be a multiple of the targeted pixel clock. +- 0.5%");

    constexpr uint8_t rgb_clock_divisor = find_rgb_clock_divisor(clock_divisor, divisor);
    static_assert(rgb_clock_divisor > 0, "Unsupported system clock: minimum pixel time is 3 clocks, if using pixel density of more than 1/3 of the resolution the system clock must be at least double of the pixel clock.");
    
    constexpr uint8_t clocks_per_pixel = find_rgb_clocks_per_pixel(clock_divisor, divisor);

    static_assert(clocks_per_pixel > 0, "test");
    
    hsync_clock_divider = clock_divisor;
    vsync_clock_divider = clock_divisor;
    rgb_clock_divider = rgb_clock_divisor;

    line_divisor = divisor;

    if constexpr(_bits_per_pixel == 1){
        if constexpr(clocks_per_pixel == 4){
            if constexpr(rgb_clock_divisor == clock_divisor){
                rgb_program = rgb_x4_1bpp_program;
                rgb_wrap_target = rgb_x4_1bpp_wrap_target;
                rgb_wrap = rgb_x4_1bpp_wrap;
            }else if constexpr(rgb_clock_divisor*2 == clock_divisor){
                rgb_program = rgb_x4_1bpp_2x_program;
                rgb_wrap_target = rgb_x4_1bpp_2x_wrap_target;
                rgb_wrap = rgb_x4_1bpp_2x_wrap;
            }else if constexpr(rgb_clock_divisor*4 == clock_divisor){
                rgb_program = rgb_x4_1bpp_4x_program;
                rgb_wrap_target = rgb_x4_1bpp_4x_wrap_target;
                rgb_wrap = rgb_x4_1bpp_4x_wrap;
            }else if constexpr(rgb_clock_divisor/2 == clock_divisor){
                rgb_program = rgb_x4_1bpp_05x_program;
                rgb_wrap_target = rgb_x4_1bpp_05x_wrap_target;
                rgb_wrap = rgb_x4_1bpp_05x_wrap;
            }
        }else if constexpr(clocks_per_pixel == 3){
            if constexpr(rgb_clock_divisor == clock_divisor){
                rgb_program = rgb_x3_1bpp_program;
                rgb_wrap_target = rgb_x3_1bpp_wrap_target;
                rgb_wrap = rgb_x3_1bpp_wrap;
            }else if constexpr(rgb_clock_divisor*3 == clock_divisor){
                rgb_program = rgb_x3_1bpp_3x_program;
                rgb_wrap_target = rgb_x3_1bpp_3x_wrap_target;
                rgb_wrap = rgb_x3_1bpp_3x_wrap;
            }else if constexpr(rgb_clock_divisor/2 == clock_divisor){
                rgb_program = rgb_x3_1bpp_05x_program;
                rgb_wrap_target = rgb_x3_1bpp_05x_wrap_target;
                rgb_wrap = rgb_x3_1bpp_05x_wrap;
            }
        }else{
            static_assert(false, "Unsupported system clock: minimum pixel time is 3 clocks, if using pixel density of more than 1/3 of the resolution the system clock must be at least double of the pixel clock.");
        }
    }else if constexpr(_bits_per_pixel == 4){
        if constexpr(clocks_per_pixel == 4){
            if constexpr(rgb_clock_divisor == clock_divisor){
                rgb_program = rgb_x4_4bpp_program;
                rgb_wrap_target = rgb_x4_4bpp_wrap_target;
                rgb_wrap = rgb_x4_4bpp_wrap;
            }else if constexpr(rgb_clock_divisor*2 == clock_divisor){
                rgb_program = rgb_x4_4bpp_2x_program;
                rgb_wrap_target = rgb_x4_4bpp_2x_wrap_target;
                rgb_wrap = rgb_x4_4bpp_2x_wrap;
            }else if constexpr(rgb_clock_divisor*4 == clock_divisor){
                rgb_program = rgb_x4_4bpp_4x_program;
                rgb_wrap_target = rgb_x4_4bpp_4x_wrap_target;
                rgb_wrap = rgb_x4_4bpp_4x_wrap;
            }else if constexpr(rgb_clock_divisor/2 == clock_divisor){
                rgb_program = rgb_x4_4bpp_05x_program;
                rgb_wrap_target = rgb_x4_4bpp_05x_wrap_target;
                rgb_wrap = rgb_x4_4bpp_05x_wrap;
            }
        }else if constexpr(clocks_per_pixel == 3){
            if constexpr(rgb_clock_divisor == clock_divisor){
                rgb_program = rgb_x3_4bpp_program;
                rgb_wrap_target = rgb_x3_4bpp_wrap_target;
                rgb_wrap = rgb_x3_4bpp_wrap;
            }else if constexpr(rgb_clock_divisor*3 == clock_divisor){
                rgb_program = rgb_x3_4bpp_3x_program;
                rgb_wrap_target = rgb_x3_4bpp_3x_wrap_target;
                rgb_wrap = rgb_x3_4bpp_3x_wrap;
            }else if constexpr(rgb_clock_divisor/2 == clock_divisor){
                rgb_program = rgb_x3_4bpp_05x_program;
                rgb_wrap_target = rgb_x3_4bpp_05x_wrap_target;
                rgb_wrap = rgb_x3_4bpp_05x_wrap;
            }
        }else{
            static_assert(false, "Unsupported system clock: minimum pixel time is 3 clocks, if using pixel density of more than 1/3 of the resolution the system clock must be at least double of the pixel clock.");
        }
    }else{
        static_assert(_bits_per_pixel == 1 || _bits_per_pixel == 4, "Invalid _bits_per_pixel: currnetly only 1 and 4 is supported.");
    }

    if constexpr(closest_res == 0){
        H_ACTIVEPORCH = supported_base_resolutions[closest_res].H_ACTIVEPORCH-1;
        V_ACTIVE = supported_base_resolutions[closest_res].V_ACTIVE-1;

        hsync_program = hsync_480p_program;
        hsync_wrap_target = hsync_480p_wrap_target;
        hsync_wrap = hsync_480p_wrap;

        vsync_program = vsync_480p_program;
        vsync_wrap_target = vsync_480p_wrap_target;
        vsync_wrap = vsync_480p_wrap;
    }else if constexpr(closest_res == 1){
        H_ACTIVEPORCH = supported_base_resolutions[closest_res].H_ACTIVEPORCH-1;
        V_ACTIVE = supported_base_resolutions[closest_res].V_ACTIVE-1;

        hsync_program = hsync_600p_program;
        hsync_wrap_target = hsync_600p_wrap_target;
        hsync_wrap = hsync_600p_wrap;

        vsync_program = vsync_600p_program;
        vsync_wrap_target = vsync_600p_wrap_target;
        vsync_wrap = vsync_600p_wrap;
    }else if constexpr(closest_res == 2){
        H_ACTIVEPORCH = supported_base_resolutions[closest_res].H_ACTIVEPORCH-1;
        V_ACTIVE = supported_base_resolutions[closest_res].V_ACTIVE-1;

        hsync_program = hsync_768p_program;
        hsync_wrap_target = hsync_768p_wrap_target;
        hsync_wrap = hsync_768p_wrap;

        vsync_program = vsync_768p_program;
        vsync_wrap_target = vsync_768p_wrap_target;
        vsync_wrap = vsync_768p_wrap;
    }else if constexpr(closest_res == 3){
        H_ACTIVEPORCH = supported_base_resolutions[closest_res].H_ACTIVEPORCH-1;
        V_ACTIVE = supported_base_resolutions[closest_res].V_ACTIVE-1;

        hsync_program = hsync_720p_program;
        hsync_wrap_target = hsync_720p_wrap_target;
        hsync_wrap = hsync_720p_wrap;

        vsync_program = vsync_720p_program;
        vsync_wrap_target = vsync_720p_wrap_target;
        vsync_wrap = vsync_720p_wrap;
    }else if constexpr(closest_res == 4){
        H_ACTIVEPORCH = supported_base_resolutions[closest_res].H_ACTIVEPORCH-1;
        V_ACTIVE = supported_base_resolutions[closest_res].V_ACTIVE-1;

        hsync_program = hsync_1080p_program;
        hsync_wrap_target = hsync_1080p_wrap_target;
        hsync_wrap = hsync_1080p_wrap;

        vsync_program = vsync_1080p_program;
        vsync_wrap_target = vsync_1080p_wrap_target;
        vsync_wrap = vsync_1080p_wrap;
    }

    self = this;
    if constexpr(_num_buffers == 2){
        front_buffer = &frame_buffer[0];
        back_buffer = &frame_buffer[frame_buffer_size];
    }else if constexpr(_num_buffers == 1){
        front_buffer = &frame_buffer[0];
        back_buffer = &frame_buffer[0];
    }
    vga_address_pointer = &front_buffer[0];
}

template <uint16_t _width, uint16_t _height, uint16_t _bits_per_pixel, uint16_t _num_buffers, uint8_t _pio_num>
inline void MyVga<_width, _height, _bits_per_pixel, _num_buffers, _pio_num>::initVga()
{
    bus_ctrl_hw->priority = BUSCTRL_BUS_PRIORITY_DMA_R_BITS | BUSCTRL_BUS_PRIORITY_DMA_W_BITS;

    uint hsync_offset = pio_add_program(pio_vga, &hsync_program);
    uint vsync_offset = pio_add_program(pio_vga, &vsync_program);
    uint rgb_offset = pio_add_program(pio_vga, &rgb_program);

    uint hsync_sm = pio_claim_unused_sm(pio_vga, true);
    uint vsync_sm = pio_claim_unused_sm(pio_vga, true);
    uint rgb_sm = pio_claim_unused_sm(pio_vga, true);

    pio_sm_config hsync_sm_config = pio_get_default_sm_config();

    sm_config_set_wrap(&hsync_sm_config, hsync_offset + hsync_wrap_target, hsync_offset + hsync_wrap);
    sm_config_set_set_pins(&hsync_sm_config, _start_pin, 1);
    sm_config_set_clkdiv(&hsync_sm_config, hsync_clock_divider);

    pio_gpio_init(pio_vga, _start_pin);

    pio_sm_set_consecutive_pindirs(pio_vga, hsync_sm, _start_pin, 1, true);
    pio_sm_init(pio_vga, hsync_sm, hsync_offset, &hsync_sm_config);

    pio_sm_config vsync_sm_config = pio_get_default_sm_config();

    sm_config_set_wrap(&vsync_sm_config, vsync_offset + vsync_wrap_target, vsync_offset + vsync_wrap);
    sm_config_set_sideset(&vsync_sm_config, 2, true, false);
    sm_config_set_set_pins(&vsync_sm_config, _start_pin+1, 1);
    sm_config_set_sideset_pins(&vsync_sm_config, _start_pin+1);
    sm_config_set_clkdiv(&vsync_sm_config, vsync_clock_divider);

    pio_gpio_init(pio_vga, _start_pin+1);

    pio_sm_set_consecutive_pindirs(pio_vga, vsync_sm, _start_pin+1, 1, true);
    pio_sm_init(pio_vga, vsync_sm, vsync_offset, &vsync_sm_config);

    pio_sm_config rgb_sm_config = pio_get_default_sm_config();

    sm_config_set_wrap(&rgb_sm_config, rgb_offset + rgb_wrap_target, rgb_offset + rgb_wrap);
    sm_config_set_clkdiv(&rgb_sm_config, rgb_clock_divider);

    if constexpr(_bits_per_pixel == 4){
        sm_config_set_set_pins(&rgb_sm_config, _start_pin+2, 4);
        sm_config_set_out_pins(&rgb_sm_config, _start_pin+2, 4);
        
        pio_gpio_init(pio_vga, _start_pin+2);
        pio_gpio_init(pio_vga, _start_pin+3);
        pio_gpio_init(pio_vga, _start_pin+4);
        pio_gpio_init(pio_vga, _start_pin+5);
        
        pio_sm_set_consecutive_pindirs(pio_vga, rgb_sm, _start_pin+2, 4, true);
    }else if constexpr(_bits_per_pixel == 1){
        sm_config_set_sideset(&rgb_sm_config, 2, true, false);
        sm_config_set_sideset_pins(&rgb_sm_config, _start_pin+2);
        sm_config_set_set_pins(&rgb_sm_config, _start_pin+2, 1);
        sm_config_set_out_pins(&rgb_sm_config, _start_pin+2, 1);
        
        pio_gpio_init(pio_vga, _start_pin+2);
        
        pio_sm_set_consecutive_pindirs(pio_vga, rgb_sm, _start_pin+2, 1, true);
    }else{
        static_assert(false, "bits_per_pixel chosen is unsupported");
    }

    pio_sm_init(pio_vga, rgb_sm, rgb_offset, &rgb_sm_config);

    pio_sm_put_blocking(pio_vga, hsync_sm, H_ACTIVEPORCH);
    pio_sm_exec(pio_vga, hsync_sm, pio_encode_pull(false, true));

    if(H_ACTIVEPORCH == 2007){
        pio_sm_put_blocking(pio_vga, vsync_sm, 34);
        pio_sm_exec(pio_vga, vsync_sm, pio_encode_pull(false, true));
        pio_sm_exec(pio_vga, vsync_sm, pio_encode_out(pio_isr, 32));
    }else if(H_ACTIVEPORCH == 655){
        pio_sm_put_blocking(pio_vga, vsync_sm, 32);
        pio_sm_exec(pio_vga, vsync_sm, pio_encode_pull(false, true));
        pio_sm_exec(pio_vga, vsync_sm, pio_encode_out(pio_isr, 32));
    }

    pio_sm_put_blocking(pio_vga, vsync_sm, V_ACTIVE);
    pio_sm_exec(pio_vga, vsync_sm, pio_encode_pull(false, true));

    pio_sm_put_blocking(pio_vga, rgb_sm, RGB_ACTIVE);
    pio_sm_exec(pio_vga, rgb_sm, pio_encode_pull(false, true));
    pio_sm_exec(pio_vga, rgb_sm, pio_encode_out(pio_isr, 32));
    
    int target_dreq, target_irq;
    if constexpr(_pio_num == 0){
        target_irq = PIO0_IRQ_0;
        switch(rgb_sm){
            case 0:
            target_dreq = DREQ_PIO0_TX0;
                break;
            case 1:
            target_dreq = DREQ_PIO0_TX1;
                break;
            case 2:
            target_dreq = DREQ_PIO0_TX2;
                break;
            case 3:
            target_dreq = DREQ_PIO0_TX3;
                break;
        }
    }else if constexpr (_pio_num == 1){
        target_irq = PIO1_IRQ_0;
        switch(rgb_sm){
            case 0:
            target_dreq = DREQ_PIO1_TX0;
                break;
            case 1:
            target_dreq = DREQ_PIO1_TX1;
                break;
            case 2:
            target_dreq = DREQ_PIO1_TX2;
                break;
            case 3:
            target_dreq = DREQ_PIO1_TX2;
                break;
        }
    }
    #ifdef PICO_RP2350
    else if(_pio_num == 2){
        target_irq = PIO2_IRQ_0;
        switch(rgb_sm){
            case 0:
            target_dreq = DREQ_PIO2_TX0;
                break;
            case 1:
            target_dreq = DREQ_PIO2_TX1;
                break;
            case 2:
            target_dreq = DREQ_PIO2_TX2;
                break;
            case 3:
            target_dreq = DREQ_PIO2_TX2;
                break;
        }
    }
    #endif 
    else{
        static_assert(false, "pio chosen doesn't exist");
    }
    pio_set_irq0_source_enabled(pio_vga, pis_interrupt0, true);

    irq_add_shared_handler(target_irq, pio_IRQ0_handeler, 0);
    irq_set_enabled(target_irq, true);
    irq_set_priority(target_irq, 0);


    dma_chan_primary = dma_claim_unused_channel(true);
    // dma_chan_reset = dma_claim_unused_channel(true);

    dma_channel_config primary_config = dma_channel_get_default_config(dma_chan_primary);

    channel_config_set_transfer_data_size(&primary_config, DMA_SIZE_8);             
    channel_config_set_read_increment(&primary_config, true);                        
    channel_config_set_write_increment(&primary_config, false);                    
    channel_config_set_dreq(&primary_config, target_dreq) ;                               
    channel_config_set_high_priority(&primary_config, true);
    // channel_config_set_chain_to(&primary_config, dma_chan_reset);
    dma_channel_configure(
        dma_chan_primary,                
        &primary_config,                      
        &pio_vga->txf[rgb_sm],         
        front_buffer,            
        TXCOUNT,                    
        false                    
    );

    // dma_channel_config reset_config = dma_channel_get_default_config(dma_chan_reset);
    // channel_config_set_transfer_data_size(&reset_config, DMA_SIZE_32);             
    // channel_config_set_read_increment(&reset_config, false);                        
    // channel_config_set_write_increment(&reset_config, false);                               
    // channel_config_set_high_priority(&reset_config, true);
    // channel_config_set_chain_to(&reset_config, dma_chan_primary);
    // dma_channel_configure(
    //     dma_chan_reset,                
    //     &reset_config,                      
    //     &dma_hw->ch[dma_chan_primary].read_addr,         
    //     &vga_address_pointer,            
    //     1,                    
    //     false                    
    // ); 

    dma_channel_set_irq0_enabled(dma_chan_primary, true);

    irq_set_exclusive_handler(DMA_IRQ_0, dma_IRQ0_handeler);
    irq_set_enabled(DMA_IRQ_0, true);
    irq_set_priority(DMA_IRQ_0, 0);

    pio_enable_sm_mask_in_sync(pio_vga, ((1u << hsync_sm) | (1u << vsync_sm) | (1u << rgb_sm)));
    dma_start_channel_mask((1u << dma_chan_primary)) ;
}

template <uint16_t _width, uint16_t _height, uint16_t _bits_per_pixel, uint16_t _num_buffers, uint8_t _pio_num>
inline void MyVga<_width, _height, _bits_per_pixel, _num_buffers, _pio_num>::drawPixel(uint16_t x, uint16_t y, ColorType color)
{
    if(x < 0 || x >= display_width) return;
    if(y < 0 || y >= display_height) return;
    
    if constexpr (_bits_per_pixel == 1){
        const uint32_t pixel_pos = (y*display_width)+x;

        const uint32_t pixel_num = pixel_pos & 0b111;
        back_buffer[pixel_pos>>3] = (back_buffer[pixel_pos>>3] & ~(1<<pixel_num)) | (color.return_color() << pixel_num) ;
    }else if constexpr (_bits_per_pixel == 4){
        const uint32_t pixel_pos = (y*display_width)+x;

        if(pixel_pos & 1){
            back_buffer[pixel_pos>>1] = (back_buffer[pixel_pos>>1] & 0b00001111) | (color.return_color() << 4) ;
        }else{
            back_buffer[pixel_pos>>1] = (back_buffer[pixel_pos>>1] & 0b11110000) | (color.return_color()) ;
        }
    }
}

template <uint16_t _width, uint16_t _height, uint16_t _bits_per_pixel, uint16_t _num_buffers, uint8_t _pio_num>
inline void MyVga<_width, _height, _bits_per_pixel, _num_buffers, _pio_num>::drawHorizontalLine(uint16_t x, uint16_t y, uint16_t length, ColorType color, uint16_t thickness)
{
    if(thickness > 1){
        for(int j = int(y)-(thickness/2)+thickness%2-1; j < y+(thickness/2); ++j)
            for(int i = x; i < (x+length); ++i) drawPixel(i, j, color);
    }else{
        for(int i = x; i < (x+length); ++i) drawPixel(i, y, color);
    }
}

template <uint16_t _width, uint16_t _height, uint16_t _bits_per_pixel, uint16_t _num_buffers, uint8_t _pio_num>
inline void MyVga<_width, _height, _bits_per_pixel, _num_buffers, _pio_num>::drawVerticalLine(uint16_t x, uint16_t y, uint16_t length, ColorType color, uint16_t thickness)
{
    if(thickness > 1){
        for(int j = int(x)-(thickness/2)+thickness%2-1; j < x+(thickness/2); ++j)
            for(int i = y; i < (y+length); ++i) drawPixel(j, i, color);
    }else{
        for(int i = y; i < (y+length); ++i) drawPixel(x, i, color);
    }
}

template <uint16_t _width, uint16_t _height, uint16_t _bits_per_pixel, uint16_t _num_buffers, uint8_t _pio_num>
inline void MyVga<_width, _height, _bits_per_pixel, _num_buffers, _pio_num>::drawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, ColorType color, uint16_t thickness)
{
    for(int i = 0; i < thickness; ++i){
        drawHorizontalLine(x, y+i,width,color);
    }
    for(int i = 0; i < thickness; ++i){
        drawHorizontalLine(x, y+height-1-i,width,color);
    }
    for(int i = 0; i < thickness; ++i){
        drawVerticalLine(x+i, y,height,color);
    }
    for(int i = 0; i < thickness; ++i){
        drawVerticalLine(x+width-1-i, y,height,color);
    }
}

template <uint16_t _width, uint16_t _height, uint16_t _bits_per_pixel, uint16_t _num_buffers, uint8_t _pio_num>
inline void MyVga<_width, _height, _bits_per_pixel, _num_buffers, _pio_num>::fillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, ColorType color)
{
    for(int i = x; i < (x+width); ++i){
        for(int j = y; j < (y+height); ++j){
            drawPixel(i, j, color);
        }
    }
}

template <uint16_t _width, uint16_t _height, uint16_t _bits_per_pixel, uint16_t _num_buffers, uint8_t _pio_num>
inline void MyVga<_width, _height, _bits_per_pixel, _num_buffers, _pio_num>::clearDisplay()
{
    memset_volatile(back_buffer, 0, frame_buffer_size*sizeof(uint8_t));
}

template <uint16_t _width, uint16_t _height, uint16_t _bits_per_pixel, uint16_t _num_buffers, uint8_t _pio_num>
inline void MyVga<_width, _height, _bits_per_pixel, _num_buffers, _pio_num>::wait_for_Vsync()
{
  vsync_ready = true;
  #ifdef FREERTOS_CONFIG_H
    while(vsync_ready) vTaskDelay(pdMS_TO_TICKS(1));
  #else
    while(vsync_ready) sleep_ms(1);
  #endif
}

template <uint16_t _width, uint16_t _height, uint16_t _bits_per_pixel, uint16_t _num_buffers, uint8_t _pio_num>
inline void MyVga<_width, _height, _bits_per_pixel, _num_buffers, _pio_num>::dma_IRQ0_handeler()
{
    dma_hw->ints0 = (1u << self->dma_chan_primary);
    self->currentScanLine++;
    if (self->currentScanLine <= self->V_ACTIVE) {
        dma_channel_set_read_addr(self->dma_chan_primary, &self->front_buffer[self->TXCOUNT * (self->currentScanLine / self->line_divisor)], true);
    }
}

template <uint16_t _width, uint16_t _height, uint16_t _bits_per_pixel, uint16_t _num_buffers, uint8_t _pio_num>
inline void MyVga<_width, _height, _bits_per_pixel, _num_buffers, _pio_num>::pio_IRQ0_handeler()
{
    pio_interrupt_clear(self->pio_vga, 0);
    self->currentScanLine = 0;
    if constexpr(_num_buffers > 1){
        if(self->vsync_ready){
            self->tmp_buffer = self->back_buffer;
            self->back_buffer = self->front_buffer;
            self->front_buffer = self->tmp_buffer;
            self->vsync_ready = false;
        }
    }else{
        self->vsync_ready = false;
    }
    dma_channel_set_read_addr(self->dma_chan_primary, &self->front_buffer[self->TXCOUNT * (self->currentScanLine / self->line_divisor)], true);
}

template <uint16_t _width, uint16_t _height, uint16_t _bits_per_pixel, uint16_t _num_buffers, uint8_t _pio_num>
inline void MyVga<_width, _height, _bits_per_pixel, _num_buffers, _pio_num>::memset_volatile(volatile unsigned char *s, unsigned char c, unsigned int n)
{
    volatile unsigned char *p = s;
    while (n-- > 0) {
        *p++ = c;
    }
}

template <uint16_t _width, uint16_t _height, uint16_t _bits_per_pixel, uint16_t _num_buffers, uint8_t _pio_num>
inline void MyVga<_width, _height, _bits_per_pixel, _num_buffers, _pio_num>::memset_volatile(volatile unsigned char *s, unsigned char c, unsigned int n, unsigned int offset)
{
    volatile unsigned char *p = s;
    while (n-- > 0) {
        *p = c;
        p += offset;
    }
}

template <uint16_t _width, uint16_t _height, uint16_t _bits_per_pixel, uint16_t _num_buffers, uint8_t _pio_num>
inline constexpr uint8_t MyVga<_width, _height, _bits_per_pixel, _num_buffers, _pio_num>::find_best_resolution_match()
{
    uint64_t min_distance = vector_dot(supported_base_resolutions[MAX_DISPLAY_RES].width/4 - _width, supported_base_resolutions[MAX_DISPLAY_RES].height/4 - _height), tmp=0;
    uint8_t res = MAX_DISPLAY_RES;
    for(int i = MAX_DISPLAY_RES; i >= 0; --i){
        tmp = vector_dot(supported_base_resolutions[i].width/8 - _width, supported_base_resolutions[i].height/8 - _height);
        if(tmp < min_distance){
            min_distance = tmp;
            res = i;
        }
        tmp = vector_dot(supported_base_resolutions[i].width/6 - _width, supported_base_resolutions[i].height/6 - _height);
        if(tmp < min_distance){
            min_distance = tmp;
            res = i;
        }
        tmp = vector_dot(supported_base_resolutions[i].width/4 - _width, supported_base_resolutions[i].height/4 - _height);
        if(tmp < min_distance){
            min_distance = tmp;
            res = i;
        }
        tmp = vector_dot(supported_base_resolutions[i].width/3 - _width, supported_base_resolutions[i].height/3 - _height);
        if(tmp < min_distance){
            min_distance = tmp;
            res = i;
        }
        tmp = vector_dot(supported_base_resolutions[i].width/2 - _width, supported_base_resolutions[i].height/2 - _height);
        if(tmp < min_distance){
            min_distance = tmp;
            res = i;
        }
        tmp = vector_dot(supported_base_resolutions[i].width - _width, supported_base_resolutions[i].height - _height);
        if(tmp < min_distance){
            min_distance = tmp;
            res = i;
        }
    }

    return res;
}

template <uint16_t _width, uint16_t _height, uint16_t _bits_per_pixel, uint16_t _num_buffers, uint8_t _pio_num>
inline constexpr uint8_t MyVga<_width, _height, _bits_per_pixel, _num_buffers, _pio_num>::find_best_resolution_divisor(uint8_t closest_match)
{
    uint64_t min_distance = vector_dot(supported_base_resolutions[closest_match].width/8 - _width, supported_base_resolutions[closest_match].height/8 - _height), tmp=0;
    uint8_t res = 8;
    tmp = vector_dot(supported_base_resolutions[closest_match].width/6 - _width, supported_base_resolutions[closest_match].height/6 - _height);
    if(tmp < min_distance){
        min_distance = tmp;
        res = 6;
    }
    tmp = vector_dot(supported_base_resolutions[closest_match].width/4 - _width, supported_base_resolutions[closest_match].height/4 - _height);
    if(tmp < min_distance){
        min_distance = tmp;
        res = 4;
    }
    tmp = vector_dot(supported_base_resolutions[closest_match].width/3 - _width, supported_base_resolutions[closest_match].height/3 - _height);
    if(tmp < min_distance){
        min_distance = tmp;
        res = 3;
    }
    tmp = vector_dot(supported_base_resolutions[closest_match].width/2 - _width, supported_base_resolutions[closest_match].height/2 - _height);
    if(tmp < min_distance){
        min_distance = tmp;
        res = 2;
    }
    tmp = vector_dot(supported_base_resolutions[closest_match].width - _width, supported_base_resolutions[closest_match].height - _height);
    if(tmp < min_distance){
        min_distance = tmp;
        res = 1;
    }
    return res;
}

template <uint16_t _width, uint16_t _height, uint16_t _bits_per_pixel, uint16_t _num_buffers, uint8_t _pio_num>
inline constexpr bool MyVga<_width, _height, _bits_per_pixel, _num_buffers, _pio_num>::check_if_exact_resolution(uint8_t closest_match, uint8_t closest_divisor)
{
    return vector_dot(supported_base_resolutions[closest_match].width/closest_divisor - _width, supported_base_resolutions[closest_match].height/closest_divisor - _height) == 0;
}

template <uint16_t _width, uint16_t _height, uint16_t _bits_per_pixel, uint16_t _num_buffers, uint8_t _pio_num>
inline constexpr uint8_t MyVga<_width, _height, _bits_per_pixel, _num_buffers, _pio_num>::find_clock_divisor(uint32_t target_clock)
{
    uint32_t min_clock = target_clock-uint32_t(double(target_clock)*0.005);
    uint32_t max_clock = target_clock+uint32_t(double(target_clock)*0.005);
    
    for(uint32_t i = 1; SYS_CLK_HZ/i >= min_clock; ++i){
        if(SYS_CLK_HZ/i >= min_clock && SYS_CLK_HZ/i <= max_clock) return i;
    }

    return 0;
}

template <uint16_t _width, uint16_t _height, uint16_t _bits_per_pixel, uint16_t _num_buffers, uint8_t _pio_num>
inline constexpr uint8_t MyVga<_width, _height, _bits_per_pixel, _num_buffers, _pio_num>::find_rgb_clock_divisor(uint8_t clock_divisor, uint8_t divisor)
{
    if(divisor == 1){
        if(clock_divisor % 3 == 0){
            return clock_divisor/3;
        }else if(clock_divisor % 4 == 0){
            return clock_divisor/4;
        }else{
            return 0;
        }
    }else if(divisor == 2){
        if(clock_divisor % 2 == 0){
            return clock_divisor/2;
        }else{
            return 0;
        }
    }else if(divisor == 3){
        return clock_divisor;
    }else if(divisor == 4){
        return clock_divisor;
    }else if(divisor == 6){
        return clock_divisor*2;
    }else if(divisor == 8){
        return clock_divisor*2;
    }
    return 0;
}

template <uint16_t _width, uint16_t _height, uint16_t _bits_per_pixel, uint16_t _num_buffers, uint8_t _pio_num>
inline constexpr uint8_t MyVga<_width, _height, _bits_per_pixel, _num_buffers, _pio_num>::find_rgb_clocks_per_pixel(uint8_t clock_divisor, uint8_t divisor)
{
    if(divisor == 1){
        if(clock_divisor % 3 == 0){
            return 3;
        }else if(clock_divisor % 4 == 0){
            return 4;
        }
    }else if(divisor == 2){
        if(clock_divisor % 2 == 0){
            return 4;
        }else{
            return 0;
        }
    }else if(divisor == 3){
        return 3;
    }else if(divisor == 4){
        return 4;
    }else if(divisor == 6){
        return 3;
    }else if(divisor == 8){
        return 4;
    }
    return 0;
}

#endif // MY_VGA_IMPL_H