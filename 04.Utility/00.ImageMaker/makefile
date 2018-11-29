
CPPSOURCEFILES = $(wildcard ./*.cpp)
CPPOBJECTFILES = $(patsubst %.cpp,%.o,$(CPPSOURCEFILES))

all: ImageMaker

ImageMaker: $(CPPOBJECTFILES)
	g++ -o $@ $^

%.o: %.cpp
	g++ -c $<

clean:
	rm -f *.o
	rm -f ImageMaker
