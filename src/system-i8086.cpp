#include "system-i8086.h"


//should contain mmu logic
Word CPU_8086::read(Word virtAdres, Byte n) {//wraper for read operation
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

void CPU_8086::write(Word virtAdres, Word value, Byte n) {//wraper for write operation
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

void CPU_8086::execute() {
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

void CPU_8086::reset() {
    this->memory.reset();//reset ram
    this->memory.loadBios();//load the bios

    for (int i = 0; i < register_count; i++) {//reset the registers
        this->registers[i] = 0;
    }
    this->registers[REG_PC] = 0x00FF; //adres where the cpu starts executing
}

void CPU_8086::status() {
    for (int i = 0; i < register_count; i++) {
        std::cout << "reg" << i << ": " << this->registers[i] << "\n";
    }
    std::cout << "FLAG_USERMODE: " << getBit(FLAG_USERMODE) << "\n";
    std::cout << "FLAG_INTERUPT: " << getBit(FLAG_INTERUPT) << "\n";
}


//constructor
CPU_8086::CPU_8086() : CPU() {
    this->reset();
}

int main(int argc, char** argv) {
    CPU_8086 sys86;
    Byte program[] = {
        255, 0
    };
    sys86.memory.load((Word*) &program, sizeof(program)/2, 128);
    sys86.memory.dump(0, 256);
    while(!sys86.getBit(CPU_8086::FLAG_HALT)) {
        sys86.execute();
        sys86.serialIO.update();
        sys86.DMAController.update();
        sys86.status();
    }
}





