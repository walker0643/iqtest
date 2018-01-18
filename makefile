PP= g++
LD= g++
CPPFLAGS+= -municode -pipe -Wall -pedantic
LDFLAGS+= -municode -Wl,--subsystem,windows -static

OBJS= iqtest.o
EXES_DEBUG= iqtest-debug.exe
EXES_RELEASE= iqtest.exe

all: debug

debug: CPPFLAGS+= -O0 -ggdb
#debug: LDFLAGS+=
debug: $(EXES_DEBUG)

release: CPPFLAGS+= -Os
release: LDFLAGS+= -s
release: $(EXES_RELEASE)

$(EXES_DEBUG): $(OBJS)
	g++ $(LDFLAGS) -o $@ $^
	
$(EXES_RELEASE): $(OBJS)
	g++ $(LDFLAGS) -o $@ $^

.PHONY:clean
clean:
	rm -f *.o
