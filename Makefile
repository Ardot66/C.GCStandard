ifeq ($(OS), Windows_NT)
	LIB_BIN = ../Bin
	SHELL = bash
	SHARED_EXT = .dll
	EXEC_EXT = .exe
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S), Linux)
		LIB_BIN = Lib
		SHARED_EXT = .so
	endif
endif

TESTS_BIN = Bin
NAME = GCStandard

OBJECT := $(LIB_BIN)/lib$(NAME)$(SHARED_EXT)
TESTS_EXE := $(TESTS_BIN)/Tests$(EXEC_EXT)
RUN := $(TESTS_EXE)

HEADERS := Header
COMPILE_FLAGS = -Wall -Wextra -pedantic

Debug: COMPILE_FLAGS += -g
Debug: Compile

Release: COMPILE_FLAGS += -s
Release: Compile

Debugger: RUN = gdb $(TESTS_EXE)
Debugger: Debug

Compile: $(OBJECT) $(TESTS_EXE)
	$(RUN)

$(OBJECT): Source/*.c $(HEADERS)/*.h
	gcc $(COMPILE_FLAGS) -fPIC -shared Source/*.c -I $(HEADERS) -lbacktrace -lm -o $(OBJECT)

$(TESTS_EXE): $(OBJECT) Tests/*.c $(HEADERS)/*.h
	gcc $(COMPILE_FLAGS) Tests/*.c -I $(HEADERS) -L$(LIB_BIN) -l$(NAME) -o $(TESTS_EXE)

Clean:
	rm -f $(TESTS_EXE) $(OBJECT)