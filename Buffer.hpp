#ifndef BUFFER_HPP_INCLUDED
#define BUFFER_HPP_INCLUDED

#include <memory>
#include <istream>
#include <ostream>
#include <vector>
#include <stdexcept>


class Buffer{
	uint64_t size;
	std::unique_ptr<uint8_t> buffer;

	uint64_t readPos, writePos;

	void lowLevelWrite(const void *data, uint64_t length, uint64_t from = 0);//lowLevel операции не затрагивают readPos и writePos
	void lowLevelRead(void *dst, uint64_t length, uint64_t from = 0) const;
public:
	Buffer();
	Buffer(uint64_t size);

	template<typename T>
	Buffer(const std::vector<T> &src): size(src.size() * sizeof(T)), buffer(new uint8_t[size]), readPos(0), writePos(size){
		lowLevelWrite(&src[0], this->size);
	}


	Buffer(const Buffer &buffer);
	Buffer(Buffer &&buffer);

	const Buffer &operator=(Buffer &&buffer);
	bool operator==(const Buffer &buffer) const;

	void fill(uint8_t value);
	void fill(const std::function<uint8_t()> &getValue);


	void append(const Buffer &buffer);

	template<typename T>
	void push_back(const T &t){
		Buffer t_buf(sizeof(t));
		t_buf.write<T>(t);
		this->append(t_buf);
	}

	void write(const void *data, uint64_t length);
	void write(std::istream &i, uint64_t length);
	void read(void *dst, uint64_t length);
	void read(std::ostream &o, uint64_t length);


	template<typename T>
	void write(const T &t){
		write(&t, sizeof(T));
	}

	template<typename T>
	T read(){
		T t;
		read(&t, sizeof(t));
		return t;
	}



	void dropPointers();

	uint64_t getWriteSpace() const;
	uint64_t getReadSpace() const;

	void *get();
	const void *get() const;




	uint64_t getSize() const;

	std::string toString() const;
};



#endif // BUFFER_HPP_INCLUDED
