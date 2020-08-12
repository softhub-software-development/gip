
ifeq ($(strip $(PLF)),)
  PLF=Linux
endif

ifeq ($(strip $(TGT)),)
  TGT=../../../build/$(PLF)
endif

include $(TGT)/generic.mk

