
APP := gip
PLF := Linux
OPT := -O0
JOB := -j2
#DBG := -g -D_DEBUG -D_DEBUG_PERF=0 -DMEM_LEAK_CHECK=0
DBG := -g
FEA := -DFEATURE_HAL_POOL_THREADS=0 -DFEATURE_HAL_UTILS=1

CFLAGS := $(DBG) $(OPT) -std=c++11 -DEXPERIMENTAL=0 $(FEA) -funwind-tables
LFLAGS := -lpthread -lgip -lgeography -lnet -lbase -lutil -lhal -rdynamic

export APP PLF CFLAGS LFLAGS

include ../defines.mk

all: ${APP}

clean:
	cd $(BCO)/base; make clean
	cd $(BCO)/hal; make clean
	cd $(BCO)/net; make clean
	cd $(BLI)/geography; make clean
	cd $(BLI)/util; make clean
	cd $(BAP)/gip; make clean
	rm -f *.o nohup.out
	rm -rf obj lib
	rm ${APP}

${APP}:
	date
	cd $(BCO)/base; make ${JOB} all
	cd $(BCO)/hal; make ${JOB} all
	cd $(BCO)/net; make ${JOB} all
	cd $(BLI)/geography; make ${JOB} all
	cd $(BLI)/util; make ${JOB} all
	cd $(BAP)/gip; make ${JOB} all
	rm -f $@
	$(CPP) -o $@ -Llib $(LFLAGS)
	date

.cpp.o:
	$(CPP) $(CFLAGS) $(INC) -Wno-deprecated -Wno-multichar -c $<

.PHONY: all clean ${APP}

