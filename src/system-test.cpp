#include "system-test.h"
#include <chrono>
#include <string.h>

SystemClock::SystemClock() {
    this->running = true;
}

uint32_t SystemClock::read(uint8_t adres) {
    if (adres == 0) {
        return (uint32_t) this->running;
    }
    return 0;
}

void SystemClock::write(uint8_t adres, uint32_t value) {
    if (adres == 0) {
        this->running = (bool) value;
    }
}

SystemTest::SystemTest() :
    ram(this->adresBusMax+1),
    BaseSystem(ram),
    ioBus(),
    clock(),
    serialIODevice(),
    storage("test.bin", 256)
{
    
    this->ioBus.addDevice(0, &this->serialIODevice);
    this->ioBus.addDevice(1, &this->clock);
    this->ioBus.addDevice(2, &this->storage);
}

void SystemTest::interrupt(uint8_t irqVector) {
    //this system doesnt have interupts
}

void SystemTest::setFlags(uint16_t value) {
    if ((int16_t)value == 0) {
        this->flag.zero = true;
        this->flag.positive = false;
        this->flag.negative = false;
    } else if ((int16_t)value > 0) {
        this->flag.positive = true;
        this->flag.zero = false;
        this->flag.negative = false;
    } else {
        this->flag.negative = true;
        this->flag.positive = false;
        this->flag.zero = false;
    }
}

void SystemTest::jump(uint16_t adres) {
    this->reg.PC = adres - 1;
}

void SystemTest::push(uint16_t val) {
    this->ram.write(this->reg.SP*2, val, 2);
    this->reg.SP += 1;
    if (this->reg.SP > SystemTest::adresBusMax) {
        this->reg.SP = 0;
    }
}

uint16_t SystemTest::pop() {
    this->reg.SP -= 1;
    if (this->reg.SP > SystemTest::adresBusMax) {
        this->reg.SP = 0;
    }
    return (uint16_t) this->ram.read(this->reg.SP*2, 2);
}

void SystemTest::reset() {
    this->reg.A = 0;
    this->reg.DP = 0;
    this->reg.SP = 0;
    this->reg.BP = 0;
    this->reg.PC = 0;
    this->clock.running = true;
}

