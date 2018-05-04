PROG = $(patsubst %.cpp,%,$(wildcard *.cpp))

MAINFLAGS = -Wall -W -Wno-unused-parameter

EXTRAFLAGS = -Werror -pedantic -Wpointer-arith -Wcast-qual \
	-Wcast-align -Wwrite-strings -Wconversion -Winline \
	-Wctor-dtor-privacy -Wnon-virtual-dtor -Wno-pmf-conversions \
	-Wsign-promo -Wchar-subscripts -Wold-style-cast \
	-Wshadow -Woverloaded-virtual

DIFFICULTFLAGS = -Weffc++ -Wunreachable-code -Wredundant-decls

CC = g++
CFLAGS = -DUNIX -O0 -g $(MAINFLAGS)
INCLUDES = -I ../include \
	-I /usr/include/smartmet \
	-I /usr/include/smartmet/newbase \
	-I /usr/include/freetype2
LIBS =  -L .. -lsmartmet_imagine \
	-L /usr/lib/smartmet -lsmartmet_newbase \
	-lboost_iostreams-mt \
	-lboost_regex-mt \
	-lboost_filesystem-mt \
	-lboost_thread-mt \
	-lboost_system-mt \
	-lfreetype -lpng -ljpeg -lz -lbz2 -lrt

all: $(PROG)
clean:
	rm -f $(PROG) *~

$(PROG) : % : %.cpp ../libsmartmet_imagine.a
	$(CC) $(CFLAGS) -o $@ $@.cpp $(INCLUDES) $(LIBS)
