#include "cpu.cpp"

//contains all the logic for the components

//ik bad code but i just needed a way to allocate memory without the bloat of vector
template <typename T> DynamicArray<T>::DynamicArray(size_t size) {
    if (size == 0) {
        std::cerr << "bro what the fuck you cant allocate 0 bytes in DynamicArray\n";
        return;
    }
    this->array = new T[size];
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

//might extend this in the future when i want to use polymorhpism for more flexibility
Component::Component(CPU* parent) { this->parent = parent; }

//RamMemory

template <typename T, typename ADR> RamMemory<T, ADR>::RamMemory(CPU* parent, ADR size) :
							ram(size),
							Component(parent)
{

}

template <typename T, typename ADR> void RamMemory<T, ADR>::reset() {
    for (ADR i = 0; i < this->ram.size; i++) {
        this->ram[i] = 0;
    }
}

template <typename T, typename ADR> void RamMemory<T, ADR>::load(T* data, ADR size, ADR adres) {
    for (ADR i = 0; i < adres; i++) {
        this->ram[i] = data[i];
    }
}

template <typename T, typename ADR> void RamMemory<T, ADR>::loadBios() {
    constexpr size_t buffersize = wordSize/512;
    T bios[buffersize];//buffer for the bios
    std::ifstream input("Bios.bin", std::ios::binary);
    if (!input) {//check if the file is opened
        std::cerr << "Error: Could not open \"Bios.bin\"\n";
    }
    input.unsetf(std::ios::skipws);//read 512 bytes into the ram memory
    input.read(reinterpret_cast<char*>(bios), 512);//idk what  this stuff means lol
    load(bios, buffersize, 0x00FF);//load the bios into ram at adres 0x00FF where the reset vector points
}

template <typename T, typename ADR> void RamMemory<T, ADR>::dump(ADR start, ADR end) {
    for (ADR i = start; i < end; i++) {
        std::cout << std::hex << this->ram[i] << ", \n";
    }
}

//read bytes from ram
template <typename T, typename ADR> T RamMemory<T, ADR>::read(ADR adres, unsigned char n) {
    char byteSelect = adres & 0b1;
    T justenditalready = 0;//very bad code since we first copy data into this var and then we make another copy since we return it
    unsigned char* datapointer = (unsigned char*) &this->ram[adres >> 1];//add the pointer to the cacheline to the byteselect to calculate the final adres
    if (&datapointer[byteSelect] + n > (unsigned char*)this->ram.lastElement) {
        return justenditalready; //if the last byte that we read is above the last element we got to return
    }
    //ugh i hate this man why cant i write good code
    memcpy(&justenditalready, &datapointer[byteSelect], n);
    return justenditalready;
}

template <typename T, typename ADR> void RamMemory<T, ADR>::write(ADR adres, T value, unsigned char n) {
    //get ready for some more unperormant trash piece of shit code
    char byteSelect = adres & 0b1; //get the value of the first bit
    unsigned char* datapointer = (unsigned char*) &this->ram[adres >> 1];
    if (&datapointer[byteSelect] + n > (unsigned char*)this->ram.lastElement) {
        return;
    }
    memcpy(&datapointer[byteSelect], &value, n);//copy the selected bytes into ram
}

//finely regained some of my sanity after finishing these trash pieces of code


//class FlashDevice
FlashDevice::FlashDevice(CPU* parent, unsigned int size) : flash(size),
                                                           Component(parent)
{
    loadFlash();
}

//load the Flash.bin file into the flash memory
void FlashDevice::loadFlash() {
    std::ifstream file("Flash.bin", std::ios::binary);
    if (!file) {
        std::cerr << "Error: could not open \"Flash.bin\"\n";
        return;
    }
    file.unsetf(std::ios::skipws);
    file.read((char*)this->flash.array, this->flash.size);
}

//store the flash memory into the Flash.bin file
void FlashDevice::storeFlash() {
    std::ofstream file("Flash.bin", std::ios::binary);
    if(!file) {
        std::cerr << "Error: could not open \"Flash.bin\" warning flash memory was not saved!\n";
    }
    file.unsetf(std::ios::skipws);
    file.write((char*)this->flash.array, this->flash.size);
}

void FlashDevice::dump(unsigned int start, unsigned int end) {
    for (int i = start; i < end; i++) {
        std::cout << std::hex << this->flash[i] << ", ";
    }
}

//this will request dma when the right io registers get the right values
void FlashDevice::update() {
    
}


//class SerialIODevice
SerialIODevice::SerialIODevice(CPU* parent) : Component(parent) {

}

void SerialIODevice::update() {
    //handle serial io stuff
}


//now the dma stuff
//class DMAChannel
template <typename T, typename ADR> DMAChannel<T, ADR>::DMAChannel() {
    this->source = NULL;
    this->size = 0;
    this->sourceAdres = 0;
    this->destAdres = 0;
    this->RW = false;
}

//class DMAControllerDevice
template <typename T, typename ADR> DMAControllerDevice<T, ADR>::DMAControllerDevice(CPU* parent) : Component(parent) {
    
}

//request a DMA read
template <typename T, typename ADR> bool DMAControllerDevice<T, ADR>::DMARead(int channel, DynamicArray<T>* dest, ADR size, unsigned int destAdres, ADR sourceAdres) {
    DMAChannel<T, ADR>* currentChannel = &DMAChannelArray[channel];
    if (currentChannel->size == 0) {
        currentChannel->source = dest;
        currentChannel->size = size;
        currentChannel->sourceAdres = destAdres;
        currentChannel->destAdres = sourceAdres;
        currentChannel->RW = false;
        return true;
    } else {
        return false; //dma request got denied since the channel was busy
    }
}

//request a DMA write
template <typename T, typename ADR> bool DMAControllerDevice<T, ADR>::DMAWrite(int channel, DynamicArray<T>* source, ADR size, unsigned int sourceAdres, ADR destAdres) {
    DMAChannel<T, ADR>* currentChannel = &DMAChannelArray[channel];
    if (currentChannel->size == 0) {
        currentChannel->source = source;
        currentChannel->size = size;
        currentChannel->sourceAdres = sourceAdres;
        currentChannel->destAdres = destAdres;
        currentChannel->RW = true;
        return true;
    } else {
        return false; //dma request got denied since the channel was busy
    }
}

//handles the DMA requests
template <typename T, typename ADR> void DMAControllerDevice<T, ADR>::update() {
    for (int channel = 0; channel < channels; channel++) {//this fr makes my brain hurt
        DMAChannel<T, ADR>* currentChannel = &DMAChannelArray[channel];
        if (currentChannel->size > 0) {
            if (currentChannel->RW) {//write
                parent->memory.write(currentChannel->destAdres, (*currentChannel->source)[currentChannel->sourceAdres], sizeof(T));
                currentChannel->sourceAdres++; //this needs to be changed so that it depends on the size of T
                currentChannel->destAdres++;
                currentChannel->size--;
            } else {//read
                (*currentChannel->source)[currentChannel->sourceAdres] = parent->memory.read(currentChannel->destAdres, sizeof(T));
                currentChannel->sourceAdres++;//this also needs to be changed
                currentChannel->destAdres++;
                currentChannel->size--;
            }
            if (currentChannel->size == 0) {//dma finished now we should trigger a hardware interupt
                std::cout << "dma finished!\n";
            }
        }
    }
}
