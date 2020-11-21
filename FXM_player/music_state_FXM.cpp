/*
 *
 *  Thanks Frantisek Fuka - FUXOFT.CZ for all sound tracks and fxm format description
 *
 *  Thanks to Bulba Sergey Vladimirovich for Source code of AY Emulator
 *
 */



#include "music_state_FXM.h"
#include "music_data_FXM.h"

uint16_t FXM_Table[] = {
		0xfbf,0xedc,0xe07,
		0xd3d,0xc7f,0xbcc,0xb22,0xa82,0x9eb,0x95d,0x8d6,0x857,0x7df,0x76e,0x703,
		0x69f,0x640,0x5e6,0x591,0x541,0x4f6,0x4ae,0x46b,0x42c,0x3f0,0x3b7,0x382,
		0x34f,0x320,0x2f3,0x2c8,0x2a1,0x27b,0x257,0x236,0x216,0x1f8,0x1dc,0x1c1,
		0x1a8, // (40 - 1)  must be C - 261,625 Hz
		      0x190,0x179,0x164,0x150,0x13d,0x12c,0x11b,0x10b,0x0fc,0x0ee,0x0e0,
		0x0d4,0x0c8,0x0bd,0x0b2,0x0a8,0x09f,0x096,0x08d,0x085,0x07e,0x077,0x070,
		0x06a,0x064,0x05e,0x059,0x054,0x04f,0x04b,0x047,0x043,0x03f,0x03b,0x038,
		0x035,0x032,0x02f,0x02d,0x02a,0x028,0x025,0x023,0x021 };

void MusicState::set_data(PROGMEM_data *md)
{
	music_data.data = md;
	music_data.offset =(uint8_t*)pgm_read_word_near(music_data.data+4)-6;

	voice1.start_addr = music_data.read_word((int)(music_data.offset)+6);	//pgm_read_word_near(md.data+6); read original start addr of vo 1
	voice2.start_addr = music_data.read_word((int)(music_data.offset)+8);	//pgm_read_word_near(md.data+8);
	voice3.start_addr = music_data.read_word((int)(music_data.offset)+10);	//pgm_read_word_near(md.data+10);
}

void MusicState::init_channal(FXM_Voice &ch)
{
	ch.Address_In_Pattern = ch.start_addr;	// go to start of chanel melody
	ch.sp = 0 ;								// clear stack
	ch.Note_Skip_Counter = 1;
	ch.FXM_Mixer = 8;
	ch.Transposit = 0;
	ch.b0e = false;
	ch.b1e = false;
	ch.b2e = false;
	ch.b3e = false;
	ch.end = false;
}


void MusicState::init_music()
{
	init_channal(voice1);
	init_channal(voice2);
	init_channal(voice3);
}


uint16_t* MusicState::GetTable(void)
{
	return  FXM_Table;
}


void MusicState::GetRegisters(FXM_Voice &ch)
{
	uint8_t b;
	uint8_t env;
	uint8_t vib;

	ch.Sample_Tik_Counter--;
	if (ch.Sample_Tik_Counter == 0)
	{
		do
		{
			env = music_data.read_byte(ch.Point_In_Sample);
			if (env < 16)
			{
				ch.Volume = env&0xf;				// envelope volume
				ch.Point_In_Sample++;
				ch.Sample_Tik_Counter = music_data.read_byte(ch.Point_In_Sample);	// envelope time
				ch.Point_In_Sample++;
				break;
			}
			else if (env == 0x80)
			{
				ch.Point_In_Sample = music_data.read_word(ch.Point_In_Sample + 1); // jump in envelope
				//	ch.volume
			}
			else if (env >= 50 && env <= 65)
			{
				ch.Volume = music_data.read_byte(ch.Point_In_Sample) - 50;	// one byte envelope descriptor
				ch.Point_In_Sample++;
				ch.Sample_Tik_Counter = 1;
				break;
			}
			else
				break;
		} while( true );
	};

	if ((ch.Ton != 0) && (!ch.b2e))   /// vibrato  (ornament)
	{
		do
		{
			vib = music_data.read_byte(ch.Point_In_Ornament);
			if (vib == 0x80)
			{
				ch.Point_In_Ornament = music_data.read_word(ch.Point_In_Ornament + 1);
			}
			else if (vib == 0x82)	//  tone vibrato mode
			{
				ch.Point_In_Ornament++;
				ch.b3e = true;
			}
			else if (vib == 0x83)	// freq vibrato mode
			{
				ch.Point_In_Ornament++;
				ch.b3e = false;
			}
			else if (vib ==0x84)	//  xor vibrato mode
			{
				ch.Point_In_Ornament++;
				ch.FXM_Mixer ^= 9;
			}
			else
			{
				if (ch.b3e)
				{
					ch.Note += (int8_t)music_data.read_byte(ch.Point_In_Ornament);
					if (ch.Note > 84)
						b = 83;
					else
						b = ch.Note;
					ch.Ton = FXM_Table[b-1];
				}
				else
				{
					ch.Ton+=(int8_t)music_data.read_byte(ch.Point_In_Ornament);
				}
				ch.Point_In_Ornament++;
				break;
			}
		} while (true);
	}
	RealGetRegisters(ch);
}

void MusicState::RealGetRegisters(FXM_Voice &channel)
{
//	  RegisterAY.Noise = Noise_Base & 0x1F;
	channel.b2e = false;
	if (channel.Ton != 0)
		channel.Amplitude = channel.Volume & 0x0f;
	else
		channel.Amplitude = 0;
}


bool MusicState::play20ms()
{
	PatternInterpeter(voice1);
	PatternInterpeter(voice2);
	PatternInterpeter(voice3);

	if (voice1.end && voice2.end && voice3.end)
		return false;

	return true;

}

