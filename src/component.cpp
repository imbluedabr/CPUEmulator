#include "component.h"

//contains all the logic for the components

//ik bad code but i just needed a way to allocate memory without the bloat of vector
template <typename T> DynamicArray<T>::DynamicArray(size_t size) {
    if (size == 0) {
        std::cerr << "bro what the fuck you cant allocate 0 bytes in DynamicArray\n";
        return;
    }
    this->array = new T[size]();
    this->size = size;
    this->lastElement = this->array + this->size - 1;//pointer to the last element
}

template <typename T> T& DynamicArray<T>::operator[](size_t index) {
    static T justkillmealready;
    if (index < this->size) {
        return this->array[index];
    }
    return justkillmealready;
}

template <typename T> DynamicArray<T>::~DynamicArray() {
    delete[] this->array;
    this->size = 0;
}

//Memory

//constructor
Memory::Memory(uint32_t size) : ram(size)
{

}

void Memory::reset() {
    for (uint32_t i = 0; i < this->ram.size; i++) {
        this->ram[i] = 0;
    }
}

void Memory::load(uint32_t* data, uint32_t size, uint32_t adres) {
    for (uint32_t i = 0; i < size; i++) {
        this->ram[adres+i] = data[i];
    }
}

void Memory::loadFile(const char* filen, size_t programSize, uint32_t adres) {
    uint32_t program[programSize];//buffer for the bios
    std::ifstream input(filen, std::ios::binary);
    if (!input) {//check if the file is opened
        std::cerr << "Error: Could not open \"" << filen << std::endl;
        return;
    }
    input.unsetf(std::ios::skipws);//read bytes into the ram memory
    input.read(reinterpret_cast<char*>(program), programSize*sizeof(uint32_t));
    load(program, programSize, adres);//load the program into ram
}

void Memory::dump(uint32_t start, uint32_t end) {
    for (; start < end; start++) {
        std::cout << std::hex << this->ram[start] << ", ";
    }
}
//max n is sizeof(uint32_t) since otherwise you would read and write data out of bounds
//read bytes from ram
uint32_t Memory::read(uint32_t adres, uint8_t n) {
    //holds the return value
    uint32_t value = 0;
    //the start of the bytes being read
    uint8_t* wordStart = (uint8_t*) (((size_t) this->ram.array) + adres);
    if (wordStart + n > (uint8_t*) this->ram.lastElement) {
        return 0;
    }
    memcpy(&value, wordStart, n);
    return value;
}

//write bytes to ram
void Memory::write(uint32_t adres, uint32_t value, uint8_t n) {
    uint8_t* wordStart = (uint8_t*) (((size_t) this->ram.array) + adres);
    if (wordStart + n > (uint8_t*) this->ram.lastElement) {
        return;
    }
    memcpy(wordStart, &value, n);
}


BaseSystem::BaseSystem(Memory& mem) : _Memory(mem) {

}


//class DMAController
DMAController::DMAController(BaseSystem& baseSystem) : _BaseSystem(baseSystem), DMAChannelArray() {

}

//request DMA
bool DMAController::DMAOperation(int channel, bool RW, DynamicArray<uint8_t>* dest, uint32_t size, uint32_t destAdres, uint32_t sourceAdres, uint8_t irqVec) {
    DMAChannel* currentChannel = &DMAChannelArray[channel];
    if (currentChannel->size == 0) {
        currentChannel->source = dest;
        currentChannel->size = size;
        currentChannel->sourceAdres = destAdres;
        currentChannel->destAdres = sourceAdres;
        currentChannel->RW = RW;
        currentChannel->irqVec = irqVec;
        return true;
    } else {
        return false; //dma request got denied since the channel was busy
    }
}

//handles the DMA requests
void DMAController::update() {
    for (int channel = 0; channel < channels; channel++) {//this fr makes my brain hurt
        DMAChannel* currentChannel = &DMAChannelArray[channel];
        if (currentChannel->size > 0) {
            if (currentChannel->RW) {//write
                this->_BaseSystem._Memory.write(currentChannel->destAdres, (*currentChannel->source)[currentChannel->sourceAdres], sizeof(uint8_t));
                currentChannel->sourceAdres++; //increment and decrement all the pointers
                currentChannel->destAdres++;
                currentChannel->size--;
            } else {//read
                (*currentChannel->source)[currentChannel->sourceAdres] = this->_BaseSystem._Memory.read(currentChannel->destAdres, sizeof(uint8_t));
                currentChannel->sourceAdres++;//increment stuff
                currentChannel->destAdres++;
                currentChannel->size--;
            }
            if (currentChannel->size == 0) {//dma finished now we should trigger a hardware interupt
                this->_BaseSystem.interrupt(currentChannel->irqVec);
            }
        }
    }
}

//IOBus
//constructor
IOBusController::IOBusController() : devices() {
    
}

//add a device to the io bus at a certain index
void IOBusController::addDevice(uint8_t index, IODevice* device) {
    this->devices[index] = device;
}

//read from the iobus(called by the cpu)
uint32_t IOBusController::read(uint8_t adres) {
    //select the most significant nible as the device index
    IODevice* dev = this->devices[(adres & 0b11110000) >> 4];
    if (dev != NULL) {
        //select the least significant nible for the adres passed on
        //since we dont want the device code to change when its io index changes
        return dev->read(adres & 0b1111);
    }
    //return 0 if there is no device at that adres
    return 0;
}

