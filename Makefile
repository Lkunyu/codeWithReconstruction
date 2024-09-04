TOP=$(shell pwd)
SRC=$(TOP)/src
INC=$(TOP)/include
LIB=$(TOP)/lib

OBJ=$(patsubst %.C,$(LIB)/%.o,$(notdir $(wildcard $(SRC)/*.C)))
libso=$(patsubst %.C,-l%,$(notdir $(wildcard $(SRC)/*.C)))

target: ReadData T0Cali T0Result ReadTracker CombineData RecData

ReadData: $(LIB)/ReadData.o $(OBJ)
	@echo " ==> Building "$(notdir $@)
	@g++ `root-config --cflags --libs` $^ -o $@ `root-config --glibs`

ReadTracker: $(LIB)/ReadTracker.o $(OBJ)
	@echo " ==> Building "$(notdir $@)
	@g++ `root-config --cflags --libs` $^ -o $@ `root-config --glibs`

T0Cali: $(LIB)/T0Cali.o $(OBJ)
	@echo " ==> Building "$(notdir $@)
	@g++ `root-config --cflags --libs` $^ -o $@ `root-config --glibs`

T0Result: $(LIB)/T0Result.o $(OBJ)
	@echo " ==> Building "$(notdir $@)
	@g++ `root-config --cflags --libs` $^ -o $@ `root-config --glibs`

CombineData: $(LIB)/CombineData.o $(OBJ)
	@echo " ==> Building "$(notdir $@)
	@g++ `root-config --cflags --libs` $^ -o $@ `root-config --glibs`

RecData: $(LIB)/RecData.o $(OBJ)
	@echo " ==> Building "$(notdir $@)
	@g++ `root-config --cflags --libs` $^ -o $@ `root-config --glibs`



$(LIB)/%.o: $(SRC)/%.C
	@echo " ==> Compiling "$(notdir $@)
	@mkdir -p lib; g++ -I$(INC) `root-config --cflags --libs` -c $^ -o $@

$(LIB)/%.o: %.C
	@echo " ==> Compiling "$(notdir $@)
	@mkdir -p lib; g++ -I$(INC) `root-config --cflags --libs` -c $^ -o $@

make clean:
	@echo " ==> Removing files: ReadData T0Cali T0Result ReadTracker CombineData RecData lib"
	@rm -rf ReadData T0Cali T0Result ReadTracker CombineData RecData lib
