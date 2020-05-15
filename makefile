test_map:
	g++ -std=c++11 -o tests/map_test.out game_files/Map.cpp tests/test_map.cpp
	tests/map_test.out
test_playerdeck:
	g++ -std=c++11 -o tests/playerdeck_test.out game_files/Map.cpp game_files/Decks.cpp tests/test_playerdeck.cpp
	tests/playerdeck_test.out
test_randomness:
	g++ -std=c++11 -o tmp.out tests/test_randomness.cpp
	./tmp.out
test_temp:
	g++ -std=c++11 -o tmp.out tests/test.cpp
	./tmp.out
test_infectdeck:
	g++ -std=c++11 -o tests/infectdeck_test.out game_files/Map.cpp game_files/Decks.cpp tests/test_infectdeck.cpp
	tests/infectdeck_test.out
test_players:
	g++ -std=c++11 -o tests/players_test.out tests/players_test.cpp game_files/Map.cpp game_files/Players.cpp
	tests/players_test.out
