CC	:= g++
CXXFLAGS := -w -std=c++11 -c

BINS	:= RedBase dbcreate dbdestroy
SRCS	:= $(wildcard *.cpp)
OBJS	:= $(SRCS:.cpp=.o)
BINOS	:= $(addsuffix .o, $(BINS))
TEMP_OBJ=$(filter-out $(BINOS), $^)

.PHONY: all clean

all:$(BINS)

$(BINS):$(OBJS)
	@echo "正在链接程序....";
	$(foreach BIN, $@, $(CC) $(TEMP_OBJ) $(BIN).o  -o $(BIN))

%.o:%.c
	@echo "正在编译代码....";
	$(CC) -c $< -o $@

clean:
	rm -rf *.o $(BINS)
