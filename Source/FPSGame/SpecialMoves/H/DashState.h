#pragma once
#include "ASpecialMovesState.h"

class DashState : ASpecialMovesState
{
public:
	virtual bool CanSwitchTheState() override;
	virtual void EndState(int fromState) override;
	virtual void StartState(int fromState) override;
};
