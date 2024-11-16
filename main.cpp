#include <cassert>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>

enum class op_kind { Tripple, Add, Sub, Mul, Div, Mod, Rnd, X, Y, COUNT };

auto op_kind_name(op_kind kind) -> const char * {
  switch (kind) {
  case op_kind::Tripple:
    return "Tripple";
  case op_kind::Add:
    return "Add";
  case op_kind::Sub:
    return "Sub";
  case op_kind::Mul:
    return "Mul";
  case op_kind::Div:
    return "Div";
  case op_kind::Mod:
    return "Mod";
  case op_kind::Rnd:
    return "Rnd";
  case op_kind::X:
    return "X";
  case op_kind::Y:
    return "Y";
  case op_kind::COUNT:
    return "COUNT";
  }
}

struct value {
  double x, y, z;
};

struct operation {
  op_kind kind;
  operation *arg_1;
  operation *arg_2;
  operation *arg_3;

  auto Evaluate(double x, double y) -> value {
    value v;
    switch (this->kind) {
    case op_kind::Tripple:
      v = {.x = arg_1->Evaluate(x, y).x,
           .y = arg_2->Evaluate(x, y).x,
           .z = arg_3->Evaluate(x, y).x};
      break;
    case op_kind::Add:
      v.x = arg_1->Evaluate(x, y).x + arg_2->Evaluate(x, y).x;
      break;
    case op_kind::Sub:
      v.x = arg_1->Evaluate(x, y).x - arg_2->Evaluate(x, y).x;
      break;
    case op_kind::Mul:
      v.x = arg_1->Evaluate(x, y).x * arg_2->Evaluate(x, y).x;
      break;
    case op_kind::Div:
      v.x = arg_1->Evaluate(x, y).x / arg_2->Evaluate(x, y).x;
      break;
    case op_kind::Mod:
      v.x = std::fmod(arg_1->Evaluate(x, y).x, arg_2->Evaluate(x, y).x);
      break;
    case op_kind::Rnd:
      v.x = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
      break;
    case op_kind::X:
      v.x = x;
      break;
    case op_kind::Y:
      v.x = y;
      break;
    case op_kind::COUNT:
      assert(false && "unreachable");
    }
    return v;
  }
  auto generate_randow_args() -> void {
    std::cout << op_kind_name(this->kind);
    switch (this->kind) {
    case op_kind::Tripple:
      this->arg_3 = rand_operation();
      this->arg_3->generate_randow_args();
    case op_kind::Add:
    case op_kind::Sub:
    case op_kind::Mul:
    case op_kind::Div:
    case op_kind::Mod:
      this->arg_1 = rand_operation();
      this->arg_1->generate_randow_args();
      this->arg_2 = rand_operation();
      this->arg_2->generate_randow_args();
      break;
    case op_kind::Rnd:
    case op_kind::X:
    case op_kind::Y:
      break;
    case op_kind::COUNT:
      assert(false && "unreachable");
    }
  }

private:
  static auto rand_operation() -> operation * {
    auto count = static_cast<int>(op_kind::COUNT);
    auto rand_num = std::ceil(
        (static_cast<double>(rand()) / static_cast<double>(RAND_MAX)) *
        static_cast<double>(count - 1));
    auto rand_op = static_cast<op_kind>(rand_num);
    return new operation{
        .kind = rand_op, .arg_1 = nullptr, .arg_2 = nullptr, .arg_3 = nullptr};
  }
};

auto main() -> int {
  constexpr auto dimx = 1000u, dimy = 1000u;

  std::ofstream ofs("first.ppm", std::ios_base::out | std::ios_base::binary);
  ofs << "P6\n" << dimx << ' ' << dimy << "\n255\n";

  operation random_function;
  random_function.generate_randow_args();

  for (auto j = 0u; j < dimy; ++j)
    for (auto i = 0u; i < dimx; ++i) {
      double x = static_cast<double>(i) / static_cast<double>(dimx);
      double y = static_cast<double>(j) / static_cast<double>(dimy);

      value res = random_function.Evaluate(x, y);

      ofs << static_cast<char>(res.x * 256.0)
          << static_cast<char>(res.y * 256.0)
          << static_cast<char>(res.z * 256.0);
    }
}
