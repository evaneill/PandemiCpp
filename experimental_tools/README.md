# Experimental Tools #

## Experiments ##

Experiments are in some sense the "final" culmination of all the objects in the repository. As I use it, an _experiment_ is a combination of 5 things:
- An Agent that will play games
- A Scenario into which the agent is placed at the beginning of every game (which may or may not incorporate randomness, according to it's definition)
- A choice of measurements of the games played by the agent 
- A number of games to play
- A location to write the results to

An experiment, then, is a means of capturing the all of the quantifiable behavior of an agent in a controlled and pre-set way. 

Experiments will write results to files that describe the experiment context (which agent, which scenario, and how many games), and the results (the final measurement for each played game). It would also be a good idea to be configurable to either break when the game enters illegal states or log the full game log to a special file on illegal state, then break.

## Measurements ##

I'm using _measurement_ to mean a number that represents some property of a single game played by an agent. Examples of measurements are: the final reward achieved by an agent, the minimum/maximum/average branching factor, the game depth, etc. 

Here I implement measurements somewhat like `Actions`, in that they are a class that keeps a pointer to the board being played by the agent. At each game step, measurements are told to update by the `Experiment` to which they belong. It's possible that measurements aren't updated until the end of the game, and it's also possible that each step requires a new recalculation. At the end of each game played, the agent will write the final measurement values to the output location.

## Scenarios ##

In the abstract, _scenarios_ are entirely or somewhat pre-specific game situations in which to test an agent. I'm using this to help test game logic and, eventually, perhaps build some scenarios to evaluate strategic capabilities.

In a more technical sense, I represent scenarios with classes that have `name` and `description` attributes and a `make_board` method that return a `Board::Board` object (and always requires a `roles` and `difficulty` argument, though this method may not use them). These classes are used in the definition of an `Experiment`.

