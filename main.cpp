#include <cassert>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <numeric>
#include <ostream>
#include <random>
#include <vector>

// utill function
auto rand_double(std::mt19937 &gen) -> double {
  return static_cast<double>(gen()) / static_cast<double>(gen.max());
}

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

// All operation kinds are flattened in this struct.
// This approach choosen to avoid usege of dynamic polymorphism.
// But it makes operations extra huge almost for every kind.
struct operation {
  op_kind kind;
  operation *arg_1;
  operation *arg_2;
  operation *arg_3;

  // Recursive evaluation function.
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
      // FIXME: Ugly
      v.x = std::bit_cast<double>(this->arg_1);
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

  // Recursive operation node generation.
  auto generate_randow_args(size_t level, std::mt19937 &gen) -> void {
    std::cout << op_kind_name(this->kind);
    switch (this->kind) {
    case op_kind::Tripple:
      this->arg_3 = rand_operation(level, gen);
      this->arg_3->generate_randow_args(level + 1, gen);
    case op_kind::Add:
    case op_kind::Sub:
    case op_kind::Mul:
    case op_kind::Div:
    case op_kind::Mod:
      this->arg_1 = rand_operation(level, gen);
      this->arg_1->generate_randow_args(level + 1, gen);
      this->arg_2 = rand_operation(level, gen);
      this->arg_2->generate_randow_args(level + 1, gen);
      break;
    case op_kind::Rnd:
      this->arg_1 = std::bit_cast<operation *>(rand_double(gen));
    case op_kind::X:
    case op_kind::Y:
      break;
    case op_kind::COUNT:
      assert(false && "unreachable");
    }
  }

private:
  // over complicated function to randomly choose kind of operation
  static auto rand_operation(size_t level, std::mt19937 &gen) -> operation * {
    // Limit is necessary, because otherwise recursion is diverges.
    // So ecursion tree is going to infinite growth.
    // This is problem of realized method of choosing random operation.
    if (level > 12) {
      return new operation{.kind = op_kind::Rnd,
                           .arg_1 = nullptr,
                           .arg_2 = nullptr,
                           .arg_3 = nullptr};
    }

    constexpr auto probabilities =
        std::array{0, 200, 200, 200, 200, 200, 200, 200, 200};
    //       Tripple, Add, Sub, Mul, Div, Mod, Rnd, X,   Y
    // Probabilitees of all operations

    constexpr auto prefixes = [probabilities]() {
      auto res = std::array<int, probabilities.size() + 1>();
      std::partial_sum(std::begin(probabilities), std::end(probabilities),
                       std::begin(res));
      return res;
    }();
    // is 100%, max value
    constexpr auto sum = prefixes.at(prefixes.size() - 1);
    // random num in range 0 to sum
    auto rand_num = std::ceil(rand_double(gen) * static_cast<double>(sum));
    auto op_idx = [prefixes, rand_num]() {
      for (size_t i = 0; i < (prefixes.size() - 1); i++) {
        if (prefixes[i] >= rand_num) {
          return i;
        }
      }
      return prefixes.size() - 1;
    }();
    auto rand_op = static_cast<op_kind>(op_idx);
    return new operation{
        .kind = rand_op, .arg_1 = nullptr, .arg_2 = nullptr, .arg_3 = nullptr};
  }
};

auto main() -> int {
  // create random number source with curr time based seed
  auto time = std::chrono::duration_cast<std::chrono::microseconds>(
                  std::chrono::high_resolution_clock::now().time_since_epoch())
                  .count();
  std::random_device rd;
  std::mt19937::result_type seed = rd() ^ (std::mt19937::result_type)time;
  std::mt19937 gen(seed);

  // image size
  constexpr auto dimx = 2000u, dimy = 2000u;

  {
    std::ofstream ofs("output/tmp.ppm",
                      std::ios_base::out | std::ios_base::binary);
    // ppm file header, reference at
    // https://rosettacode.org/wiki/Bitmap/Write_a_PPM_file#C++
    ofs << "P6\n" << dimx << ' ' << dimy << "\n255\n";

    operation random_function;
    random_function.generate_randow_args(1, gen);

    for (auto j = 0u; j < dimy; ++j)
      for (auto i = 0u; i < dimx; ++i) {
        double x = static_cast<double>(i) / static_cast<double>(dimx);
        double y = static_cast<double>(j) / static_cast<double>(dimy);

        // fill image with generated function in normilized coordinates
        value res = random_function.Evaluate(x, y);

        ofs << static_cast<char>(res.x * 256.0)
            << static_cast<char>(res.y * 256.0)
            << static_cast<char>(res.z * 256.0);
      }
  }
  // tmp used in viewer and refreshes after re-generation
  // seeded image saved to collection
  std::filesystem::copy_file("output/tmp.ppm",
                             std::format("output/{}.ppm", time));
  return 0;
}
