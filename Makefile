CC = g++
CXX = g++
LIBS = x11 libpulse-simple cairo
CXXFLAGS = -Wall `pkg-config --cflags $(LIBS)`
LDFLAGS = `pkg-config --libs $(LIBS)`

app: main.o
	$(CXX) $(LDFLAGS) main.o -o app
	rm *.o
