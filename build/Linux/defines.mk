
MAP := application
MCO := common
MLI := library

BAS := ../../../src
BAP := $(BAS)/$(MAP)
BCO := $(BAS)/$(MCO)
BLI := $(BAS)/$(MLI)
INC := -I.. -I$(BCO) -I$(BLI) -I$(BAP)
CPP := g++
DBG :=

