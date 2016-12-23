CC = g++
DEBUG = -g
CFLAGS = -Wall -c
LFLAGS = -Wall -pthread

Sim04 : Sim04.o SimulatorFunctions.o SimpleQueue.o
	$(CC) $(LFLAGS) Sim04.o SimulatorFunctions.o SimpleQueue.o -o Sim04

Sim04.o : Sim04.cpp SimulatorFunctions.h SimpleQueue.h
	$(CC) $(CFLAGS) Sim04.cpp

SimulatorFunctions.o : SimulatorFunctions.cpp SimulatorFunctions.h
	$(CC) $(CFLAGS) SimulatorFunctions.cpp
	
SimpleQueue.o : SimpleQueue.h SimpleQueue.cpp
	$(CC) $(CFLAGS) SimpleQueue.cpp
	
clean:
	\rm *.o Sim04

