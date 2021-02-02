#include "SpecialMoves/H/WallRunState.h"
#include "SpecialMoves/H/DashState.h"

bool WallRunState::CanSwitchTheState()
{
	return false;
}

void WallRunState::EndState(int fromState)
{
	
}

void WallRunState::StartState(int fromState)
{
	auto state = GetState(fromState);
	
}
