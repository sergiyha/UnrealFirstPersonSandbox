#pragma once

class AState
{
public:
	virtual ~AState() = default;
	virtual void EndState(int fromState) =0;
	virtual void StartState(int fromState) =0;
	virtual bool CanSwitchTheState() =0;
};
