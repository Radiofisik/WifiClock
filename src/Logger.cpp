#include <Logger.h>

bool Logger::begin() {
  if (!_buffer) {
    _buffer = new char[MAX_SIZE];
    if (!_buffer)
      return false;
  }
  clear();
  return true;
}

void Logger::clear() {
  *_buffer = '\0';
  _length = 0;
}

inline uint16_t Logger::length() const {
  return _length;
}

inline Logger::operator const char*() {
  return _buffer;
}

const char* Logger::getBuffer(){
  return _buffer;
}

size_t Logger::write(uint8_t val) {
  if (val != '\r') {
    if (_length >= MAX_SIZE - 1) {
      uint16_t start = 0;

      while ((start < MAX_SIZE / 2) && (_buffer[start] != '\n'))  // Find EOL
        ++start;
      ++start;  // Skip '\n'
      _length -= start;
      memmove(_buffer, &_buffer[start], _length);
    }
    _buffer[_length++] = val;
    _buffer[_length] = '\0';
  }
  if (_dup)
    _dup->write(val);
  return sizeof(val);
}
