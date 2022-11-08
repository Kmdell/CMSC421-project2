CXX = gcc
CXXFLAGS = -Wall

all: monolithic_test race0 race1 init shut

monolithic_test: monolithic_test.c
	$(CXX) $(CXXFLAGS) monolithic_test.c -o monolithic_test

init: initialize.c
	$(CXX) $(CXXFLAGS) initialize.c -o init

shut: shutdown.c
	$(CXX) $(CXXFLAGS) shutdown.c -o shut

race0: race0.c
	$(CXX) $(CXXFLAGS) race0.c -o race0

race1: race1.c
	$(CXX) $(CXXFLAGS) race1.c -o race1

run_monotlithic:
	./monolithic_test

run_init:
	./init

run_shut:
	./shut

run_race:
	./race0 & ./race1