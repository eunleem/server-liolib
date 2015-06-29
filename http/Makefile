CC=g++
ROOT_DIR=$(HOME)/devel

LIOLIB_DIR=$(ROOT_DIR)/liolib

CFLAGS=-Wall -std=c++11

SRC_EXT=.cpp
OBJ_EXT=.o
DEBUG_OBJ_EXT=_debug$(OBJ_EXT)
RELEASE_OBJ_EXT=_release$(OBJ_EXT)

OBJ_DIR=obj
SOURCES=$(wildcard *$(SRC_EXT))

RELEASE_OBJ=$(patsubst %$(SRC_EXT),$(OBJ_DIR)/%$(RELEASE_OBJ_EXT),$(SOURCES))
DEBUG_OBJ=$(patsubst %$(SRC_EXT),$(OBJ_DIR)/%$(DEBUG_OBJ_EXT),$(SOURCES))

GMOCK_DIR=$(ROOT_DIR)/gmock
GTEST_DIR=$(GMOCK_DIR)/gtest

LIBRARY_DIR=-L$(ROOT_DIR)/libs
INCLUDE_DIR=-I$(ROOT_DIR)

LIBRARIES=-lz

DEBUG_FLAGS=-O0 -ggdb
RELEASE_FLAGS=-O3

OBJECT_FLAGS=-c $(CFLAGS) -O3 -pg #-ggdb #-pg 
EXECUTABLE_FLAGS=$(CFLAGS) -O3 -pg #-ggdb #-pg 

GMOCK_INCLUDE_DIR=-isystem $(GTEST_DIR)/include -isystem $(GMOCK_DIR)/include
GMOCK_LIB=-lgmock
GMOCK_FLAGS=-pthread



OBJECTS=$(SOURCES:.cpp=.o)

UNIT_TEST_OFF=_UNIT_TEST false
UNIT_TEST_ON=_UNIT_TEST true

DEBUG_OFF=_DEBUG false
DEBUG_ON=_DEBUG true

UNITTEST= \
	rm -f $(1).exe; \
	echo -e "\n\e[1;32m=============== START COMPILING ===============\e[0m"; \
	echo -e "Compiling \e[1;36m$(1)\e[0m module Unit Test..."; \
	echo "  Enabling UnitTest by modifying the UNIT TEST LINE."; \
	echo -e "\e[1;34m=============== COMPILER MESSAGE ===============\e[0m"; \
	sed -i.bak "s/${UNIT_TEST_OFF}/${UNIT_TEST_ON}/g" $(1).cpp; \
	$(CC) $(EXECUTABLE_FLAGS) $(INCLUDE_DIR) $(LIBRARY_DIR) $(1).cpp $(2) $(LIBRARIES) -o "$(1).exe"; \
	sed -i.bak "s/${UNIT_TEST_ON}/${UNIT_TEST_OFF}/g" $(1).cpp; \
	read -p "Press any key to continue... " -n1 -s; \
	echo -e "\n\e[1;34m=============== RUNNING TEST ===============\e[0m"; \
	./$(1).exe; \
	echo -e "\e[1;31m=============== END OF TEST ===============\e[0m\n" 

GMOCK_TEST= \
	sed -i.bak "s/${UNIT_TEST_OFF}/${UNIT_TEST_ON}/g" $(1).cpp; \
	sed -i.bak "s/${DEBUG_OFF}/${DEBUG_ON}/g" $(1).hpp; \
	$(CC) $(EXECUTABLE_FLAGS) $(GMOCK_FLAGS) $(INCLUDE_DIR) $(GMOCK_INCLUDE_DIR) $(LIBRARY_DIR) $(1).cpp $(2) $(LIBRARIES) $(GMOCK_LIB) -o "$(1)_Test.exe"; \
	sed -i.bak "s/${UNIT_TEST_ON}/${UNIT_TEST_OFF}/g" $(1).cpp; \
	sed -i.bak "s/${DEBUG_ON}/${DEBUG_OFF}/g" $(1).hpp; \
	./$(1)_Test.exe
	

COMPILE= \
	rm -f $(1).exe; \
	echo -e "\n\e[1;32m=============== START COMPILING ===============\e[0m"; \
	echo -e "Compiling \e[1;36m$(1)\e[0m..."; \
	echo -e "\e[1;34m=============== COMPILER MESSAGE ===============\e[0m"; \
	sed -i.bak "s/${UNIT_TEST_ON}/${UNIT_TEST_OFF}/g" $(1).cpp; \
	$(CC) $(EXECUTABLE_FLAGS) $(INCLUDE_DIR) $(LIBRARY_DIR) $(3).cpp $(2) $(LIBRARIES) -o "$(1).exe" ; \
	echo -e "\e[1;33m=============== COMPILER MESSAGE END ===============\e[0m"; \
	echo -e "\nDONE: \e[1;33m$@\e[0m."

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECT_FLAGS) $(OBJECTS) -o $@


HttpRequest: 
	@$(call GMOCK_TEST,$@,$^)

HttpRequestParser: $(LIOLIB_DIR)/Util.o $(LIOLIB_DIR)/CustomExceptions.o $(LIOLIB_DIR)/StringMap.o 
	@$(call UNITTEST,$@,$^)

HttpResponseBuilder: $(LIOLIB_DIR)/Util.o $(LIOLIB_DIR)/CustomExceptions.o
	@$(call UNITTEST,$@,$^)


HttpClient: $(LIOLIB_DIR)/Socket.o $(LIOLIB_DIR)/Util.o $(LIOLIB_DIR)/CustomExceptions.o
	@$(call UNITTEST,$@,$^)

Http:
	@$(call UNITTEST,$@,$^)


.cpp.o:
	@echo -e "Compiling \e[1;33m$<\e[0m for dependency..."
	@$(CC) $(OBJECT_FLAGS) $(INCLUDE_DIR) $(LIBRARY_DIR) $< $(LIBRARIES) -o $@

clean:
	rm -rf *.o *.exe $(EXECUTABLE)
	@echo "Object files and Executables are removed!"
