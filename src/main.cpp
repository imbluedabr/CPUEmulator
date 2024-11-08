#include "componentLogic.cpp"

int main(int argc, char **argv) {
    CPU cpu;
    Word program[] = {
        1, 0, 1, //movw A, 1
        1, 1, 1, //movw B, 1
        255      //hlt
    };
    cpu.memory.load((Word*)&program, sizeof(program), 0x00FF);
    while(!cpu.getBit(CPU::FLAG_HALT)) {//execute until the halt flag gets set
        cpu.execute();
    }
    cpu.status();
    return 0;
}
