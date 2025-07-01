#ifndef FONT_H
#define FONT_H

class Font{
    public:
        virtual const uint8_t* __not_in_flash_func(get_char)(uint16_t character);
};

#endif // FONT_H