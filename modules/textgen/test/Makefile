PROG = $(patsubst %.cpp,%,$(wildcard *.cpp))
PHPPROG = $(wildcard *.php)
# OBS = $(wildcard ../obj/*.o)

MAINFLAGS = -Wall -W -Wno-unused-parameter

EXTRAFLAGS = -Werror -pedantic -Wpointer-arith -Wcast-qual \
	-Wcast-align -Wwrite-strings -Wconversion -Winline \
	-Wctor-dtor-privacy -Wnon-virtual-dtor -Wno-pmf-conversions \
	-Wsign-promo -Wchar-subscripts -Wold-style-cast \
	-Wshadow -Woverloaded-virtual

DIFFICULTFLAGS = -Weffc++ -Wunreachable-code -Wredundant-decls

processor := $(shell uname -p)
UNAME_S := $(shell uname -s)

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

CC = g++
CFLAGS = -DUNIX -O0 -g $(MAINFLAGS)

INCLUDES = \
	-I /usr/include/smartmet \
	-I /usr/include/mysql
LIBS = \
	../libsmartmet-textgen.so \
	-lsmartmet-calculator \
	-lsmartmet-newbase \
	-L $(libdir)/mysql -lmysqlclient -lmysqlpp

INCLUDES += -I ../textgen

LIBS += \
	-lsmartmet-macgyver \
	-lboost_date_time \
        -lboost_regex\
        -lboost_filesystem\
        -lboost_iostreams\
	-lboost_locale \
	-lboost_thread \
        -lboost_system\
	`pkg-config --cflags --libs icu-i18n` \
        -lz -lbz2 -lpthread -lrt

all: $(PROG)
clean:
	rm -f $(PROG) *~

test: $(PROG)
	@echo Running tests:
	@for prog in $(PROG); do \
	 ./$$prog; \
	done

$(PROG) : % : %.cpp ../libsmartmet-textgen.so
	$(CC) $(CFLAGS) -o $@ $@.cpp $(INCLUDES) $(LIBS)

