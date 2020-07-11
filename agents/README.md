# Agents

All agents are responsible for having a method `take_step()` that uses the `GameLogic` attribute to advance the game state using a selected action. Each agent can request a legal action (or list of all legal actions) from the `GameLogic`.

Restrictions to legality are described on the main page, and make it so that play isn't 100% loyal to original game. Many changes are reductions of strictly dominated actions, while some are restrictions that don't end up actually changing the power of agents (e.g. making `Quiet Night` only legal at the beginning of a turn).

## Random Agents

Random agents all call on specific methods in `GameLogic` that return actions according to prescribed randomness. These methods are essentially the logic of the agents. 

In both kinds of random methods, `DoNothing` isn't an allowed action if `Treat` or `Cure` is legal. Further, if `Cure` is legal and some conservative checks on disease cube counts and outbreaks are met, then `Cure` is the only returned action.

### Uniform Random
Actions are drawn uniformly over all legal actions. 

### ByGroup Random
The _type_ of action is chosen from among _types_ of legal actions uniformly (e.g. `Move`, `Treat`, `DirectFlight`,...), and then a legal action of that _type_ is chosen.

### ListAction Random (unimplemented/tested)

This requests the full list of legal actions then chooses on at random. This is therefore the same as Uniform but with a lot more effort. This was used for testing `GameLogic`, but should *not* be used, since it represents a huge memory leak (it never deletes the actions that the game logic creates).

## Search Agents

These agents select highest-UCB1 score children in order to select an action once the simulation budget is used. Some use Max Child (max of (max child reward) + confidence bound, defined iteratively), which incorporates a rough "convergence criteria" for nodes that forces them to either return their score if not meeting the convergence criteria, or max of average children rewards if they do.

### Rollout-Based UCT MCTS

Heuristics (or no heuristic - `Naive`) are used to evaluate the terminal state reached after rollouts from tree-leaf states. The returned heuristic value is used for backup. The intuition behind doing this is that a default policy is incredibly unlikely to win, and so some measure of "closeness" to winning achieved by game-end is used instead of 1-0 win/loss reward. By default they'll choose the highest-UCB1 child successor. Some use greedy expectimax child selection (`MaxChild`) calculated on the partial game tree.

Right now pretty much all of these agents use the `ByGroup` randomness in their default policy.

There is one agent that uses a heavy rollout ("SmartRollout") incorporating epsilon-greedy child selection based on a compound heuristic comprised of (2/3)-(1/3) weighted combination of cure precondition and "smart" loss proximity. B/c this implementation is kind of clumsy, it is very costly to calculate this because you have to generate, alter, and evaluate a copy of the current state to value each child. 

### Heuristic Evaluation-based UCT MCTS

With this kind of agent, all of the same steps of MCTS are used to grow a partial game tree. Instead of rollout-based value estimation, though, heuristics are used to evaluate the leaf states of a tree as it grows. Because the heuristics are human-coded estimates of state value, they are uncertain and so upper confidence bounds are kept just like in plain MCTS. All of these use greedy expectimax child-selection.