void SystemTest::step() {
    
    uint16_t instruction = this->ram.read(this->reg.PC*2, 2);
    //std::cout << "instr: " << std::hex << instruction << std::endl;
    //value is a 11 bit number
    uint16_t value = (instruction & 0xFFE0) >> 5;

    uint8_t opCode = instruction & 0x001F;
    //std::cout << "opcode: " << (uint16_t) opCode << ", value: " << value << std::endl;
    switch(opCode) {
        case 0:
            this->reg.A = value;
            break;
        case 1:
            this->reg.A = this->ram.read(((this->reg.DP + value) & SystemTest::to11bit)*2, 2);
            break;
        case 2:
            this->reg.A = this->ram.read(((this->reg.BP + value) & SystemTest::to11bit)*2, 2);
            break;
        case 3:
            this->reg.A = this->ram.read(((this->reg.SP + value) & SystemTest::to11bit)*2, 2);
            break;
        case 4:
            this->reg.A = this->ram.read(((this->reg.PC + value) & SystemTest::to11bit)*2, 2);
            break;
        case 5:
            this->ram.write(((this->reg.DP + value) & SystemTest::to11bit)*2, this->reg.A, 2);
            break;
        case 6:
            this->ram.write(((this->reg.BP + value) & SystemTest::to11bit)*2, this->reg.A, 2);
            break;
        case 7:
            this->ram.write(((this->reg.SP + value) & SystemTest::to11bit)*2, this->reg.A, 2);
            break;
        case 8:
            this->ram.write(((this->reg.PC + value) & SystemTest::to11bit)*2, this->reg.A, 2);
            break;
        case 9:
            this->reg.DP = value & SystemTest::to11bit;
            break;
        case 10:
            this->reg.DP = this->ram.read(((this->reg.BP + value) & SystemTest::to11bit)*2, 2) & SystemTest::to11bit;
            break;
        case 11:
            this->reg.DP = this->ram.read(((this->reg.SP + value) & SystemTest::to11bit)*2, 2) & SystemTest::to11bit;
            break;
        case 12:
            this->reg.DP = this->ram.read(((this->reg.PC + value) & SystemTest::to11bit)*2, 2) & SystemTest::to11bit;
            break;
        case 13:
            this->ram.write(((this->reg.BP + value) & SystemTest::to11bit)*2, this->reg.DP, 2);
            break;
        case 14:
            this->ram.write(((this->reg.SP + value) & SystemTest::to11bit)*2, this->reg.DP, 2); 
            break;
        case 15:
            this->ram.write(((this->reg.PC + value) & SystemTest::to11bit)*2, this->reg.DP, 2);
            break;
        case 16:
            this->reg.A = this->ioBus.read((uint8_t) this->ram.read(((this->reg.SP + value) & SystemTest::to11bit)*2, 2));
            break;
        case 17:
            this->ioBus.write((uint8_t) this->ram.read(((this->reg.SP + value) & SystemTest::to11bit)*2, 2), this->reg.A);
            break;
        case 18:
            this->jump(value);
            break;
        case 19:
            if (this->flag.zero) {
                this->jump(value);
            }
            break;
        case 20:
            if (!this->flag.zero) {
                this->jump(value);
            }
            break;
        case 21:
            this->setFlags(this->reg.A - value);
            break;
        case 22:
            this->reg.DP = this->ram.read(((this->reg.DP + value) & SystemTest::to11bit)*2, 2) & SystemTest::to11bit;
            break;
        case 23:
            this->reg.A = this->reg.A + value;
            this->setFlags(this->reg.A);
            break;
        case 24:
            this->reg.A = this->reg.A - value;
            this->setFlags(this->reg.A);
            break;
        case 25:
            this->push(this->reg.PC + 1);
            this->jump(value);
            break;
        case 26:
            this->push(value);
            break;
        case 27:
            if (this->flag.zero) {
                this->jump((this->reg.PC + value) & SystemTest::to11bit);
            }
            break;
        case 28:
            if (!this->flag.zero) {
                this->jump((this->reg.PC + value) & SystemTest::to11bit);
            }
            break;
        case 29:
            this->jump((this->reg.PC + value) & SystemTest::to11bit);
            break;
        case 30:
            this->push(this->reg.PC + 1);
            this->jump((this->reg.PC + value) & SystemTest::to11bit);
            break;
        case 31:
            opCode = (instruction & 0x03E0) >> 5;
            switch(opCode) {
                case 0:
                    this->reg.A = this->reg.DP;
                    break;
                case 1:
                    this->reg.A = this->reg.BP;
                    break;
                case 2:
                    this->reg.A = this->reg.SP;
                    break;
                case 3:
                    this->reg.A = this->reg.PC;
                    break;
                case 4:
                    this->reg.A = this->reg.A + this->ram.read(this->reg.DP*2, 2);
                    this->setFlags(this->reg.A);
                    break;
                case 5:
                    this->reg.A = this->reg.A - this->ram.read(this->reg.DP*2, 2);
                    this->setFlags(this->reg.A);
                    break;
                case 6:
                    this->reg.A = this->reg.A + this->reg.DP;
                    this->setFlags(this->reg.A);
                    break;
                case 7:
                    this->reg.A = this->reg.A - this->reg.DP;
                    this->setFlags(this->reg.A);
                    break;
                case 8:
                    this->reg.DP = this->reg.A & SystemTest::to11bit;
                    break;
                case 9:
                    this->reg.DP = this->reg.BP;
                    break;
                case 10:
                    this->reg.DP = this->reg.SP;
                    break;
                case 11:
                    this->reg.A = this->reg.A >> this->reg.DP;
                    break;
                case 12:
                    this->reg.A = this->reg.A & this->reg.DP;
                    break;
                case 13:
                    this->reg.BP = this->reg.A & SystemTest::to11bit;
                    break;
                case 14:
                    this->reg.BP = this->reg.DP;
                    break;
                case 15:
                    this->reg.BP = this->reg.SP;
                    break;
                case 16:
                    this->reg.A = this->reg.A << this->reg.DP;
                    break;
                case 17:
                    this->reg.SP = this->reg.A & SystemTest::to11bit;
                    break;
                case 18:
                    this->reg.SP = this->reg.DP;
                    break;
                case 19:
                    this->reg.SP = this->reg.BP;
                    break;
                case 20:
                    this->setFlags(this->reg.A - this->reg.DP);
                    break;
                case 21:
                    this->jump(this->reg.A & SystemTest::to11bit);
                    break;
                case 22:
                    if (this->flag.zero) {
                        this->jump(this->reg.A & SystemTest::to11bit);
                    }
                    break;
                case 23:
                    if (!this->flag.zero) {
                        this->jump(this->reg.A & SystemTest::to11bit);
                    }
                    break;
                case 24:
                    if (this->flag.positive) {
                        this->jump(this->reg.A & SystemTest::to11bit);
                    }
                    break;
                case 25:
                    if (this->flag.negative) {
                        this->jump(this->reg.A & SystemTest::to11bit);
                    }
                    break;
                case 26:
                    this->push(this->reg.A);
                    break;
                case 27:
                    this->push(this->reg.DP);
                    break;
                case 28:
                    this->reg.A = this->pop();
                    break;
                case 29:
                    this->reg.DP = this->pop() & SystemTest::to11bit;
                    break;
                case 30:
                    this->push(this->reg.PC + 1);
                    this->jump(this->reg.A & SystemTest::to11bit);
                    break;
                case 31:
                    this->jump(this->pop() & SystemTest::to11bit);
                    break;
            }
    }
    uint16_t pc = this->reg.PC + 1;
    if (pc > this->adresBusMax) {
        pc = 0;
    }
    this->reg.PC = pc;
}

