CXXFLAGS = -DBOOST_ALL_DYN_LINK
CXXFLAGS += -Wall -Wextra -ansi -pedantic -Weffc++ -Wshadow -Werror
CXXFLAGS += -Wno-error=effc++ # for boost and other libs.
CXXFLAGS += -std=c++11

MESSAGES = messages
TIME = time
UTIL = util
BASE = .

PRJS := $(MESSAGES) $(TIME) $(UTIL) $(BASE)

LIB_OLCORE_BASE = libolcore-mt
ifeq ($(MAKECMDGOALS), debug)
LIB_OLCORE_SUFFIX = -d.a
CONFIG=debug
else
LIB_OLCORE_SUFFIX = .a
CONFIG=release
endif
LIB_OLCORE=$(LIB_OLCORE_BASE)$(LIB_OLCORE_SUFFIX)

OBJ_BASE=lib
OBJ_DIR = $(OBJ_BASE)/$(CONFIG)
BIN_DIR = bin

SRCS := $(foreach prj, $(PRJS), $(wildcard $(prj)/*.cc))
OBJS := $(foreach prj, $(PRJS), $(addprefix $(OBJ_DIR)/, $(patsubst %.cc, %.o, $(subst $(prj)/,,$(wildcard $(prj)/*.cc)))))

VPATH := $(addsuffix :, $(PRJS))
INC := $(addprefix -I, $(PRJS))
INC += -I$(BOOST_DIR)

$(OBJ_DIR)/%.o : %.cc 
	$(COMPILE.cc) $(INC) $(OUTPUT_OPTION) $<

all: $(OBJ_DIR)/$(LIB_OLCORE)

debug: CXXFLAGS += -DDEBUG -g -O0
debug: $(OBJ_DIR)/$(LIB_OLCORE)

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

$(OBJ_DIR)/$(LIB_OLCORE): $(OBJS)
	mkdir -p $(OBJ_DIR)
	ar cr $@ $(OBJS)
	#ctags -R --c-kinds=+cdefglmnpstuvx --extra=+f
	#cscope -Rb

$(OBJS) $(DEPS) : | $(OBJ_DIR) $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

.PHONY: clean
clean :
	rm -f numbers output.log
	rm -rf $(OBJ_BASE) $(BIN_DIR) *.o *.d
