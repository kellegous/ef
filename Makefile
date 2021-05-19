CFLAGS := -std=c++17 -Wall $(shell pkg-config cairomm-1.16 --cflags)
LDFLAGS := -Wall $(shell pkg-config cairomm-1.16 --libs)

ALL: efa

%.o: %.cc %.h
	g++ $(CFLAGS) -c -o $@ $<

efa: efa.o point.o
	g++ $(LDFLAGS) -o $@ $^

clean:
	rm -f efa *.o