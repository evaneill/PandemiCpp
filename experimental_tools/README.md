## Scenarios ##

In the abstract, scenarios are entirely or somewhat pre-specific game situations in which to test an agent. I'm using this to help test game logic and, eventually, perhaps build some scenarios to evaluate strategic capabilities.

In a more technical sense, I represent scenarios with functions (that may or may not require inputs) that return a `Board::Board` object. Each such function has to  instantiate and then alter a board, returning the result to the caller. I _think_ this is only going to be called by whatever has to instatiate `GameLogic` with a board.