//write to the iobus(also called by the cpu)
void IOBusController::write(uint8_t adres, uint32_t value) {
    //select the most significant nible as the device index
    IODevice* dev = this->devices[(adres & 0b11110000) >> 4];
    if (dev != NULL) {
        //select the least significant nible for the adres passed on
        dev->write(adres & 0b1111, value);
    }
}




//class FlashDevice
FlashDevice::FlashDevice(DMAController& dmacontroller, const char* name, uint32_t size) : flash(size), _DMAController(dmacontroller)
{
    loadFlash(name);
}

//load the Flash.bin file into the flash memory
void FlashDevice::loadFlash(const char* name) {
    this->imageName = (char*) name;
    std::ifstream file(name, std::ios::binary);
    if (!file) {
        std::cerr << "Error: could not open \"" << name << "\"\n";
        return;
    }
    file.unsetf(std::ios::skipws);
    file.read((char*)this->flash.array, this->flash.size);
}

//store the flash memory into the Flash.bin file
void FlashDevice::storeFlash() {
    std::ofstream file(this->imageName, std::ios::binary);
    if(!file) {
        std::cerr << "Error: could not open \"" << this->imageName << "\"\n";
        return;
    }
    file.unsetf(std::ios::skipws);
    file.write((char*)this->flash.array, this->flash.size);
}

void FlashDevice::dump(uint32_t start, uint32_t end) {
    for (uint32_t i = start; i < end; i++) {
        std::cout << std::hex << this->flash[i] << ", ";
    }
}

uint32_t FlashDevice::read(uint8_t adres) {
    /* example of how PIO could be done this way
     * if (adres == IO_DATA) {
     *     return this->flash[this->Registers[IO_ADRES]];
     * }
     * */
    return this->Registers[adres];
}

void FlashDevice::write(uint8_t adres, uint32_t value) {
    this->Registers[adres] = value;
}


//TODO: add io functionality
//this will request dma when the right io registers get the right values
void FlashDevice::update() {
    if ((this->Registers[IO_CR] & 0x2) == 0x2) {//check if the dma request bit is set in the control register
        bool succes = _DMAController.DMAOperation(
            this->DMAChannel, //dma channel of the flash device
            this->Registers[IO_CR] & 0x1, //check bit 0 of the control register
            &this->flash,
            this->Registers[IO_BLOCKS] * 512,
            this->Registers[IO_RAMADR],
            this->Registers[IO_DISKADR] * 512,
            this->DMAInterrupt
        );
        if (succes) {
            this->Registers[IO_CR] &= !(0x2); //clear dma request bit
        }
    }
}

BasicStorageDevice::BasicStorageDevice(const char* filename, uint32_t size) : storage(size) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "error: could not open \"" << filename << "\"\n";
        return;
    }
    file.unsetf(std::ios::skipws);
    file.read((char*) this->storage.array, this->storage.size*sizeof(uint16_t));
    this->adres = 0;
}

uint32_t BasicStorageDevice::read(uint8_t adres) {
    if (adres == IO_ADRES) {
        return (uint32_t) this->adres;
    } else if (adres == IO_DATA) {
        return (uint32_t) this->storage[this->adres];
    }
    return 0;
}

void BasicStorageDevice::write(uint8_t adres, uint32_t value) {
    if (adres == IO_ADRES) {
        this->adres = (uint16_t) value;
    } else if (adres == IO_DATA) {
        this->storage[this->adres] = (uint16_t) value;
    }
}

void BasicStorageDevice::update() {

}


//class SerialIODevice
SerialIODevice::SerialIODevice() : Registers() {

#ifdef __linux__
    struct termios buff;
    tcgetattr(1, &buff);

    buff.c_lflag &= ICANON;

    tcsetattr(1, TCSANOW, &buff);
#endif

    this->TXSize = 0;
    this->RXSize = 0;
}

uint32_t SerialIODevice::read(uint8_t adres) { //if the cpu reads from RX, read it from RXBuffer
    if (adres == IO_DATA && ((this->Registers[IO_CTRL] & 0x4) == 0x4)) {
        if (this->RXSize > 0) {
            return this->RXBuffer[--this->RXSize];
        }
    } else {
        return this->Registers[adres];
    }
    return 0;
}

void SerialIODevice::write(uint8_t adres, uint32_t value) { //if the cpu writes to TX add it to TXBuffer
    if ((adres == IO_DATA) && ((this->Registers[IO_CTRL] & 0x1) == 0x1)) {
        if (this->TXSize < 16) {
            this->TXBuffer[this->TXSize++] = (char) value;
        }
    } else {
        this->Registers[adres] = value;
    }
}

void SerialIODevice::update() {
    //handle serial io stuff
    if ((this->TXSize > 0) && ((this->Registers[IO_CTRL] & 0x1) == 0x1)) {
        std::cout << this->TXBuffer[--this->TXSize];
    }

#ifdef __linux__
    char c;
    if ((::read(1, &c, 1) == 1) && (this->RXSize < 16) && (this->Registers[IO_CTRL] & 0x4)) {
        this->RXBuffer[this->RXSize++] = c;
    }
#else
    if (kbhit() && (this->RXSize < 16) && ((this->Registers[IO_CTRL] & 0x4) == 0x4)) {
        this->RXBuffer[this->RXSize++] = getch();
    }
#endif
}

SerialIODevice::~SerialIODevice() {
#ifdef __linux__
    struct termios buff;
    tcgetattr(1, &buff);
    buff.c_lflag |= ICANON;
    buff.c_lflag |= ECHO;
    tcsetattr(1, TCSANOW, &buff);
#endif
}


template class DynamicArray<uint16_t>;
template class DynamicArray<uint32_t>;
template class DynamicArray<uint8_t>;
