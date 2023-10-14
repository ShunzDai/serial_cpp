#include "serial.h"

void test1() {
  using data_t = std::tuple<
    int, int &, const int, const int &,
    std::tuple<>, std::tuple<> &, const std::tuple<>, const std::tuple<> &,
    std::tuple<int>, std::tuple<int> &, const std::tuple<int>, const std::tuple<int> &
  >;
  int i1 = 0;
  std::tuple<> t1 = {};
  std::tuple<int> t2 = {};
  auto data = serial::pack(data_t{
    i1, i1, i1, i1,
    t1, t1, t1, t1,
    t2, t2, t2, t2
  });
  serial::unpack<
    int, int &, const int, const int &
  >(data);
}

int main(int argc, char *argv[]) {
  test1();
  return 0;
}
