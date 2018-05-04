SUBNAME = newbase
LIB = smartmet-$(SUBNAME)
SPEC = smartmet-library-$(SUBNAME)
INCDIR = smartmet/$(SUBNAME)

# Installation directories

processor := $(shell uname -p)

ifeq ($(origin PREFIX), undefined)
  PREFIX = /usr
else
  PREFIX = $(PREFIX)
endif

ifeq ($(processor), x86_64)
  libdir = $(PREFIX)/lib64
else
  libdir = $(PREFIX)/lib
endif

bindir = $(PREFIX)/bin
includedir = $(PREFIX)/include
datadir = $(PREFIX)/share
objdir = obj

# Compiler options

DEFINES = -DUNIX -D_REENTRANT -DFMI_COMPRESSION -DBOOST -DBOOST_IOSTREAMS_NO_LIB

ifeq ($(CXX), clang++)

 FLAGS = \
	-std=c++11 -fPIC -MD \
	-Weverything \
	-Wno-c++98-compat \
	-Wno-float-equal \
	-Wno-padded \
	-Wno-missing-prototypes

 INCLUDES = \
	-isystem $(includedir) \
	-isystem $(includedir)/smartmet

else

 FLAGS = -std=c++11 -fPIC -MD -Wall -W -Wno-unused-parameter

 FLAGS_DEBUG = \
	-Wcast-align \
	-Winline \
	-Wno-multichar \
	-Wno-pmf-conversions \
	-Woverloaded-virtual  \
	-Wpointer-arith \
	-Wcast-qual \
	-Wredundant-decls \
	-Wwrite-strings \
	-Wsign-promo \
	-Wno-inline

 FLAGS_RELEASE = -Wuninitialized

 INCLUDES = \
	-I$(includedir) \
	-I$(includedir)/smartmet

endif

# Compile options in detault, debug and profile modes

CFLAGS         = $(DEFINES) $(FLAGS) $(FLAGS_RELEASE) -DNDEBUG -O2 -g
CFLAGS_DEBUG   = $(DEFINES) $(FLAGS) $(FLAGS_DEBUG)   -Werror  -Og -g
CFLAGS_PROFILE = $(DEFINES) $(FLAGS) $(FLAGS_PROFILE) -DNDEBUG -O2 -g -pg

LIBS = -L$(libdir) \
	-lboost_filesystem \
	-lboost_regex \
	-lboost_thread \
	-lgdal

# What to install

LIBFILE = libsmartmet-$(SUBNAME).so
ALIBFILE = libsmartmet-$(SUBNAME).a

# How to install

INSTALL_PROG = install -p -m 775
INSTALL_DATA = install -p -m 664

ARFLAGS = -r

# Compile option overrides

ifneq (,$(findstring debug,$(MAKECMDGOALS)))
  CFLAGS = $(CFLAGS_DEBUG)
endif

ifneq (,$(findstring profile,$(MAKECMDGOALS)))
  CFLAGS = $(CFLAGS_PROFILE)
endif

# Compilation directories

vpath %.cpp $(SUBNAME)
vpath %.h $(SUBNAME)

# The files to be compiled

SRCS = $(wildcard $(SUBNAME)/*.cpp)
HDRS = $(wildcard $(SUBNAME)/*.h)
OBJS = $(patsubst %.cpp, obj/%.o, $(notdir $(SRCS)))

INCLUDES := -Iinclude $(INCLUDES)

.PHONY: test rpm

# The rules

all: objdir $(LIBFILE) $(ALIBFILE)
debug: all
release: all
profile: all

$(LIBFILE): $(OBJS)
	$(CXX) $(CFLAGS) -shared -rdynamic -o $(LIBFILE) $(OBJS) $(LIBS)

$(ALIBFILE): $(OBJS)
	$(AR) $(ARFLAGS) $(ALIBFILE) $(OBJS)

clean:
	rm -f $(LIBFILE) $(ALIBFILE) *~ $(SUBNAME)/*~
	rm -rf $(objdir)

format:
	clang-format -i -style=file $(SUBNAME)/*.h $(SUBNAME)/*.cpp test/*.cpp

install:
	@mkdir -p $(includedir)/$(INCDIR)
	@list='$(HDRS)'; \
	for hdr in $$list; do \
	  HDR=$$(basename $$hdr); \
	  echo $(INSTALL_DATA) $$hdr $(includedir)/$(INCDIR)/$$HDR; \
	  $(INSTALL_DATA) $$hdr $(includedir)/$(INCDIR)/$$HDR; \
	done
	@mkdir -p $(libdir)
	echo $(INSTALL_PROG) $(LIBFILE) $(libdir)/$(LIBFILE)
	$(INSTALL_PROG) $(LIBFILE) $(libdir)/$(LIBFILE)
	echo $(INSTALL_DATA) $(ALIBFILE) $(libdir)/$(ALIBFILE)
	$(INSTALL_DATA) $(ALIBFILE) $(libdir)/$(ALIBFILE)

test:
	+cd test && make test

objdir:
	@mkdir -p $(objdir)

rpm: clean
	if [ -e $(SPEC).spec ]; \
	then \
	  tar -czvf $(SPEC).tar.gz --transform "s,^,$(SPEC)/," * ; \
	  rpmbuild -ta $(SPEC).tar.gz ; \
	  rm -f $(SPEC).tar.gz ; \
	else \
	  echo $(SPEC).spec file missing; \
	fi;

.SUFFIXES: $(SUFFIXES) .cpp

modernize:
	for F in newbase/*.cpp; do echo $$F; clang-tidy $$F -fix -checks=-*,modernize-* -- $(CFLAGS) $(DEFINES) $(INCLUDES); done

obj/%.o: %.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) -c -o $@ $<

ifneq ($(wildcard obj/*.d),)
-include $(wildcard obj/*.d)
endif
