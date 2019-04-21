#include <iostream>
#include <string>
#include <vector>

template <typename ValType>
class IntIndexVector {
 public:
  IntIndexVector(int64_t startIndex, int64_t endIndex)
      : offset_(-startIndex), vec_(endIndex - startIndex + 1, 0) {}

  ValType& operator[](int64_t index) { return vec_[index + offset_]; }

  void reset() { vec_ = std::vector<ValType>(vec_.size(), 0); }

  void print() {
    for (int64_t i = -offset_; i <= offset_; ++i) {
      printf("%4lld", i);
    }
    std::cout << std::endl;
    for (int64_t i = -offset_; i <= offset_; ++i) {
      printf("%4llu", (*this)[i]);
    }
    std::cout << "\n----------------" << std::endl;
  }

 private:
  int64_t offset_;
  std::vector<ValType> vec_;
};

class MyersDiff {
 public:
  typedef std::pair<int64_t, int64_t> point;
  MyersDiff(const std::string& src, const std::string& dst)
      : forward(-(src.size() + dst.size()), src.size() + dst.size()),
        reverse(-(src.size() + dst.size()), src.size() + dst.size()) {}

  bool overlap(const std::string& src, const int64_t srcPos, const int64_t N,
               const std::string& dst, const int64_t dstPos, const int64_t M,
               point& head, point& tail) {
    int64_t x = head.first, y = head.second;
    for (; x < N && y < M &&
           src[srcPos + x - 1 + 1] == dst[dstPos + y - 1 + 1];) {
      x += 1;
      y += 1;
    }
    return x == tail.first && y == tail.second;
  }

  int64_t middleSnake(const std::string& src, const int64_t srcPos,
                      const int64_t N, const std::string& dst,
                      const int64_t dstPos, const int64_t M, point& head,
                      point& tail) {
    std::cout << "src: ";
    for (size_t i = 0; i != N; ++i) {
      std::cout << src[srcPos + i];
    }
    std::cout << std::endl;
    std::cout << "dst: ";
    for (size_t i = 0; i != M; ++i) {
      std::cout << dst[dstPos + i];
    }
    std::cout << std::endl;
    int64_t x, y;
    int64_t delta = N - M;
    bool odd = ((uint64_t)delta & (uint64_t)1) == 1;
    forward.reset();
    reverse.reset();
    for (int64_t d = 0; d <= (N + M + 1) / 2; ++d) {
      for (int64_t k = -d; k <= d; k += 2) {
        if (k == -d || (k != d && forward[k - 1] < forward[k + 1])) {
          x = forward[k + 1];
        } else {
          x = forward[k - 1] + 1;
        }
        y = x - k;
        for (; x < N && y < M &&
               src[srcPos + x - 1 + 1] == dst[dstPos + y - 1 + 1];) {
          x += 1;
          y += 1;
        }
        forward[k] = x;
        head = {N - reverse[delta - k], M - (reverse[delta - k] - (delta - k))};
        tail = {x, y};
        if (odd && overlap(src, srcPos, N, dst, dstPos, M, head, tail)) {
          std::cout << "foward overloadps: (" << head.first << ","
                    << head.second << ") -> (" << tail.first << ","
                    << tail.second << "),delta: " << delta << std::endl;
          return 2 * d - 1;
        }
      }
      for (int64_t k = -d; k <= d; k += 2) {
        if (k == -d || (k != d && reverse[k - 1] < reverse[k + 1])) {
          x = reverse[k + 1];
        } else {
          x = reverse[k - 1] + 1;
        }
        y = x - k;
        for (; x < N && y < M &&
               src[srcPos + (N - x) - 1] == dst[dstPos + (M - y) - 1];) {
          x += 1;
          y += 1;
        }
        reverse[k] = x;
        head = {N - x, M - y};
        tail = {forward[delta - k], forward[delta - k] - (delta - k)};
        if (!odd && overlap(src, srcPos, N, dst, dstPos, M, head, tail)) {
          std::cout << "reverse overloadps: (" << head.first << ","
                    << head.second << ") -> (" << tail.first << ","
                    << tail.second << "),delta: " << delta << std::endl;
          return 2 * d;
        }
      }
      std::cout << "******************" << d << "****************" << std::endl;
      forward.print();
      reverse.print();
    }
    return 0;
  }

  int64_t LCS(const std::string& src, const int64_t srcPos, const int64_t N,
              const std::string& dst, const int64_t dstPos, const int64_t M,
              std::string& diffOut) {
    std::cout << "LCS----start" << std::endl;
    std::cout << "src: ";
    for (size_t i = 0; i != N; ++i) {
      std::cout << src[srcPos + i];
    }
    std::cout << std::endl;
    std::cout << "dst: ";
    for (size_t i = 0; i != M; ++i) {
      std::cout << dst[dstPos + i];
    }
    std::cout << std::endl;
    std::cout << "LCS----end" << std::endl;
    if (M == 0 && N > 0) {
      for (int64_t i = 0; i < N; ++i) {
        diffOut.push_back('-');
        diffOut.push_back(src[srcPos + i]);
        diffOut.push_back('\n');
      }
      return 0;
    }
    if (N == 0 && M > 0) {
      for (int64_t i = 0; i < M; ++i) {
        diffOut.push_back('+');
        diffOut.push_back(dst[dstPos + i]);
        diffOut.push_back('\n');
      }
      return 0;
    }
    if (N == 0 && M == 0) {
      return 0;
    }
    point head, tail;
    int64_t d = middleSnake(src, srcPos, N, dst, dstPos, M, head, tail);
    std::cout << "d: " << d << std::endl;
    if (d >= 1) {
      int64_t first =
          LCS(src, srcPos, head.first, dst, dstPos, head.second, diffOut);
      for (int64_t i = head.first + 1; i <= tail.first; ++i) {
        diffOut.push_back(src[srcPos + i - 1]);
        diffOut.push_back('\n');
      }
      int64_t last = LCS(src, srcPos + tail.first, N - tail.first, dst,
                         dstPos + tail.second, M - tail.second, diffOut);
      return first + last + tail.first - head.first;
    } else {
      for (int64_t i = head.first+1; i <= tail.first; ++i) {
        diffOut.push_back(src[srcPos + i - 1]);
        diffOut.push_back('\n');
      }
      return tail.first - head.first;
    }
    return 0;
  }

 private:
  IntIndexVector<int64_t> forward;
  IntIndexVector<int64_t> reverse;
};

int main() {
  std::string src = "ABCABBA";
  std::string dst = "CBABAC";
  MyersDiff diff(src, dst);
  std::string out;
  auto lcs = diff.LCS(src, 0, src.size(), dst, 0, dst.size(), out);
  std::cout << "##################\nLCS: " << lcs << std::endl;
  std::cout << out << std::endl;
  return 0;
}
