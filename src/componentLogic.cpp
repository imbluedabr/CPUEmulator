#include "cpu.h"

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

template <typename T, typename ADR> RamMemory<T>::RamMemory(CPU* parent, ADR size) :
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
    constexpr buffersize = wordSize/512;
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
    for (ADR i = start, i < end; i++) {
        std::cout << std::hex << this->ram[i] << ", \n";
    }
}

//read bytes from ram
template <typename T, typename ADR> unsigned char* RamMemory<T, ADR>::read(ADR adres, unsigned char n) {
    char byteSelect = adres & 0b1;
    unsigned char* datapointer = (unsigned char*) &this->ram[adres >> 1]);//add the pointer to the cacheline to the byteselect to calculate the final adres
    if (&datapointer[byteSelect] + n > this->ram.lastElement) {
        return &DynamicArray::justkillmealready; //if the last byte that we read is above the last element we got to return
    }
    return &datapointer[byteSelect];
}

template <typename T, typename ADR> void RamMemory<T, ADR>::write(ADR adres, T value, unsigned char n) {
    this->ram[adres << 1] = value;
}
