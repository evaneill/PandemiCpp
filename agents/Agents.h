#ifndef AGENTS_H
#define AGENTS_H

#include "../game_files/GameLogic.h"
#include "../game_files/Actions.h"

namespace Agents
{
    class BaseAgent{
    public:
        BaseAgent(GameLogic::Game& _active_game);

        // Keep a reference to the active game to hand out inside of returned actions
        GameLogic::Game &active_game;

        // Name that can be used for logging
        std::string name;
        
        // BaseAgent methods below should NEVER be called, they're always pure virtual.

        // An agent just has to be able to generate an action
        // May or may not care about all_actions, the game state, ...
        virtual Actions::Action* generate_action(bool verbose = false)=0;

        // Choose an action and move forward a step
        virtual void take_step(bool verbose=false)=0;
    };     
}

#endif