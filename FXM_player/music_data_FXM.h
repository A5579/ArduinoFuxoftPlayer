#ifndef MUSIC_DATA_FXM_H
#define MUSIC_DATA_FXM_H

struct MusicData_FXM {
    const uint8_t* data;
    uint8_t* offset;

    uint8_t read_byte(uint16_t address) const {
        return pgm_read_byte_near(data + address - offset);
    }

    uint16_t read_word(uint16_t address) const {
        return pgm_read_word_near(data + address - offset);
    }
};

typedef const PROGMEM uint8_t PROGMEM_data;




PROGMEM_data* get_magnetic_fields4(void);

PROGMEM_data* get_sound_track(uint8_t index);


uint8_t get_playlist_size(void);

#endif
