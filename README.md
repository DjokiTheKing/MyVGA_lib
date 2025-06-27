# MyVGA_lib
rp2040/rp2350 vga library

### ---> In development

This is my simple to use vga library for pico sdk, there is still a lot of work to be done, but even at this stage it can be used.
 * Currently only 1 bit and 4 bit modes are supported, with plans to add 8 and 16 bit modes. 
 * I only tested using rp2040 but will test soon with rp2350
 * The clock speed of the pi pico has to be within 0.5% of a multiple of the pixel clock speed from the specification ([you can check on here](https://projectf.io/posts/video-timings-vga-720p-1080p/))

Will come back to improve this once i finish the base of my pwmaudio library, usb keyboard library, http/https get libraries... i have a lot of work to do... 
