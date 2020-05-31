# PandemiCpp
C++ implementation of pandemic board game for testing MCTS agents

## Requirements
I believe that you only need a C++ compiler that's capable of compiling the C++17 standard. This standard is required because I've used some `inline` statements to define global variables. 

## Test the game
Right now `Playtest.cpp` holds the logic to call the random agent in a playthrough of the game. It draws actions uniformly from the set of ALL legal actions, meaning that moves are typically biased towards the kinds that have more legal options (e.g. Airlift). You can compile a binary and run it with `make playtest`, which makes a `playtest.out` binary you can call over and over.

Right now there are several compile warnings regarding the possibility of not returning anything in some `ActionConstructor` functions and others. That's fine to me right now, I would very much want the game to fail if any of those points are reached.

## Modifications to the game rules
### Roles
- "Dispatcher" and "Contingency Planner" have been removed from the game. The former induces an large branching factor, while the latter is just typically a pretty low-value role (don't @ me). I think it would not be that difficult to implement a dispatcher by defining the role in `Players.h` and `Players.cpp`, as well as creating a new `Action` and corresponding `ActionConstructor` specifically for dispatcher movements, which are all "regular" movements of a pawn by a player. If this Constructor were then included in the `GameLogic`, it should work? Writing in the Action legality guards and list_actions would be hard.
### Cards
- "Forecast" and "Resilient Population" cards have been removed from the game. The former has factorial (in discard size) branching factor when considered as a possible player action, and so is intractable. If considered only as a "random" action, it's a potentially damaging or helpful just like existing stochasticity that's already in the game, and so really doesn't add anything to the problem at hand. Potential implementations could include some hard rules about "allowable" orders, but I suspect even then it'd be a large branching factor. "Resilient population" might be interesting to implement in the future, since it allows players to directly alter the distribution of cards after the next Epidemic "Intensify" step. At least right now, though, I'm not trying to find out the degree to which agents can understand and learn to manipulate the distribution represented by the current infect discard pile.
### Movement and Actions
- Right now the `GameLogic` precludes the use of any event cards outside of the players 4 actions. This means that if the agent does't use an available card by the "draw player card" phase of the game, they won't be able to use it until the start of the next player turn. I suspect this is a mild handicap to performance compared to canon game rules, since it prevents agents from capitalizing on information as it's revealed during the stochastic phases of the game (for example Airlifting a medic or quarantine specialist to the site of a city who will now with certainty be drawn from the infection deck). This helps reduce the number of decision nodes and branching factor - for example Airlift has ~n_players*n_cities options. What if this were considered between every single card draw?
- The `Actions` all have `legal()` guards, whose truth value indicates whether or not there's at least one *legal and not _entirely_ stupid* move to make. Their `random_action` and `list_actions` incorporate the same rules for restricting which actions can be returned. (e.g. it's stupid to move from a city and back, or discard the card of the city you're in to move to the city you're in - even though they're both legal!).
    - All basic movement options, as well as airlift, preclude the player from returning to their _last position_ unless they've done something since then (Treated disease, built a station, cured a disease, or traded cards). This _doesn't_ prevent loops, but I hope helps to broaden random walks slightly decrease branching in a helpful way.
    - `Airlift`, `DirectFlight`, and `CharterFlight` will additionally not count the movement of a player to a neighboring city as valid. For `DirectFlight` and `CharterFlight` this is a strict optimization, since discarding a card to move one city a way can never be better than just moving to it. In the case of `Airlift` it's more of an almost-always optimization designed to reduce the branching factor.
- The `GameLogic` will ALWAYS return a `ForcedDiscard` list of actions if ANY player has >7 cards. Among available actions are the use of any event cards in their hand. 
- the `Cure` action will always discard the first 4 (if scientist) or 5 (if not) cards whose color matches the color to be cured, rather than giving the player a choice over exactly which they want to discard in the case that they have more than necessary. This is nothing but a search optimization.

## Pandemic Board Game

The rules can be found [here](https://images-cdn.zmangames.com/us-east-1/filer_public/25/12/251252dd-1338-4f78-b90d-afe073c72363/zm7101_pandemic_rules.pdf), which is nice because you can also see a game map.
