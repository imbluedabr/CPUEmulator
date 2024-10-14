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
    if (getBit(FLAG_INTERUPT)) {
        return;
    }
    if (getBit(FLAG_USERMODE) && reg > REG_SR) {
        this->registers[reg+1] = value;
    } else {
        this->registers[reg] = value;
    }
}

inline Word CPU::parseFlags(Word value) {
    if (value == 0) {
        //set zero flag
        setBit(FLAG_ZERO);
    } else if ((int)value < 0) {
        //set negative flag
        setBit(FLAG_NEG);
    } else if (value == 0xFFFF) {
        setBit(FLAG_CARRY);
    }
    return value;
}

inline void CPU::interupt(Byte intv) {
    setBit(FLAG_INTERUPT);
    clearBit(FLAG_USERMODE);
    push(this->regs[REG_PC], 2);//push the pc to the kernel stack pointer
    this->regs[REG_PC] = *(Word*)this->memory.read(intv, 2);
}

inline Word CPU::pop(Byte n) {
    
}

inline void CPU::push(Word value, Byte n) {
    //this->memory.read(getReg(REG_SP)); this shit is hella broken
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
