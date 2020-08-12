
ifeq ($(strip $(APP)),)
  APP := test
endif

ifeq ($(strip $(MOD)),)
  MOD := $(shell basename $(shell pwd))
endif

SLF := $(dir $(lastword $(MAKEFILE_LIST)))

include $(SLF)/defines.mk

LPP := $(wildcard *.lpp)
YPP := $(wildcard *.ypp)
LGN := $(LPP:%.lpp=%.yy.cpp)
YGN := $(YPP:%.ypp=%.tab.cpp)

BOB := $(TGT)/$(APP)/obj/$(MOD)
BLB := $(TGT)/$(APP)/lib

SRC := $(wildcard *.cpp) $(LGN) $(YGN)
OBJ := $(SRC:%.cpp=$(BOB)/%.o)
DPS := $(SRC:%.cpp=$(BOB)/%.d)
HDR := $(SRC:.cpp=.h)
LIB := $(BLB)/lib$(MOD).a

CXXFLAGS := $(CFLAGS) $(FEA) $(DBG) $(INC) -Wno-deprecated -Wno-multichar -Wno-psabi

all: $(LGN) $(YGN) $(LIB)

clean:
	rm -rf $(BOB)
	rm -f $(LIB)
	rm -f *.o *.yy.cpp *.tab.h *.tab.cpp *.output

$(LIB): $(OBJ)
	rm -f $@
	mkdir -p $(BLB)
	ar cq $@ $(OBJ)

#$(BOB)/%.o: %.cpp | $(BOB)
#	$(CPP) $(CXXFLAGS) -MMD -MP -MF"$@" -c -o $@ $<
#	$(CPP) $(CXXFLAGS) -MMD $(patsubst $(BOB)/%.o,$(BOB)/%.d,$@) -c -o $@ $<
#	$(CPP) $(CXXFLAGS) -MM -MF $(patsubst $(BOB)/%.o,$(BOB)/%.d,$@) -c -o $@ $<
#	$(CPP) $(CXXFLAGS) -c -o $@ $<

$(BOB)/%.o: %.cpp | $(BOB)
	$(CPP) $(CXXFLAGS) -c -o $@ $<

#$(BOB)/%.d: %.cpp | $(BOB)
#	$(CPP) $(CXXFLAGS) -MF"$@" -MG -MM -MD -MP -MT"$@" -MT"$(OBJ)" $<

# -MF  write the generated dependency rule to a file
# -MG  assume missing headers will be generated and don't stop with an error
# -MM  generate dependency rule for prerequisite, skipping system headers
# -MP  add phony target for each header to prevent errors when header is missing
# -MT  add a target to the generated dependency

$(BOB):
	mkdir -p $@

%.yy.cpp: %.lpp
	flex -o $@ $<

%.tab.cpp: %.ypp
	bison -v --defines=$(@:$.cpp=h) -o $@ -t -d $<

.PHONY: clean all

-include $(DPS)

