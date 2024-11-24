#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <unordered_map>
#include <utility>
#include <vector>

// utill function
auto rand_double() -> double {
  return static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX);
}

enum struct op_kind : int8_t {
  Tripple,
  Add,
  Sub,
  Mul,
  Div,
  Mod,
  Rnd,
  X,
  Y,
  COUNT
};
constexpr int op_kind_count = static_cast<int>(op_kind::COUNT);

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
auto op_kind_is_expr(op_kind kind) -> int8_t {
  switch (kind) {
  case op_kind::Tripple:
  case op_kind::Add:
  case op_kind::Sub:
  case op_kind::Mul:
  case op_kind::Div:
  case op_kind::Mod:
    return 1;
  case op_kind::Rnd:
  case op_kind::X:
  case op_kind::Y:
    return -1;
  case op_kind::COUNT:
    assert(0 && "count is not a kind");
  }
}

template <typename value_t> struct value {
  value_t x, y, z;
};

struct operation {
  op_kind kind;
};

template <typename value_t>
value<value_t> eval(value_t x, value_t y, value_t *values, int values_count,
                    int8_t *ops, int ops_count) {

  int op_i = 0, val_i = 0;

  auto calc_bin_op = [&](std::function<value_t(value_t, value_t)> f) {
    value_t a = values[val_i];
    val_i -= 1;
    value_t b = values[val_i];
    values[val_i] = f(a, b);
  };

  while (true) {
    op_kind kind = static_cast<op_kind>(ops[op_i]);
    op_i += 1;

    switch (kind) {
    case op_kind::Tripple:
      return {values[val_i], values[val_i - 1], values[val_i - 2]};
    case op_kind::Add:
      calc_bin_op([](value_t a, value_t b) { return a + b; });
      break;
    case op_kind::Sub:
      calc_bin_op([](value_t a, value_t b) { return a - b; });
      break;
    case op_kind::Mul:
      calc_bin_op([](value_t a, value_t b) { return a * b; });
      break;
    case op_kind::Div:
      calc_bin_op([](value_t a, value_t b) { return a / b; });
      break;
    case op_kind::Mod:
      calc_bin_op([](value_t a, value_t b) { return std::fmod(a, b); });
      break;
    case op_kind::Rnd:
      values[val_i] = *reinterpret_cast<value_t *>(ops + op_i);
      val_i += 1;
      op_i += sizeof(value_t);
      break;
    case op_kind::X:
      values[val_i] = x;
      val_i += 1;
      break;
    case op_kind::Y:
      values[val_i] = y;
      val_i += 1;
      break;
    case op_kind::COUNT:
      assert(0 && "impossible to get count as real kind");
      break;
    }
  }
}

union four_bytes {
  double d;
  int8_t b[4];
};

auto generate_expression() -> std::pair<std::vector<int8_t>, int> {
  constexpr int weights[op_kind_count] = {0, 1, 1, 1, 0, 0, 1, 1, 1};
  constexpr auto weights_prefix = [&]() {
    std::array<int, op_kind_count> tmp = {};
    std::partial_sum(weights, weights + op_kind_count, tmp.begin());
    return tmp;
  }();
  for (auto v : weights_prefix) {
    std::cout << v << " ";
  }

  std::cout << weights_prefix.back() << " ";
  auto take_random_kind = [&]() {
    int rnd = std::rand() % weights_prefix.back();
    for (int i = 0; i < op_kind_count; i++) {
      if (weights_prefix.at(i) > rnd) {
        return static_cast<op_kind>(i);
      }
    }
    assert(0 && "impossible to get random out of bound");
  };
  std::vector<int8_t> buffer = {static_cast<int8_t>(op_kind::Tripple)};
  int balance = 3;
  int values_count = 0;
  while (true) {
    if (balance == 0) {
      break;
    }
    op_kind kind = take_random_kind();
    std::cout << op_kind_name(kind) << " ";
    auto is_expr = op_kind_is_expr(kind);
    balance = balance + is_expr;
    values_count += is_expr == -1;
    if (kind == op_kind::Rnd) {
      four_bytes val = {.d = rand_double()};
      std::reverse_copy(val.b, val.b + 4, std::back_inserter(buffer));
    }
    buffer.push_back(static_cast<int8_t>(kind));
  }

  std::reverse(buffer.begin(), buffer.end());
  return {buffer, values_count};
}

auto main() -> int {
  auto time = std::time(nullptr);
  std::srand(time);
  // image size
  constexpr auto dimx = 1000u, dimy = 1000u;

  {
    std::ofstream ofs("output/tmp.ppm",
                      std::ios_base::out | std::ios_base::binary);

    // ppm file header, reference at
    // https://rosettacode.org/wiki/Bitmap/Write_a_PPM_file#C++
    ofs << "P6\n" << dimx << ' ' << dimy << "\n255\n";

    // TODO: init expr
    auto [ops, values_count] = generate_expression();
    std::vector<double> values(values_count, 0.0);

    for (auto j = 0u; j < dimy; ++j)
      for (auto i = 0u; i < dimx; ++i) {
        double x = static_cast<double>(i) / static_cast<double>(dimx);
        double y = static_cast<double>(j) / static_cast<double>(dimy);

        // fill image with generated function in normilized coordinates
        value res = eval<double>(x, y, values.data(), values.size(), ops.data(),
                                 ops.size());

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
