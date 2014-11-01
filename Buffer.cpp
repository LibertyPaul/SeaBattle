#include <cstring>
#include <stdexcept>

using namespace std;
#include "Buffer.hpp"

Buffer::Buffer(): size(0), buffer(nullptr), readPos(0), writePos(0){}
Buffer::Buffer(uint64_t size): size(size), buffer(new uint8_t[size]), readPos(0), writePos(0){}

Buffer::Buffer(const Buffer &buffer): size(buffer.getSize()), buffer(new uint8_t[size]), readPos(buffer.readPos), writePos(buffer.writePos){
	memcpy(this->get(), buffer.get(), size);
}

Buffer::Buffer(Buffer &&buffer): size(buffer.getSize()), buffer(move(buffer.buffer)), readPos(buffer.readPos), writePos(buffer.writePos){}

const Buffer &Buffer::operator=(Buffer &&buffer){
	this->buffer = move(buffer.buffer);
	this->size = buffer.size;
	this->readPos = buffer.readPos;
	this->writePos = buffer.writePos;

	return *this;
}


//кагоцел 1
//вильпрофен 500мг

void Buffer::append(const Buffer &buffer){
	uint64_t sumSize = this->getSize() + buffer.getSize();//для безопасости надо бы поставить ограничение макс. размера в 2^63

	unique_ptr<uint8_t> newBlock(new uint8_t[sumSize]);

	this->lowLevelRead(newBlock.get(), this->getSize());
	buffer.lowLevelRead(newBlock.get() + this->getSize(), buffer.getSize());

	this->buffer = move(newBlock);
	this->size = sumSize;

	if(this->getWriteSpace() > 0)
		this->dropPointers();//если в конце 1-го блока было не инициализированное место, то нарушается логика, поэтому просто сбрасываем поинтеры
	else
		this->writePos += buffer.writePos;//если инициализированное пространство не разделено, то можно продолжать его читать.
}



void Buffer::lowLevelWrite(const void *data, uint64_t length, uint64_t from){
	if(getSize() - from + 1 < length)
		throw logic_error("Buffer out of range");

	memcpy(buffer.get() + from, data, length);
}

void Buffer::lowLevelRead(void *dst, uint64_t length, uint64_t from) const{
	if(getSize() - from + 1 < length)
		throw logic_error("Buffer out of range");

	memcpy(dst, buffer.get() + from, length);
}



void Buffer::dropPointers(){
	readPos = 0;
	writePos = 0;
}


void Buffer::write(const void *data, uint64_t length){
	lowLevelWrite(data, length, writePos);
	writePos += length;
}

void Buffer::write(istream &i, uint64_t length){
	if(getWriteSpace() < length)
		throw logic_error("Buffer overflow");

	void *ptr = buffer.get() + writePos;
	i.read(reinterpret_cast<char *>(ptr), length);
	writePos += length;
}


uint64_t Buffer::getWriteSpace() const{
	return getSize() - writePos;
}

uint64_t Buffer::getReadSpace() const{
	return getSize() - readPos;
}

void Buffer::read(void *dst, uint64_t length){
	lowLevelRead(dst, length, readPos);
	readPos += length;
}


void Buffer::read(ostream &o, uint64_t length){
	if(getReadSpace() < length)
		throw logic_error("Buffer reading overflow");

	void *ptr = buffer.get() + readPos;
	o.write(reinterpret_cast<char *>(ptr), length);
	readPos += length;
}


void *Buffer::get(){
	return buffer.get();
}

const void *Buffer::get() const{
	return buffer.get();
}

uint64_t Buffer::getSize() const{
	return size;
}





