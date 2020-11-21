#ifndef MUSIC_STATE_FXM_H
#define MUSIC_STATE_FXM_H

#include "defines.h"
#include "music_data_FXM.h"
#include "ay_channel.h"

struct FXM_Voice {
	char indexCH;
    uint8_t Note;
	uint8_t Note_Skip_Counter;
	uint16_t start_addr;
	uint16_t Address_In_Pattern;		// указатель на текущую обрабатываемую позицию в патерне при инициализации находится на start_addr
	uint16_t Point_In_Sample;     /// pointer inside envelope
	uint16_t SamplePointer;		// envelope pointer
	uint8_t Sample_Tik_Counter;	// timer envelope
	uint16_t stack[20];
	uint8_t  sp;		// index of stack pointer
	uint16_t Point_In_Ornament;
	uint16_t OrnamentPointer;
	int8_t Transposit;
	uint16_t Ton;
	bool b0e;/// b0e - legato (true false)  (false false)
	bool b1e;
	bool b2e;
	bool b3e;  /// true - tone vibrato,  false - freq vibrato
	bool end;
	uint8_t FXM_Mixer;
	uint8_t Volume;
	uint8_t Amplitude;
};

class MusicState {
private:
	MusicData_FXM music_data;

    uint16_t voice1_start_addr;
    uint16_t voice2_start_addr;
    uint16_t voice3_start_addr;
    uint8_t amad_andsix;
    void init_channal(FXM_Voice &ch);
	void RealGetRegisters(FXM_Voice &channel);
	void GetRegisters(FXM_Voice &channel);
	void PatternInterpeter(FXM_Voice &vo);

public:
	uint16_t* GetTable(void);
    uint16_t Noise_Base;
	FXM_Voice voice1, voice2, voice3;
    void set_data(PROGMEM_data *md);
    void init_music(void);
    bool play20ms(void);
    void next(Channel &channel_a, Channel &channel_b, Channel &channel_c);
};

#endif

