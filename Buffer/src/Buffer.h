#ifndef BUFFER_H_
#define BUFFER_H_

class Buffer final {
private:
  static const unsigned BUFFER_SIZE = 512;

  const char* fileName;
  char curBuffer[BUFFER_SIZE];
  //char* prevBuffer;
  unsigned off;
public:
	explicit Buffer(const char* fileName);
  ~Buffer();

  char nextChar();
  char currentChar();
  unsigned offset();
  char* range(char *buffer, const unsigned offset, const unsigned len);
  void setOffset(const unsigned offset);
private:
  void readFile(const char* name);
  void fillBuffer();
};

#endif
