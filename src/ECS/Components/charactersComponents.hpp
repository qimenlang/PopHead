#pragma once

namespace ph::component {
	
	struct Health
	{
		int healthPoints;
		int maxHealthPoints;
	};
	
	struct Player
	{
	};

	struct MeleeAttacker
	{
		float minSecondsInterval;
		bool isTryingToAttack;
	};

	struct GunAttacker
	{
		float minSecondsInterval;
		unsigned bullets;
		bool isTryingToAttack;
	};

	struct TaggedToDestroy
	{

	};
}