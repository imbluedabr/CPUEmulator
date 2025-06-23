#include "system-i8086.h"

//main functions

void System8086::System8086() : BaseSystem(ram), ioBus()
{
    
}

void System8086::pushW(uint16_t val) {
    this->ram.write(this->reg.SS*16 + this->reg.SP, val, 2);
    this->reg.SP -= 2;
}

void System8086::pushB(uint8_t val) {
    this->reg.SP -= 1;
    this->ram.write(this->reg.SS*16 + this->reg.SP, val, 1);
}

uint16_t System8086::popW() {
    this->reg.SP += 2;
    return this->ram.read(this->reg.SS*16 + this->reg.SP, 2);
}

uint8_t System8086::popB() {
    this->reg.SP += 2;
    return this->ram.read(this->reg.SS*16 + this->reg.SP, 2);
}

void System8086::interrupt(uint8_t irqVector) {
    this->pushW(this->reg.FLAGS);
    this->reg.FLAGS.reg;
    this->pushW(this->)
}

void System8086::reset() {
    this->reg.PC = 0;
    this->reg.CS = 0xFFFF;
    this->reg.STATUS = 0;
}

void System8086::step() {
    
}

void System8086::status() {

}

void System8086::run() {

}

void System8086::runDebug() {

}

