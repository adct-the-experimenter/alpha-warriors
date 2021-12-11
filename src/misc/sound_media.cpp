#include "sound_media.h"

#include "globalvariables.h"

#include "char_media.h"

bool LoadGeneralAudio()
{
	for(size_t i = 0; i < NUMBER_OF_GENERAL_SOUNDS; i++)
	{
		std::string fp = DATADIR_STR + general_sound_audio_files_paths[i];
		general_sounds[i] = LoadSound(fp.c_str());
	}
	
	
	return true;
}

void PlayGeneralSound(GeneralSoundID sound_id)
{
	PlaySound(general_sounds[ static_cast<int>(sound_id) ]);
}

void PlayCharacterSound(std::int16_t& char_index,CharSoundID& sound_id)
{
	PlaySound(character_sounds[char_index].sounds[ static_cast<int>(sound_id) ]);
}

void UnloadGeneralAudio()
{
	for(size_t i = 0; i < NUMBER_OF_GENERAL_SOUNDS; i++)
	{
		UnloadSound(general_sounds[i]);
	}
	
}
