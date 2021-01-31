#pragma once

class ASpecialMovesState
{
public:
	//virtual ~ASpecialMovesState() = default;
	//virtual void EndState(ASpecialMovesState* fromState) ;
	virtual void StartState(ASpecialMovesState* fromState) =0;
	//virtual bool CanSwitchTheState() ;
};
