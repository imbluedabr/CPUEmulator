bin_folder := bin
src_folder := src
objects    := component.o cpu.o

all: $(bin_folder) $(bin_folder)/main.exe

$(bin_folder):
	mkdir -p $(bin_folder)

$(objects): %.o: $(src_folder)/%.cpp
	g++ -c -o $@ $^

$(bin_folder)/main.exe: $(objects) $(src_folder)/main.cpp
	g++ -o $(bin_folder)/main.exe $^

clean:
	rm -rf $(bin_folder) $(objects)
