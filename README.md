# PandemiCpp
C++ implementation of pandemic board game for testing MCTS agents

## Requirements
I believe that you only need a C++ compiler that's capable of compiling the C++17 standard. This standard is required because I've used some `inline` statements to define global variables. 

## Test the game
Right now `Playtest.cpp` holds the logic to call the random agent in a playthrough of the game. It draws actions uniformly from the set of ALL legal actions, meaning that moves are typically biased towards the kinds that have more legal options (e.g. Airlift). You can compile a binary and run it with `make playtest`, which makes a `playtest.out` binary you can call over and over.

Right now there are several compile warnings regarding the possibility of not returning anything in some `ActionConstructor` functions and others. That's fine to me right now, I would very much want the game to fail if any of those points are reached.


## Pandemic Board Game

The rules can be found [here](https://images-cdn.zmangames.com/us-east-1/filer_public/25/12/251252dd-1338-4f78-b90d-afe073c72363/zm7101_pandemic_rules.pdf), which is nice because you can also see a game map.
