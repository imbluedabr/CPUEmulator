
main_files := main.cpp componentLogic.cpp cpu.cpp cpu.h components.h
bin_folder := bin
main_target := main.cpp
src_folder := src/

all: $(bin_folder) $(bin_folder)/main.exe

$(bin_folder):
	mkdir -p $(bin_folder)

$(bin_folder)/main.exe: $(addprefix $(src_folder),$(main_files))
	g++ -o $(bin_folder)/main.exe $(addprefix $(src_folder),$(main_target))

clean:
	rm -rf $(bin_folder)
