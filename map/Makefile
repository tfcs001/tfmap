#for tfc

CXX = g++

TARGET = tf_map

INC = -I. -I../common -I../config -I../log

CFLAGS += -Wall -O0 -g   -std=c++11 -DSINGLE

CFLAGS += $(INC)
CFLAGS += $(DEFS)

LDFLAGS += -lpthread

LDFLAGS += -L/usr/local/lib -L/usr/lib64/ 

DIRS = .  ../common ../config ../log

FILES = $(foreach dir,$(DIRS),$(wildcard $(dir)/*.c))
CPPFILES = $(foreach dir,$(DIRS),$(wildcard $(dir)/*.cpp))

# all object files.
OBJ = $(patsubst %.c,%.o,$(FILES))
CPPOBJ += $(patsubst %.cpp,%.o,$(CPPFILES))

all:$(OBJ) $(CPPOBJ)
	$(CXX) $(CFLAGS) -o $(TARGET) $(OBJ) $(CPPOBJ) $(LDFLAGS) $(LIBS)
	@echo "+----------------------------------------------------+"
	@echo "+         $(TARGET) build successfully!         +"
	@echo "+----------------------------------------------------+"

%.o:%.cpp
	$(CXX) $(CFLAGS) -c -o $@ $<

install:
	@cp -vf ./$(TARGET).conf  /etc/
	@cp -vf ./$(TARGET)       /usr/bin/
uninstall:
	@rm -vf /etc/$(TARGET).conf
	@rm -vf /usr/bin/$(TARGET)

clean:
	@rm -vf $(OBJ) $(CPPOBJ)
	@rm -vf $(TARGET)


