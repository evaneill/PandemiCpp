# PandemiCpp 
C++ implementation of pandemic board game for testing MCTS agents. 

Disclaimer: This was created as a means to explore and test MCTS (and tree based stochastic planning) rather than develop or try state-of-the-art methods. So it's lacking some functionality that would help towards these ends.

## Status
As of Jul 6, 2020 this has been through a decent amount of testing. I'm inclined to call this `v0`. 

Also this is my first time with C++, so you'll have to overlook all of the breaches of best-practice. Further, I'm certain that there are probably inefficiencies or impracticalities that would be obvious to a real practitioner, but I don't know about. 

## Requirements
I believe that you only need a C++ compiler that's capable of compiling the C++17 standard. This standard is required because I've used some `inline` statements to define global variables. The commands in `makefile` require `g++`, specifically, and have been developed on an OSX system. It takes my compiler ~30-60 ish sec to build a full experiment.

## Hello World Playtest
Right now `Playtest.cpp` holds the logic to call the random agent in a playthrough of the game. It draws actions uniformly over _types_ of legal actions, and then uniformly over legal actions of the chosen type. You can compile a binary and run it with `make playtest`, which makes a `./playtest.out` binary you can call over and over.

## Run an experiment
I've made `Experiment`s to encapsulate all of the specification of an agent, initial board state, etc. required to run and log an experiment. By default experiments will be written to `results/` in the form of a `.header` and `.csv` file, which describe meta information about the experiment and experimental results describing game-level information, respectively. 

An experiment can be built with `make experiment EXPERIMENT="experiments/ByGroupRandomAgentGameExperiment"`, for example. It can be ran by running the binary created in `experiments/`, for example with `experiments/ByGroupRandomAgentGameExperiment.out`. That particular experiment will run many many games using the same random agent used by `Playtest.cpp`.

