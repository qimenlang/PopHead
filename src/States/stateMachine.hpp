#ifndef POPHEAD_STATES_STATEMACHINE_H_
#define POPHEAD_STATES_STATEMACHINE_H_

#include <memory>
#include <vector>
#include <deque>
#include <SFML/System.hpp>

#include "States/state.hpp"
#include "States/stateIdentifiers.hpp"

namespace ph {

class GameData;

using StatePtr = std::unique_ptr<State>;

class StateMachine
{
public:
    StateMachine();

    void pushState(StateID);
    void replaceState(StateID);
    void popState();
    void clearStates();
    
	void changingStatesProcess();
private:
	void pushAction();
	void replaceAction();
	void popAction();
	void clearAction();

public:
    void input();
    void update(sf::Time delta);

    auto getStatesAmount() const -> unsigned int {return mActiveStates.size();}
    bool getHideInStateNr(unsigned int nrOfState) const; /// 0 is top
    bool getPauseInStateNr(unsigned int nrOfState) const;

    void setHideInStateNr(unsigned int nrOfState, bool hide);
    void setPauseInStateNr(unsigned int nrOfState, bool pause);

    void setGameData( ph::GameData* const gameData ){mGameData = gameData;}

private:
    auto getStatePtr(StateID id) const -> std::unique_ptr<State>;

    std::vector<StatePtr> mActiveStates;
    std::deque<StatePtr> mPendingStates;

    GameData* mGameData;

    bool mIsPushing;
    bool mIsReplacing;
    bool mIsPopping;
    bool mIsClearing;
};

}

#endif // !POPHEAD_STATES_STATEMACHINE_H_
