#include "system-x16.h"

void CPU_X16::write(Word adres, Word value, Byte n) {
    this->memory.write(adres, value, n);
}

Word CPU_X16::read(Word adres, Byte n) {
    return this->memory.read(adres, n);
}

void CPU_X16::execute() {
}

void CPU_X16::reset() {
}

void CPU_X16::status() {
    for (int i = 0; i < register_count; i++) {
        std::cout << "reg" << i << ": " << this->registers[i] << "\n";
    }
    std::cout << "FLAG_INTERUPT: " << getBit(FLAG_INTERUPT) << "\n"; 
}


int main(int argc, char** argv) {
    CPU_X16 system;
}

