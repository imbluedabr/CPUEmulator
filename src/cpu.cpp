#include "component.h"
#include "cpu.h"

//class IOBus

IOEntry::IOEntry() {
    this->write = NULL;
    this->read = NULL;
    this->val = 0;
}

IOBusHandeler::IOBusHandeler(CPU* parent) : Component(parent) {}

void IOBusHandeler::atachDev(Byte adres, void (*write)(CPU*, Word), Word (*read)(CPU*)) {
    this->IOList[adres].write = write;
    this->IOList[adres].read = read;
}

Word IOBusHandeler::readIO(Byte adres) {
    IOEntry* entry = &this->IOList[adres];
    if (entry->read != NULL) {
        return entry->read(this->parent);
    }
    return entry->val;
}

void IOBusHandeler::writeIO(Byte adres, Word value) {
    IOEntry* entry = &this->IOList[adres];
    if (entry->write != NULL) {
        entry->write(this->parent, value);
    }
    entry->val = value;
}

Word& IOBusHandeler::operator[](Byte adres) {
    return this->IOList[adres].val;
}


//class CPU

CPU::CPU() : memory(this, 1024),
             DMAController(this),
             IOBus(this),
             serialIO(this),
             flash(this, 1024)
{
}

void CPU::setBit(Byte index) {
    this->registers[REG_SR] |= (1 << index);
}

bool CPU::getBit(Byte index) {
    return this->registers[REG_SR] & (1 << index);
}

void CPU::clearBit(Byte index) {
    this->registers[REG_SR] &= ~(1 << index);
}

Word CPU::parseFlags(Word value) {
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
void CPU::interupt(Byte intv) {
    if (getBit(FLAG_MASKINT) && (intv != INT_NMI)) {//dont interupt if the int mask is high and the interupt is not a non maskable interupt
        return;
    }
    setBit(FLAG_INTERUPT);
    clearBit(FLAG_USERMODE);
    for (int i = 0; i < 7; i++) {//push the general purpose registers to the stack to preserve them
        this->memory.write(this->registers[REG_KSP], this->registers[i], 2);
        this->registers[REG_KSP] += 2;
    }
    this->registers[REG_PC] = this->memory.read(intvTable + intv*2, 2); //read the interupt vector from memory
}

//pop n bytes from the stack
Word CPU::pop(Byte n) {
    this->registers[REG_SP + getBit(FLAG_USERMODE)] += n;
    return read(this->registers[REG_SP + getBit(FLAG_USERMODE)], n);
}

//push n bytes to the stack
void CPU::push(Word value, Byte n) {
    write(this->registers[REG_SP + getBit(FLAG_USERMODE)], value, n);//write n bytes to the adres in REG_SP
    this->registers[REG_SP + getBit(FLAG_USERMODE)] -= n;//this is a decrementing stack architecture so decrement REG_SP by n bytes
}

void CPU::incPC(Word i) { //increment the PC with i
    this->registers[REG_PC] += i;
}

Word CPU::getOp(Byte n) { //get n amount of bytes from the adres currently in the PC and then increment the PC with n
    Word val = read(this->registers[REG_PC], n);
    incPC(n);
    return val;
}

