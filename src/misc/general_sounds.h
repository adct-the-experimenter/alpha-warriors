#ifndef GENERAL_SOUNDS_H
#define GENERAL_SOUNDS_H

#include <string>
#include "raylib.h"

//General sounds
#define NUMBER_OF_GENERAL_SOUNDS 2

extern Sound general_sounds[NUMBER_OF_GENERAL_SOUNDS];

enum class GeneralSoundID : uint8_t {TEST=0,TELEPORT_ONE};

extern std::string general_sound_audio_files_paths[NUMBER_OF_GENERAL_SOUNDS];

extern void PlayGeneralSound(GeneralSoundID sound_id);

#endif
