#include "cpu.h"

//class CPU

CPU::CPU() : memory(this, 1024),
             DMAController(this),
             serialIO(this),
             flash(this, 1024)
{
    //idk
}

inline void CPU::setBit(Byte index) {
    
}

inline bool CPU::getBit(Byte index) {
    
}

inline void CPU::clearBit(Byte index) {
    
}

inline Word CPU::getReg(Byte reg) {
    
}

inline void CPU::setReg(Byte reg, Word value) {
    
}

inline Word CPU::parseFlags(Word value) {
    
}

inline void CPU::interupt(Byte intv) {
    
}

inline Word CPU::pop(Byte n) {
    
}

inline void CPU::push(Word value, Byte n) {
    
}

//main functions

void CPU::execute() {
    
}

void CPU::reset() {
    this->memory.reset();//reset ram
    this->memory.loadBios();//load the bios

    for (int i = 0; i < register_count; i++) {//reset the registers
        this->registers[i] = 0;
    }
}
