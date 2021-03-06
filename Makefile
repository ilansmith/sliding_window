CC=gcc
CPP=cpp
#CFLAGS=-ansi -pedantic-errors -Werror -Wall -g
CFLAGS=-Werror -Wall -g
APP=slw

OBJS=sliding_window.o sliding_window_test.o

all: $(APP)

%.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

$(APP): $(OBJS)
	$(CC) -o $@ $^

clean:
	@echo "removing object files"
	@ls *.o 2>/dev/null | grep -v $(APP) | xargs rm -f

cleanall: clean
	@echo "removing tag file"
	@rm -rf tags
	@echo "removing $(APP)"
	@rm -f $(APP)

