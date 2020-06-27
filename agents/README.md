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

Heuristics are used to evaluate the terminal state reached after rollouts from tree-leaf states. The returned heuristic value is used for backup.

Right now all of these agents use the `ByGroup` randomness in their default policy.

### Heuristic Evaluation-based UCT MCTS

Heuristics are used to evaluate the leaf states of a tree as it grows. Because the heuristics are human-coded estimates of state value, they are uncertain and so upper confidence bounds are kept just like in plain MCTS. All of these use Max Child selection policy.