#ifndef AGENTS_H
#define AGENTS_H

#include "../game_files/GameLogic.h"
#include "../game_files/Actions.h"

namespace Agents
{
    class BaseAgent{
    public:
        BaseAgent(GameLogic::Game& _active_game);
        virtual ~BaseAgent(){};
        // Keep a reference to the active game to hand out inside of returned actions
        GameLogic::Game &active_game;

        // Name that can be used for logging
        std::string name;
        
        // Whether or not the agent has any measurements to report to the experimental infrastructure
        bool measurable=false;

        // BaseAgent methods below should NEVER be called, they're always pure virtual.

        // An agent just has to be able to generate an action
        // May or may not care about all_actions, the game state, ...
        virtual Actions::Action* generate_action(bool verbose = false)=0;

        // Choose an action and move forward a step
        virtual void take_step(bool verbose=false)=0;

        // For if the agent stores any game-level information for decision making or measurement
        virtual void reset()=0;

        // For measurement: get measurement headers
        virtual std::vector<std::string> get_keys()=0;

        // For measurement: get measurement values, comma separated (beginning with comma)
        virtual std::vector<double> get_values()=0;
    };     
}

#endif