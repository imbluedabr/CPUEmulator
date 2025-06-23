bin := bin
src := src
opt := O2

all: $(bin) $(bin)/system-test.exe



$(bin):
	mkdir -p $(bin)

component.o: $(src)/component.cpp
	g++ -$(opt) -c -o $@ $^

$(bin)/system-i8086.exe: component.o $(src)/system-i8086.cpp
	g++ -$(opt) -o $@ $^

$(bin)/system-x16.exe: component.o $(src)/system-x16.cpp
	g++ -$(opt) -o $@ $^

$(bin)/system-test.exe: component.o $(src)/system-test.cpp
	g++ -$(opt) -o $@ $^

clean:
	rm -rf $(bin) component.o
