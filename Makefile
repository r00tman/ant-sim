all: compile run
compile:
	g++ ./main.cpp -o main -O0 -g -DLOCAL -std=c++11 -lGL -lglfw
	g++ ./visbase.cpp -o visbase -O0 -g -DLOCAL -std=c++11 -lGL -lglfw
run:
	./main

run-vis:
	./visbase
