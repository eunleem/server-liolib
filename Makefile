DATE=`date +"%Y-%m-%d"`
ARCH=`uname -a | rev | cut -d ' ' -f 2 | rev`

CC=g++
CFLAGS=-Wall -std=c++11 -O2 -ggdb

SRC_EXT=.cpp
OBJ_EXT=.o

ROOT_DIR=$(HOME)/devel

LIB_DIR=-L$(ROOT_DIR)/libs
INCLUDE_DIR=-I$(ROOT_DIR)

GMOCK_DIR=$(ROOT_DIR)/gmock
GTEST_DIR=$(GMOCK_DIR)/gtest

GMOCK_INCLUDE_DIR=-isystem $(GTEST_DIR)/include -isystem $(GMOCK_DIR)/include
GMOCK_LIB=-lgmock
GMOCK_FLAGS=-pthread

LIBS=-lz

OBJ_FLAGS=-c $(CFLAGS) 
EXE_FLAGS=$(CFLAGS) 

UNIT_TEST_OFF=_UNIT_TEST false
UNIT_TEST_ON=_UNIT_TEST true

DEBUG_OFF=_DEBUG false
DEBUG_ON=_DEBUG true

UNITTEST= \
	rm -f $(1).exe; \
	echo -e "\n\e[1;32m=============== START COMPILING ===============\e[0m"; \
	echo -e "Compiling \e[1;36m$(1)\e[0m module Unit Test..."; \
	echo "  Enabling UnitTest by modifying the UNIT TEST LINE."; \
	sed -i.bak "s/${UNIT_TEST_OFF}/${UNIT_TEST_ON}/g" $(1).cpp; \
	echo -e "\e[1;34m=============== COMPILER MESSAGE ===============\e[0m"; \
	$(CC) $(EXE_FLAGS) $(INCLUDE_DIR) $(LIB_DIR) $(1).cpp $(2) $(LIBS) -o "$(1).exe"; \
	sed -i.bak "s/${UNIT_TEST_ON}/${UNIT_TEST_OFF}/g" $(1).cpp; \
	read -p "Press any key to continue... " -n1 -s; \
	echo -e "\n\e[1;34m=============== RUNNING TEST ===============\e[0m"; \
	./$(1).exe; \
	echo -e "\e[1;31m=============== END OF TEST ===============\e[0m\n" 

GMOCK_TEST= \
	echo -e "\n\e[1;32m=============== START COMPILING ===============\e[0m"; \
	echo -e "Compiling \e[1;36m$(1)\e[0m for Gmock Test..."; \
	sed -i.bak "s/${UNIT_TEST_OFF}/${UNIT_TEST_ON}/g" $(1).cpp; \
	sed -i.bak "s/${DEBUG_OFF}/${DEBUG_ON}/g" $(1).hpp; \
	echo -e "\e[1;34m=============== COMPILER MESSAGE ===============\e[0m"; \
	$(CC) $(EXE_FLAGS) $(GMOCK_FLAGS) $(INCLUDE_DIR) $(GMOCK_INCLUDE_DIR) $(LIB_DIR) $(1).cpp $(2) $(LIBS) $(GMOCK_LIB) -o "$(1)-Test-$(DATE)-$(ARCH).exe"; \
	read -p "Press any key to continue... " -n1 -s; \
	echo -e "\n\e[1;34m=============== RUNNING TEST ===============\e[0m"; \
	sed -i.bak "s/${UNIT_TEST_ON}/${UNIT_TEST_OFF}/g" $(1).cpp; \
	sed -i.bak "s/${DEBUG_ON}/${DEBUG_OFF}/g" $(1).hpp; \
	./$(1)-Test-$(DATE)-$(ARCH).exe

COMPILE= \
	rm -f $(1).exe; \
	echo -e "\n\e[1;32m=============== START COMPILING ===============\e[0m"; \
	echo -e "Compiling \e[1;36m$(1)\e[0m..."; \
	echo -e "\e[1;34m=============== COMPILER MESSAGE ===============\e[0m"; \
	sed -i.bak "s/${UNIT_TEST_ON}/${UNIT_TEST_OFF}/g" $(1).cpp; \
	$(CC) $(EXE_FLAGS) $(INCLUDE_DIR) $(LIB_DIR) $(3).cpp $(2) $(LIBS) -o "$(1).exe" ; \
	echo -e "\e[1;33m=============== COMPILER MESSAGE END ===============\e[0m"; \
	echo -e "\nDONE: \e[1;33m$@\e[0m."

AsyncSocket: Socket.o Util.o 
	@$(call UNITTEST,$@,$^)

HttpRequest: Util.o 
	@$(call GMOCK_TEST,$@,$^)

HttpRequestParser: Util.o  StringMap.o 
	@$(call UNITTEST,$@,$^)

HttpResponseBuilder: Util.o 
	@$(call UNITTEST,$@,$^)

Inotify: AsyncIo.o Util.o 
	@$(call UNITTEST,$@,$^)

MemoryPool: Util.o 
	@$(call UNITTEST,$@,$^)
	
Gzip: MemoryPool.o Util.o 
	@$(call UNITTEST,$@,$^)

HttpClient: Socket.o Util.o 
	@$(call UNITTEST,$@,$^)

Http:
	@$(call UNITTEST,$@,$^)

Logger: Util.o 
	@$(call UNITTEST,$@,$^)

MapStorageTest: Util.o 
	@$(call UNITTEST,$@,$^)

SharedMemory: Util.o 
	@$(call UNITTEST,$@,$^)

Util: 
	@$(call UNITTEST,$@,$^)


.cpp.o:
	@echo -e "Compiling \e[1;33m$<\e[0m for dependency..."
	@$(CC) $(OBJ_FLAGS) $(INCLUDE_DIR) $(LIB_DIR) $< $(LIBS) -o $@

.PHONY: clean
clean:
	rm -f *.o *.exe *.bak
	@echo "Object files and Executables are removed!"
