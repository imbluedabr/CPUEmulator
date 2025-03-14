bin := bin
src := src
objects := component.o cpu.o

all: $(bin) $(bin)/system-i8086.exe $(bin)/system-x16.exe

$(bin):
	mkdir -p $(bin)

$(objects): %.o: $(src)/%.cpp
	g++ -c -o $@ $^

$(bin)/system-i8086.exe: $(objects) $(src)/system-i8086.cpp
	g++ -o $@ $^

$(bin)/system-x16.exe: $(objects) $(src)/system-x16.cpp
	g++ -o $@ $^

clean:
	rm -rf $(bin) $(objects)
