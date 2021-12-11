#include "misc/general_sounds.h"

Sound general_sounds[NUMBER_OF_GENERAL_SOUNDS];

//DATADIR_STR not working here for some strange reason.

std::string general_sound_audio_files_paths[NUMBER_OF_GENERAL_SOUNDS] = 
{
	("/general_sounds/test.wav"),
	("/general_sounds/teleport1.wav")
};
