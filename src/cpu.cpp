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

//cause an interupt
inline void CPU::interupt(Byte intv) {
    setBit(FLAG_INTERUPT);
    clearBit(FLAG_USERMODE);
    push(this->registers[REG_PC], 2);//push the pc to the kernel stack pointer
    this->registers[REG_PC] = this->memory.read(intvTable + intv*2, 2); //read the interupt vector from memory
}

//pop n bytes from the stack
inline Word CPU::pop(Byte n) {
    this->registers[REG_SP + getBit(FLAG_USERMODE)] += n;
    return this->memory.read(this->registers[REG_SP], n);
}

//push n bytes to the stack
inline void CPU::push(Word value, Byte n) {
    this->memory.write(this->registers[REG_SP], value, n);//write n bytes to the adres in REG_SP
    this->registers[REG_SP + getBit(FLAG_USERMODE)] -= n;//this is a decrementing stack architecture so decrement REG_SP by n bytes
}

inline void CPU::incPC(Word i) {
    this->registers[REG_PC] += i;
}

inline Word CPU::getOp(Byte n) { //get operand
    Word val = this->memory.read(this->registers[REG_PC], n);
    incPC(n);
    return val;
}

inline Word CPU::read(Word adres, Byte n) {//wraper for read operation
    return this->memory.read(adres, n);
}

inline void CPU::write(Word adres, Word value, Byte n) {//wraper for write operation
    this->memory.write(adres, value, n);
}

//main functions

void CPU::execute() {
    Byte opcode = (Byte) getOp(1);
    //instruction set
    //register select is 1 byte
    //opcodes are 1 byte
    //adreses are 2 bytes
    //inmeadiate values are 1 or 2 bytes, depends on the instruction, if it is a "word" or "byte" instruction
    //TODO: add MMU and make setreg and mem write functions fail when FLAG_INTERUPT is high
    switch(opcode) {
        case 0: {
            incPC(1);
        }
        case 1: { //movw reg[op1] = op2
            setReg(getOp(1), getOp(2));
        }
	case 2: { //movw reg[op1] = reg[op2]
	    setReg(getOp(1), getOp(2));
	}
	case 3: {//movw reg[op1] = read(op2)
            setReg(
                getOp(1), //get the operand for the register
                read( //read word from memory
                    getOp(2), //get operand for the adres
                    2
                )
            );
	}
        default:
            interupt(INT_INVOPC); //cause invalid opcode interupt
    }
}

void CPU::reset() {
    this->memory.reset();//reset ram
    this->memory.loadBios();//load the bios

    for (int i = 0; i < register_count; i++) {//reset the registers
        this->registers[i] = 0;
    }
}

void CPU::status() {
    for (int i = 0; i < register_count; i++) {
        std::cout << "reg" << i << ": " << this->registers[i] << "\n";
    }
    std::cout << "FLAG_USERMODE: " << getBit(FLAG_USERMODE) << "\n";
    std::cout << "FLAG_INTERUPT: " << getBit(FLAG_INTERUPT) << "\n";
}