## Modifications to the game rules
### Roles
- "Dispatcher" and "Contingency Planner" have been removed from the game. The former induces an large branching factor, while the latter is just typically a pretty low-value role (don't @ me). I think it would not be that difficult to implement a dispatcher by defining the role in `Players.h` and `Players.cpp`, as well as creating a new `Action` and corresponding `ActionConstructor` specifically for dispatcher movements, which are all "regular" movements of a pawn by a player. If this Constructor were then included in the `GameLogic`, it should work? Writing in the Action legality guards and list_actions would be hard.
### Cards
- "Forecast" and "Resilient Population" cards have been removed from the game. The former has factorial (in discard size) branching factor when considered as a possible player action, and so is intractable. If considered only as a "random" action, it's a potentially damaging or helpful just like existing stochasticity that's already in the game, and so really doesn't add anything to the problem at hand. Potential implementations could include some hard rules about "allowable" orders, but I suspect even then it'd be a large branching factor. Further, I wouldn't consider an agent that uses human-coded notions of "good" and "bad" card orders to have figured out anything significant on it's own. "Resilient population" might be interesting to implement in the future, since it allows players to directly alter the distribution of cards after the next Epidemic "Intensify" step. At least right now, though, I'm not trying to find out the degree to which agents can understand and learn to manipulate the distribution represented by the current infect discard pile. These cards also don't jibe with the current determinization search-tree. 

### Movement and Actions
- Right now the `GameLogic` precludes the use of any event cards outside of the players 4 actions. This means that if the agent does't use an available card by the "draw player card" phase of the game, they won't be able to use it until the start of the next player turn. I suspect this is a mild handicap to performance compared to canon game rules, since it prevents agents from capitalizing on information as it's revealed during the stochastic phases of the game (for example Airlifting a medic or quarantine specialist to the site of a city who will now with certainty be drawn from the infection deck). This helps reduce the number of decision nodes and branching factor - for example Airlift has ~n_players*n_cities options. What if this were considered between every single card draw?
- The `Actions` all have `legal()` guards, whose truth value indicates whether or not there's at least one *legal and not _entirely_ stupid* move to make. Their `random_action` and `list_actions` incorporate the same rules for restricting which actions can be returned. (e.g. it's stupid to move from a city and back, or discard the card of the city you're in to move to the city you're in - even though they're both legal!).
    - `Airlift`, `DirectFlight`, and `CharterFlight` will additionally not count the movement of a player to a neighboring city as valid. For `DirectFlight` and `CharterFlight` this is a strict optimization, since discarding a card to move one city away can never be better than just moving to it. In the case of `Airlift` it's more of an almost-always optimization designed to reduce the branching factor.
    - `GovernmentGrant` and `QuietNight` can only be used at the beginning of a player turn. This is just a search optimization that uses the fact that if such a card would be good to use _during_ the player turn, then one might as well use it at the beginning. This is a modest search-space reduction.
    - `Do Nothing` is only an available action if it's the beginning of a player turn or a player has "done something" (built, cured, treated, or traded). This is a search restriction that might slightly handicap very weak agents because it could prevent course correction, but for more capable agents serves as a branching reduction.
    - `Direct Flight` is only available if it's the beginning of a player turn or a player has "done something" (built, cured, treated, or traded). This is a search restriction that might slightly handicap very weak agents because it could prevent course correction, but for more capable agents serves as a branching reduction.
    - `Airlift` is only available if it's the beginning of a player turn or the player has "done something" (built, cured, treated, or traded). This is a search restriction that might slightly handicap very weak agents because it could prevent course correction, but for more capable agents serves as a branching reduction.
- The `GameLogic` will ALWAYS return a `ForcedDiscard` actions (or list of actions) if ANY player has >7 cards. Among available actions are the use of any event cards in that players hand. 
- the `Cure` action will always discard the first 4 (if scientist) or 5 (if not) cards whose color matches the color to be cured, rather than giving the player a choice over exactly which they want to discard in the case that they have more than necessary. This is nothing but a search optimization.

## Pandemic Board Game

The rules can be found [here](https://images-cdn.zmangames.com/us-east-1/filer_public/25/12/251252dd-1338-4f78-b90d-afe073c72363/zm7101_pandemic_rules.pdf), which is nice because you can also see a game map.

## Further optimization and improvements (probably never will be done)
- The most obvious improvement would be to provide an API for external tools. Making agents only in C++ is not sustainable. Particularly an API for python would be useful.

- Existing notion of `Node` keeps an `Action*` pinned on it (the action that was taken to arrive at that node). Since actions are always instantiated on-the-spot, this means there's a huge amount of wasted space and time in creating and storing new actions that have already been defined somewhere else in the tree (E.g. `DoNothing`). A better option would be to instantiate _all possible_ actions as `inline` lists of pointers on program startup (which isn't hella bad since there's only O(10^5) actions for all possible games), and constructors always return actions from that list. Presumably would be a lot better time & space performance. This change would require a lot of changes everywhere that an action is deleted, since you would *NOT* want to delete these shared actions.

- Right now the only search tool (`KDeterminizedGameTree`) requires that stochastic transitions are the same on all branches of the tree. This makes it so it wouldn't work if `Forecast` or `Resilient Population` were implemented, since it means determinizations in different parts of the tree would be different when those cards are used. So would require a new tree!

- There should be some equality measure between board states to allow for state search. This would rely, to some degree, on an opinionated decision of what elements of the board would designate it as "the same" as another. It would allow for a graph search implementation, though, which could probably do better.

### Known bugs

When a disease is already cured, infect cards should not result in the placement of that disease at the location of the medic. Right now the logic will still place disease there. I don't suspect this has a significant impact on games, since this circumstance is relatively rare. In addition, once a disease is cured the medic's position will properly be cleared of disease (as well as after each movement action), so this inappropriate placement could only ever be placing a single disease cube on a 0-cube city.
