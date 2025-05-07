SHELL = cmd

DEPEND = $() Try
BIN = ../Bin
HEADER = Header
SOURCE = Source/*
TESTS = Tests/*
NAME = CollectionsPlus

DLL := $(BIN)/lib$(NAME).dll
TESTS_EXE := $(BIN)/$(NAME)Tests.exe

HEADERS_WILDCARD = ../*/Header
HEADERS := $(subst $() , -I , $(wildcard $(HEADERS_WILDCARD)))

All: $(DLL) $(TESTS_EXE)
	$(TESTS_EXE)

$(DLL): $(SOURCE) $(HEADERS_WILDCARD)/*
	gcc -fPIC -shared $(SOURCE) $(HEADERS) -L$(BIN) $(subst $() ,-l,$(DEPEND)) -o $(DLL)

$(TESTS_EXE): $(DLL) $(TESTS) $(HEADERS_WILDCARD)/*
	gcc $(TESTS) $(HEADERS) -L$(BIN) -l$(NAME) -o $(TESTS_EXE)

Clean:
	del /Q $(subst /,\,$(TESTS_EXE) $(DLL))
