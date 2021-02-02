#pragma once
#include "StateMachine/H/AState.h"

enum SpecialMoves
{
	WallRun,
	DashState
};

class ASpecialMovesState : public AState
{
public:
	virtual void EndState(int fromState) override =0;
	virtual void StartState(int fromState) override =0;
	virtual bool CanSwitchTheState() override =0;
protected:
	SpecialMoves GetState(int index);
};
