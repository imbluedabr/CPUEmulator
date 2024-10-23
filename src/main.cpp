#include "componentLogic.cpp"

int main(int argc, char **argv) {
    CPU cpu;
    cpu.memory.write(0, 1, 1);
    cpu.memory.write(1, 5, 1);
    std::cout << cpu.memory.read(0, 2) << "\n";
    cpu.execute();
    cpu.status();
    return 0;
}
