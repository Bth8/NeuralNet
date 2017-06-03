#!/usr/bin/make -f

TARGET		= net

CXX			= g++
RM			= rm -f

CPPFLAGS	= -MD -Wall -Werror -Weffc++ -std=c++11
CXXFLAGS	= -O3
LDFLAGS		=
LDLIBS		=

SRCFILES = neuron.cpp synapse.cpp network.cpp main.cpp noise.cpp \
	network_param.cpp connectome.cpp
SRCDIR = src/
SRCS = $(addprefix $(SRCDIR), $(SRCFILES))
OBJS = $(SRCS:.cpp=.o)
DEPS = $(OBJS:.o=.d)

all: $(TARGET)

condor: CXX = condor_compile g++
condor: LDLIBS += dl-support.o
condor: dl-support.o $(TARGET)

debug: CPPFLAGS += -DDEBUG
debug: CXXFLAGS += -g -O0
debug: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

dl-support.o:
	$(AR) -x /lib64/libc.a dl-support.o

clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

.PHONY: all clean debug

-include $(DEPS)