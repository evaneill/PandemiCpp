test_map:
	g++ -std=c++17 -o tests/map_test.out game_files/Map.cpp tests/test_map.cpp
	tests/map_test.out
test_playerdeck:
	g++ -g -std=c++17 -o tests/playerdeck_test.out game_files/Map.cpp game_files/Decks.cpp tests/test_playerdeck.cpp game_files/Board.cpp game_files/Actions.cpp game_files/StochasticActions.cpp game_files/Players.cpp
	tests/playerdeck_test.out
test_randomness:
	g++ -O3 -std=c++17 -o tmp.out tests/test_randomness.cpp
	./tmp.out
test_temp:
	g++ -g -std=c++17 -I tests/test.h -o tmp.out tests/test.cpp 
	./tmp.out
test_infectdeck:
	g++ -std=c++17 -o tests/infectdeck_test.out game_files/Map.cpp game_files/Decks.cpp tests/test_infectdeck.cpp
	tests/infectdeck_test.out
test_players:
	g++ -std=c++17 -o tests/players_test.out tests/players_test.cpp game_files/Map.cpp game_files/Players.cpp
	tests/players_test.out
test_gamelogic:
	c++ -std=c++17 -o tests/gamelogic_test.out -I game_files/ game_files/*.cpp tests/test_gamelogic.cpp
	tests/gamelogic_test.out
test_board:
	c++ -std=c++17 -o tests/board_test.out -I game_files/ game_files/*.cpp tests/test_board.cpp
	tests/board_test.out
test_infectcity:
	c++ -std=c++17 -o tests/infectcity_test.out game_files/Board.cpp tests/test_infectcity.cpp game_files/Map.cpp game_files/Decks.cpp game_files/Players.cpp
	tests/infectcity_test.out
playtest:
	g++ -g -O3 -march=native -std=c++17 -I game_files/ -I agents/ game_files/*.cpp agents/ByGroupRandomAgent.cpp agents/Agents.cpp experimental_tools/Scenarios.cpp Playtest.cpp -o playtest.out
test:
	g++ -g -O3 -march=native -std=c++17 -I game_files/ -I agents/ -I experimental_tools/ -I agents/search_tools/ game_files/*.cpp agents/*.cpp experimental_tools/*.cpp agents/search_tools/*.cpp $(TESTFILE).cpp -o $(TESTFILE).out
experiment:
	g++ -g -O3 -march=native -std=c++17 -I game_files/ -I agents/ -I experimental_tools/ -I agents/search_tools/ game_files/*.cpp agents/*.cpp experimental_tools/*.cpp agents/search_tools/*.cpp $(EXPERIMENT).cpp -o $(EXPERIMENT).out