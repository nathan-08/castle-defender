#include "app.hpp"
#include <iostream>
using namespace std;

int main() {
  App::init();
  App::mainloop();
  App::close();
}
