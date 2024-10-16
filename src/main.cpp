#include "componentLogic.cpp"

int main(int argc, char **argv) {
    CPU cpu;
    cpu.memory.write(0, 10, 2);
    std::cout << cpu.memory.read(0, 2) << "\n";
    std::cout << "i want to kill my self\n";
    return 0;
}