void MusicState::PatternInterpeter(FXM_Voice &vo)
{
	int lc;

	vo.Note_Skip_Counter--;

	if (vo.Note_Skip_Counter != 0)
    	GetRegisters(vo);
    else
    {
		do
		{
			uint8_t command = music_data.read_byte(vo.Address_In_Pattern);
			if (command < 0x80)
			{
				if (command != 0 )
				{
					vo.Note = command  + vo.Transposit;
					if (vo.Note >  84)
						vo.Ton = FXM_Table[84-1];
					else
						vo.Ton = FXM_Table[vo.Note - 1];

					vo.b3e = false;
				}
				else
				{
					vo.Ton = 0;
				}
				vo.Address_In_Pattern++;
				vo.Note_Skip_Counter = music_data.read_byte(vo.Address_In_Pattern);
				vo.Address_In_Pattern++;
				vo.Point_In_Ornament = vo.OrnamentPointer;

				if (!vo.b1e)
				{
					vo.b1e = vo.b0e;
					vo.Point_In_Sample = vo.SamplePointer;						/// идем в начало огибающей env
					vo.Volume = music_data.read_byte(vo.Point_In_Sample);       // громкость ставим из начала огибающей
					vo.Point_In_Sample++;
					vo.Sample_Tik_Counter = music_data.read_byte(vo.Point_In_Sample);  // обновляем таймер огибающей
					vo.Point_In_Sample++;
					RealGetRegisters(vo);                                         // пишем громкость в amplitude
				}
				else
					GetRegisters(vo);

				break;
			//	return;		//-!!---------------------------
			}
			else if (command == 0x80)
			{
				vo.Address_In_Pattern = music_data.read_word(vo.Address_In_Pattern+1);
				vo.end = true;
			}
			else if (command == 0x81) // CALL ROUTINE
			{
				vo.stack[vo.sp] = vo.Address_In_Pattern + 3;	// PUSH return point of call to stack
				vo.sp++;
				vo.Address_In_Pattern =  music_data.read_word(vo.Address_In_Pattern+1);
			}
			else if (command == 0x82)	// LOOP
			{
				vo.Address_In_Pattern++;
				vo.stack[vo.sp] = (uint16_t)music_data.read_byte(vo.Address_In_Pattern); // push loop count
				vo.sp++;
				vo.Address_In_Pattern++;
				vo.stack[vo.sp] = vo.Address_In_Pattern; // push loop entry point
				vo.sp++;
			}
			else if (command == 0x83)// NEXT   (End of last LOOP)
			{
				vo.stack[vo.sp-2] -= 1; // dec loop count and get it
				lc = vo.stack[vo.sp-2] ; // dec loop count and get it
				if (lc == 0)
				{
					vo.Address_In_Pattern++;
					vo.sp--;
					vo.sp--;
				}else{
					vo.Address_In_Pattern = vo.stack[vo.sp-1];
				}
			}
			else if (command == 0x84) // Set Noise
			{
				vo.Address_In_Pattern++;
				Noise_Base = music_data.read_byte(vo.Address_In_Pattern);
				vo.Address_In_Pattern++;
			}
			else if (command == 0x85) // Set TYPE
			{
				vo.Address_In_Pattern++;
				vo.FXM_Mixer = music_data.read_byte(vo.Address_In_Pattern);
				vo.Address_In_Pattern++;
			}
			else if (command == 0x86) // Set VIBration ptr
			{
				vo.OrnamentPointer = music_data.read_word(vo.Address_In_Pattern+1);
				vo.Address_In_Pattern+=3;
			}
			else if (command == 0x87) // Set ENVelope ptr
			{
				vo.Address_In_Pattern++;
				vo.SamplePointer = music_data.read_word(vo.Address_In_Pattern);
				vo.Address_In_Pattern+=2;
			}
			else if (command == 0x88) // TRanspose
			{
				vo.Address_In_Pattern++;
				vo.Transposit = music_data.read_byte(vo.Address_In_Pattern);
				vo.Address_In_Pattern++;
			}
			else if (command == 0x89)// RETurn from routine
			{
				vo.sp--;
				vo.Address_In_Pattern = vo.stack[vo.sp];
			}
			else if (command == 0x8A) // LEG+
			{
				vo.Address_In_Pattern++;
				vo.b0e = true;
				vo.b1e = false;
			}
			else if (command == 0x8B) // LEG -
			{
				vo.Address_In_Pattern++;
				vo.b0e = false;
				vo.b1e = false;
			}
			else if (command == 0x8C) // External_CALL
			{
				vo.Address_In_Pattern+=3;
			}
			else if (command == 0x8D)	// noise +
			{
				vo.Address_In_Pattern++;
				Noise_Base += (int8_t)music_data.read_byte(vo.Address_In_Pattern);
				Noise_Base &= 0x1f;
				vo.Address_In_Pattern++;
			}
			else if (command == 0x8E)
			{
				vo.Address_In_Pattern++;
				vo.Transposit += (int8_t)music_data.read_byte(vo.Address_In_Pattern);
				vo.Address_In_Pattern++;
			}
			else if (command == 0x8F)
			{
				vo.stack[vo.sp] = vo.Transposit;		// push Transposit to stack
				vo.sp++;
				vo.Address_In_Pattern++;
			}
			else if (command == 0x81) // pop  Transposit from stack
			{
				vo.sp--;
				vo.Transposit = vo.stack[vo.sp];
				vo.Address_In_Pattern++;
			}
			else
			{
				vo.Address_In_Pattern++;
			}
		} while (true);
	}
}
