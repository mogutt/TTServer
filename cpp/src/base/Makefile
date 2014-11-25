LIBBASE = libbase.a

COMMON_PATH := ./
SRCDIRS   := ./
CPPFLAGS  += -fPIC  -I$(COMMON_PATH)
CPPFLAGS += -Wall -Wno-deprecated -g

SRCEXTS   := .c .cpp
CC      = g++
SHELL   = /bin/sh
SOURCES = $(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*,$(SRCEXTS))))
OBJS= $(patsubst %.cpp,./obj/%.o,$(notdir $(SOURCES)))
GCNOS= $(patsubst %.cpp,./obj/%.gcno,$(notdir $(SOURCES)))

all: $(LIBBASE)

$(LIBBASE): $(OBJS)
	ar -r $@ $(OBJS)

objs : $(OBJS)
./obj/%.o : %.c
	gcc -c $(CPPFLAGS) $(CFLAGS) $< -o $(patsubst %.c,%.o, $<)
./obj/%.o : %.cpp
	@mkdir -p $(dir $@)
	@echo "=================="
	@echo "Compiling $<"
	$(CC) -c $(CPPFLAGS)  $< -o $@

.PHONY : clean
clean: 
	rm -f $(OBJS)
	rm -f $(LIBBASE)
