# ArduinoFuxoftPlayer


Aruino play Fuxoft tracks


## the original idea by https://github.com/admitrievsky/ay-3-8912-d4

  Play Fuxoft Sound tracks on stand alone Arduino
  Need connect only some speaker or amplifier line in is better on D3 pin (via capacitor?)
  and you can hear...
 
 Yon can connect key button to D5 pin and GND that will
 short press - next
 long press - 7 sec. preview every track of playlist

 Also terminal out present


 Playlist, and data tracks are in music_data_FXM.cpp file


 Thanks Frantisek Fuka -  https://github.com/fuxoftfor http://FUXOFT.CZ all sound tracks and fxm format description

 Thanks to Bulba Sergey Vladimirovich for Source code of AY Emulator

 Thanks to Anton Dmitrievsky for ay_channel.cpp code and Idea

 the changes I made to ay_channel.cpp are wet and need correction
 since there are inaccuracies in sound reproduction, sorry