void SystemTest::status() {
    std::cout << "Acc: " << this->reg.A << std::endl;
    std::cout << "MAR: " << this->reg.DP << std::endl;
    std::cout << "PC: " << this->reg.PC << std::endl;
    std::cout << "SP: " << this->reg.SP << std::endl;
    std::cout << "BP: " << this->reg.BP << std::endl << std::endl;
}

void SystemTest::run() {
    
    using namespace std::chrono;

    // Start time before the loop
    auto start = high_resolution_clock::now();

    int loopcount = 0;
    this->reset();
    char c;

    while(this->clock.running) {
        this->step();
        this->serialIODevice.update();
        loopcount++;
    }
    // End time after the loop
    auto end = high_resolution_clock::now();

    // Calculate the duration
    auto duration = duration_cast<nanoseconds>(end - start);
    std::cout << "Time for " << loopcount << " iterations: " << duration.count() << " ns\n";
    std::cout << "Average per iteration: " << duration.count() / loopcount << " ns\n";
}

void SystemTest::runDebug() {
    this->reset();
    char c;
    bool stepmode = true;
    uint16_t breakpoint = 0;

    while(this->clock.running) {
        this->step();
        this->serialIODevice.update();
        
        if (this->reg.PC == breakpoint) {
            stepmode = true;
            std::cout << "\nreached breakpoint!\n";
        }

        if (stepmode) {
            this->status();
            std::cout << "debug> ";
            std::cin >> c;

            if (c == 'h') { //halt
                this->clock.running = false;
            } else if (c == 'e') { //examine
                int start;
                int end;
                std::cout << "start adres> ";
                std::cin >> start;
                std::cout << "end adres> ";
                std::cin >> end;
                for (; start < end; start++) {
                    std::cout << (uint16_t) this->ram.read(start*2, 2) << ", ";
                }
                std::cout << "\n";
            } else if (c == 'b') { //breakpoint
                std::cout << "breakpoint> ";
                std::cin >> breakpoint;
            } else if (c == 'r') { //run
                stepmode = false;
            } else if (c == 'n') { //next
                //empty
            }
        }
    }
}



int main(int argc, char** argv) {

    SystemTest sys;

    //sys.ram.load((uint32_t*) &program, sizeof(program)/(sizeof(uint32_t)), 0);
    sys.ram.loadFile("../ScrapOs/kernel.bin", 2048/sizeof(uint32_t), 0);
    
    const char* arg = "debug";

    if (argc > 1) {
        if (strcmp(argv[1], arg) == 0) {
            sys.runDebug();
        } else {
            std::cout << "error: invalid argument\n";
        }
    } else {
        sys.run();
    }

    sys.ram.dump(0, 100);

    return 0;
}




