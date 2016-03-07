#ifndef _Arduino_h_
#define _Arduino_h_
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#define NOT_ARDUINO

#define pinMode(a,b)
#define digitalWrite(a,b)

void setup(void);
void loop(void);

typedef uint8_t byte;

class Print;
class Printable
{
 public:
  virtual size_t printTo(Print& p) const = 0;
};

class Print
{
 public:
  void begin(int){};
  void write(const char *buf){fputs(buf,stdout);}
  void print(const char *buf){write(buf);}
  void print(int k){printf("%d",k);}
  void print(unsigned k){printf("%u",k);}
  void print(char k){putchar(k);}
  void print(const Printable& x){x.printTo(*this);}
  void println(void){write("\n");}
  void println(const char *buf){write(buf);write("\n");}
  void println(int k){print(k);write("\n");}
  void println(unsigned k){print(k);write("\n");}
  void println(const Printable& x){print(x);write("\n");}
  bool available(){return true;}
  char read(void);
};

extern Print Serial;
#endif