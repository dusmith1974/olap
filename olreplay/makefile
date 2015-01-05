CC = distcc
CXX = distcc g++
CXXFLAGS = -DBOOST_ALL_DYN_LINK
CXXFLAGS += -Wall -Wextra -ansi -pedantic -Weffc++ -Wshadow -Werror
CXXFLAGS += -Wno-error=effc++ # for boost and other libs.
CXXFLAGS += -std=c++11

READERS = readers
TIMING = timing
REPLAY = replay
BASE = .
PRJS := $(BASE) $(REPLAY) $(TIMING) $(READERS)

LIB_OSOA_BASE = libosoa-mt
LIB_OLCORE_BASE = libolcore-mt
ifeq ($(MAKECMDGOALS), debug)
LIB_OSOA_SUFFIX = -d.a
CONFIG=debug
else
LIB_OSOA_SUFFIX = .a
CONFIG=release
endif
OSOA_DIR=../../osoa
OLCORE_DIR=../olcore
LIB_OSOA_DIR=$(OSOA_DIR)/lib/$(CONFIG)
LIB_OSOA=$(LIB_OSOA_DIR)/$(LIB_OSOA_BASE)$(LIB_OSOA_SUFFIX)
LIB_OLCORE_DIR=$(OLCORE_DIR)/lib/$(CONFIG)
LIB_OLCORE=$(LIB_OLCORE_DIR)/$(LIB_OLCORE_BASE)$(LIB_OSOA_SUFFIX)

OBJ_BASE=lib
OBJ_DIR=$(OBJ_BASE)/$(CONFIG)
BIN_DIR=bin

SRCS := $(foreach prj, $(PRJS), $(wildcard $(prj)/*.cc))
OBJS := $(foreach prj, $(PRJS), $(addprefix $(OBJ_DIR)/, $(patsubst %.cc, %.o, $(subst $(prj)/,,$(wildcard $(prj)/*.cc)))))

VPATH := $(addsuffix :, $(PRJS))
INC := $(addprefix -I, $(PRJS))
INC += -I$(BOOST_DIR)
INC += -I$(OSOA_DIR)
INC += -I$(OLCORE_DIR)
INC += -I..
INC += -I../..

$(OBJ_DIR)/%.o : %.cc 
	$(COMPILE.cc) $(INC) $(OUTPUT_OPTION) $<

all: $(BIN_DIR)/olreplay

debug: CXXFLAGS += -DDEBUG -g -O0
debug: $(BIN_DIR)/olreplay

ifneq ($(MAKECMDGOALS), debug)
CXXFLAGS += -O3 -DNDEBUG
endif

ifeq ($(MAKECMDGOALS),clean)
DEPS=
else
DEPS=$(OBJS:.o=.d)
$(OBJ_DIR)/%.d : %.cc
	$(CXX) $(CXXFLAGS) -MM $(INC) $< |sed -e '1 s/^/obj\//' > $@ 
-include $(DEPS)
endif

$(BIN_DIR)/olreplay: $(OBJS)
	$(MAKE) -C $(OSOA_DIR) $(MAKECMDGOALS)
	$(MAKE) -C $(OLCORE_DIR) $(MAKECMDGOALS)
	$(LINK.cc) $(OBJS) $(LIB_OLCORE) $(LIB_OSOA) -dynamic -lPocoUtild -lPocoNetd -lPocoXMLd -lPocoFoundationd -pthread -lboost_program_options -lboost_log_setup -lboost_log -lboost_system -lboost_thread -lboost_date_time -lboost_filesystem -lboost_regex -lboost_chrono $(OUTPUT_OPTION)
	ctags -R --c-kinds=+cdefglmnpstuvx --extra=+f
	cscope -Rb

$(OBJS) $(DEPS) : | $(OBJ_DIR) $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

.PHONY: clean
clean :
	$(MAKE) -C $(OSOA_DIR) clean
	$(MAKE) -C $(OLCORE_DIR) clean
	rm -rf $(OBJ_BASE) $(BIN_DIR) *.o *.d
