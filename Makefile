bin := bin
src := src
opt := O2

ifeq ($(shell uname -o),Msys)
	ext = .exe
else
	ext = 
endif

all: $(bin) $(bin)/system-test$(ext)



$(bin):
	mkdir -p $(bin)

component.o: $(src)/component.cpp
	g++ -$(opt) -c -o $@ $^

$(bin)/system-i8086$(ext): component.o $(src)/system-i8086.cpp
	g++ -$(opt) -o $@ $^

$(bin)/system-x16$(ext): component.o $(src)/system-x16.cpp
	g++ -$(opt) -o $@ $^

$(bin)/system-test$(ext): component.o $(src)/system-test.cpp
	g++ -$(opt) -o $@ $^

clean:
	rm -rf $(bin) component.o
