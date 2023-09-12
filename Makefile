##
# Project Title
#
# @file
# @version 0.1

OUT = ./nlwtn
CC = clang++
ODIR = obj
SDIR = src
CXXFLAGS := -c -g -O0 --std=c++17 -lsquish -llz4 -lz

_OBJS = main.o
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))

$(ODIR)/%.o: $(SDIR)/%.cpp
	$(CC) --std=c++17 -c -g -o $@ $< $(CFLAGS)

$(OUT): $(OBJS)
	$(CC) $(CXXFLAGS) -o $(OUT) $^

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o $(OUT)

# end
