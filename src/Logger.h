#ifndef LOGGERSTATEH
#define LOGGERSTATEH

#include <Print.h>

const uint16_t MAX_SIZE = 4096;

class Logger : public Print {
 public:
  Logger(Print* dup = nullptr) : Print(), _dup(dup), _buffer(nullptr) {
  }
  ~Logger() {
    if (_buffer)
      delete _buffer;
  }

  bool begin();
  void clear();
  uint16_t length() const;
  operator const char*();
  const char* getBuffer();
  size_t write(uint8_t val);

 protected:
  Print* _dup;
  char* _buffer;
  uint16_t _length;
};

extern Logger logger;

#endif