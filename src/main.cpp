#include "component.h"
#include "cpu.h"

int main(int argc, char **argv) {
    CPU cpu;
    Byte program[] = {
        200, 0x1A, 65, 0, //out 1A, 65 --write 0x41 to port 0x1A, wich is the TX buffer
        13, 255, 0   //jmp 0xFF
    };
    cpu.memory.load((Word*) &program, sizeof(program)/2, 128);
    cpu.memory.dump(0, 256);
    while(!cpu.getBit(CPU::FLAG_HALT)) {//execute until the halt flag gets set
        cpu.execute();
        cpu.serialIO.update();
        cpu.status();
    }
    return 0;
}
