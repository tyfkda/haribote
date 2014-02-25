#include "apilib.h"
#include "stdio.h"
#include "stdlib.h"

class Base {
public:
  Base() {
    printf("Base::ctor called: %p\n", this);
  }
  virtual ~Base() {
    printf("Base::dtor called\n");
  }
  virtual void hello() = 0;
};

class Main : public Base {
public:
  Main(int param) : param_(param) {
    printf("Main::ctor called: %p, %d\n", this, param_);
  }
  virtual ~Main() {
    printf("Main::dtor called\n");
  }
  virtual void hello() override {
    printf("Main#hello\n");
  }

protected:
  static constexpr int W = 150, H = 50;
  unsigned char buf_[W * H];
  int param_;
  int win_;
};

int main() {
  puts("main start");
  Base* win = new Main(123);
  win->hello();
  delete win;
  puts("main end");
  return 0;
}
