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
inline Word CPU::pop(Byte n) {
    this->registers[REG_SP + getBit(FLAG_USERMODE)] += n;
    return read(this->registers[REG_SP + getBit(FLAG_USERMODE)], n);
}

//push n bytes to the stack
inline void CPU::push(Word value, Byte n) {
    write(this->registers[REG_SP + getBit(FLAG_USERMODE)], value, n);//write n bytes to the adres in REG_SP
    this->registers[REG_SP + getBit(FLAG_USERMODE)] -= n;//this is a decrementing stack architecture so decrement REG_SP by n bytes
}

inline void CPU::incPC(Word i) { //increment the PC with i
    this->registers[REG_PC] += i;
}

inline Word CPU::getOp(Byte n) { //get n amount of bytes from the adres currently in the PC and then increment the PC with n
    Word val = read(this->registers[REG_PC], n);
    incPC(n);
    return val;
}

//should contain mmu logic
inline Word CPU::read(Word virtAdres, Byte n) {//wraper for read operation
    if (getBit(FLAG_USERMODE)) {
        Word adres = this->IOBus[MMU_BASEADR] + virtAdres;
        if (adres > this->IOBus[MMU_TOPADR]) {
            interupt(INT_GPF); //cause a segmentation fault
            return 0;
        }
        return this->memory.read(adres, n);
    }
    return this->memory.read(virtAdres, n);
}

inline void CPU::write(Word virtAdres, Word value, Byte n) {//wraper for write operation
    if (getBit(FLAG_INTERUPT)) {
        return;
    }
    if (getBit(FLAG_USERMODE)) {
        Word adres = this->IOBus[MMU_BASEADR] + adres;
        if (adres > this->IOBus[MMU_TOPADR]) {
            interupt(INT_GPF); //cause a segmentation fault
            return;
        }
        this->memory.write(adres, value, n);
    } else {
        this->memory.write(virtAdres, value, n);
    }
}

//main functions

void CPU::execute() {
    Byte opcode = (Byte) getOp(1);
    Byte reg;
    Word adres;
    //instruction set
    //register select is 1 byte
    //opcodes are 1 byte
    //adreses are 2 bytes
    //inmeadiate values are 1 or 2 bytes, depends on the instruction, if it is a "word" or "byte" instruction
    //TODO: add MMU and make setreg and mem write functions fail when FLAG_INTERUPT is high
    std::cout << "opc: " << (Word) opcode << "\n";
    switch(opcode) {
        case 0:
            break;
        case 1: //movw reg[op1] = op2
            setReg(getOp(1), getOp(2));
            break;
	case 2: //movw reg[op1] = reg[op2]
	    setReg(getOp(1), getReg(getOp(1)));
	    break;
	case 3: //movw reg[op1] = mem[op2]
            setReg(
                getOp(1), //get the operand for the register
                read( //read word from memory
                    getOp(2), //get operand for the adres
                    2
                )
            );
	    break;
        case 4: //movw reg[op1] = mem[reg[op2]]
            setReg(
                getOp(1),
                read(
                    getReg(
                        getOp(1)
                    ),
                    2
                )
            );
            break;
        case 5: //movw reg[op1] = mem[reg[op2] + op3] --op3 is an ofset of 1 byte
            setReg(
                getOp(1),
                read(
                    getReg(
                        getOp(1)
                    ) + getOp(1),
                    2
                )
            );
            break;
        case 6: //movw mem[op1] = op2
            write(
                getOp(2),
                getOp(2),
                2
            );
            break;
        case 7: //movw mem[op1] = reg[op2]
            write(
                getOp(2),
                getReg(getOp(1)),
                2
            );
            break;
        case 8: //movw mem[reg[op1]] = reg[op2]
            write(
                getReg(
                    getOp(1)
                ),
                getReg(
                    getOp(1)
                ),
                2
            );
            break;
        case 9: //movw mem[reg[op1] + op2] = reg[op3] --op2 is an ofset of 1 byte
            write(
                getReg(
                     getOp(1)
                ) + getOp(1),
                getReg(
                     getOp(1)
                ),
                2
            );
            break;
        case 10: //inc
            reg = (Byte) getOp(1);
            setReg(reg, getReg(reg) + 1);
            break;
        case 11: //jmp
            setReg(REG_PC, getOp(2));
            break;
        case 12:
            adres = getOp(2);
            if (getBit(FLAG_ZERO)) {
                setReg(REG_PC, adres);
            }
            break;
        case 13:
            adres = getOp(2);
            if (!getBit(FLAG_ZERO)) {
                setReg(REG_PC, adres);
            }
            break;
        case 200: //io[op1] = op2
            this->IOBus.writeIO(getOp(1), getOp(2));
            break;
        case 201: //io[op1] = reg[op2]
            this->IOBus.writeIO(getOp(1), getReg(getOp(1)));
            break;
        case 202: //reg[op1] = io[op2]
            setReg(getOp(1), this->IOBus.readIO(getOp(1)));
            break;
        case 255:
            setBit(FLAG_HALT);
            break;
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
    this->registers[REG_PC] = 0x00FF; //adres where the cpu starts executing
}

void CPU::status() {
    for (int i = 0; i < register_count; i++) {
        std::cout << "reg" << i << ": " << this->registers[i] << "\n";
    }
    std::cout << "FLAG_USERMODE: " << getBit(FLAG_USERMODE) << "\n";
    std::cout << "FLAG_INTERUPT: " << getBit(FLAG_INTERUPT) << "\n";
}
