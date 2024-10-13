#include "cpu.h"

//class CPU

CPU::CPU() : memory(this, 1024),
             DMAController(this),
             serialIO(this),
             flash(this, 1024)
{
    this->reset();
}

inline void CPU::setBit(Byte index) {
    this->registers[REG_SR] |= (index << 1);
}

inline bool CPU::getBit(Byte index) {
    return this->registers[REG_SR] & (index << 1);
}

inline void CPU::clearBit(Byte index) {
    this->registers[REG_SR] &= ~(index << 1);
}

inline Word CPU::getReg(Byte reg) {
    if (getBit(FLAG_USERMODE) && reg > REG_SR) {
        return this->registers[reg+1];
    }
    return this->registers[reg];
}

inline void CPU::setReg(Byte reg, Word value) {
    if (getBit(FLAG_USERMODE) && reg > REG_SR) {
        this->registers[reg+1] = value;
    } else {
        this->registers[reg] = value;
    }
}

inline Word CPU::parseFlags(Word value) {
    if (value == 0) {
        //set zero flag
    }
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
    this->userMode = false;
}
