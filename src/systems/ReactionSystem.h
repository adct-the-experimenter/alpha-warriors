#ifndef REACTION_SYSTEM_H
#define REACTION_SYSTEM_H

#include "core/system.h"

class ReactionSystem : public System
{
public:
	void HandleReactionToCollisions(float& dt);

private:

};

#endif
