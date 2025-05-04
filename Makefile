SHELL = cmd

DEPEND = 
BIN = Bin
HEADER = Header
SOURCE = Source/*
TESTS = Tests/*
NAME = CollectionsPlus

DLL := $(BIN)/lib$(NAME).dll
TESTS_EXE := $(BIN)/Tests.exe

HEADERS_WILDCARD = ../*/Header
HEADERS := $(subst $() , -I , $(wildcard $(HEADERS_WILDCARD)))

All: $(DLL) $(TESTS_EXE)
	$(TESTS_EXE)

$(DLL): $(SOURCE) $(HEADERS_WILDCARD)/*
	gcc -fPIC -shared $(SOURCE) $(HEADERS) -o $(DLL)

$(TESTS_EXE): $(DLL) $(TESTS) $(HEADERS_WILDCARD)/*
	gcc $(TESTS) $(HEADERS) -L$(BIN) -l$(NAME) -o $(TESTS_EXE)

Links:
	$(subst &END,,$(foreach DEPENDENCY, $(DEPEND),cmd /C mklink $(BIN)\lib$(DEPENDENCY).dll ..\$(DEPENDENCY)\$(BIN)\lib$(DEPENDENCY).dll /H &)END)

Clean:
	del /Q $(subst /,\,$(TESTS_EXE) $(DLL))
