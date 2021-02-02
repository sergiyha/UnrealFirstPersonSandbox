#pragma once
#include "ASpecialMovesState.h"

class WallRunState : public ASpecialMovesState
{
public:
	virtual void StartState(int fromState) override;

	virtual bool CanSwitchTheState() override;

	virtual void EndState(int fromState) override;
};
