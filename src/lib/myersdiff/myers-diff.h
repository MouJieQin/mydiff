#ifndef _MYERS_DIFF_H_
#define _MYERS_DIFF_H_

#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>

namespace mydiff {

template <typename FIter,
          typename Compare =
              std::equal_to<typename std::iterator_traits<FIter>::value_type>>
class MyersDiff {
 public:
  enum EDIT_SCRIPT { ES_RETAIN, ES_DELETE, ES_INSERT };
  typedef typename std::iterator_traits<FIter>::value_type value_type;
  typedef typename std::iterator_traits<FIter>::difference_type difference_type;
  typedef difference_type diff_t;
  typedef std::pair<diff_t, diff_t> point_t;
  typedef std::vector<std::pair<EDIT_SCRIPT, diff_t>> ses_t;

 public:
  MyersDiff(const diff_t maxPath) : forward(maxPath), reverse(maxPath){};

 private:
  class IntIndexVector {
   public:
    IntIndexVector(const diff_t maxPath)
        : offset_(maxPath), vec_(2 * maxPath + 1, 0) {}

    diff_t& operator[](const diff_t index) { return vec_[index + offset_]; }

    void reset(const diff_t maxK) {
      std::fill_n(vec_.begin() + (offset_ + (-maxK)), 2 * maxK + 1, 0);
    }

   private:
    diff_t offset_;
    std::vector<diff_t> vec_;
  };

 public:
  diff_t shortestEditScript(FIter first1, FIter last1, FIter first2,
                            FIter last2, ses_t& ses) {
    return shortestEditScript(first1, 0, std::distance(first1, last1), first2,
                              0, std::distance(first2, last2), ses);
  }

  diff_t shortestEditScript(FIter first1, const diff_t srcOffset,
                            const diff_t N, FIter first2,
                            const diff_t dstOffset, const diff_t M,
                            ses_t& ses) {
    ses_t tmpSes;
    diff_t lcs = shortestEditScriptImple(first1, srcOffset, N, first2,
                                         dstOffset, M, tmpSes);
    ses.swap(tmpSes);
    return lcs;
  }

 private:
  diff_t shortestEditScriptImple(FIter src, const diff_t srcOffset,
                                 const diff_t N, FIter dst,
                                 const diff_t dstOffset, const diff_t M,
                                 ses_t& ses) {
    if (M == 0 && N > 0) {
      for (diff_t i = 0; i < N; ++i) {
        ses.emplace_back(ES_DELETE, srcOffset + i);
      }
      return 0;
    }
    if (N == 0 && M > 0) {
      for (diff_t i = 0; i < M; ++i) {
        ses.emplace_back(ES_INSERT, dstOffset + i);
      }
      return 0;
    }
    if (N == 0 && M == 0) {
      return 0;
    }
    point_t head, tail;
    diff_t d =
        findMiddleSnake(src, srcOffset, N, dst, dstOffset, M, head, tail);
    if (d >= 1) {
      diff_t first = shortestEditScriptImple(src, srcOffset, head.first, dst,
                                             dstOffset, head.second, ses);
      for (diff_t i = head.first + 1; i <= tail.first; ++i) {
        ses.emplace_back(ES_RETAIN, srcOffset + i - 1);
      }
      diff_t last = shortestEditScriptImple(
          src, srcOffset + tail.first, N - tail.first, dst,
          dstOffset + tail.second, M - tail.second, ses);
      return first + last + tail.first - head.first;
    } else {
      for (diff_t i = head.first + 1; i <= tail.first; ++i) {
        ses.emplace_back(ES_RETAIN, srcOffset + i - 1);
      }
      return tail.first - head.first;
    }
    return 0;
  }

 private:
  bool overlap(FIter src, const diff_t srcOffset, const diff_t N, FIter dst,
               const diff_t dstOffset, const diff_t M, const point_t& head,
               const point_t& tail) {
    diff_t x = head.first, y = head.second;
    FIter xIter = std::next(src, srcOffset + (x - 1));
    FIter yIter = std::next(dst, dstOffset + (y - 1));
    for (; x < N && y < M && equalTo(*(++xIter), *(++yIter));) {
      x += 1;
      y += 1;
    }
    return x == tail.first && y == tail.second;
  }

  diff_t findMiddleSnake(FIter src, const diff_t srcOffset, const diff_t N,
                         FIter dst, const diff_t dstOffset, const diff_t M,
                         point_t& head, point_t& tail) {
    diff_t x, y;
    diff_t delta = N - M;
    bool odd = ((delta & 1) == 1);
    diff_t ceilD = (N + M + 1) / 2;
    forward.reset(ceilD);
    reverse.reset(ceilD);
    FIter xIter, yIter;
    for (diff_t d = 0; d <= ceilD; ++d) {
      for (diff_t k = -d; k <= d; k += 2) {
        if (k == -d || (k != d && forward[k - 1] < forward[k + 1])) {
          x = forward[k + 1];
        } else {
          x = forward[k - 1] + 1;
        }
        y = x - k;
        xIter = std::next(src, srcOffset + (x - 1));
        yIter = std::next(dst, dstOffset + (y - 1));
        for (; x < N && y < M && equalTo(*(++xIter), *(++yIter));) {
          x += 1;
          y += 1;
        }
        forward[k] = x;
        head = {N - reverse[delta - k], M - (reverse[delta - k] - (delta - k))};
        tail = {x, y};
        if (odd && overlap(src, srcOffset, N, dst, dstOffset, M, head, tail)) {
          return 2 * d - 1;
        }
      }
      for (diff_t k = -d; k <= d; k += 2) {
        if (k == -d || (k != d && reverse[k - 1] < reverse[k + 1])) {
          x = reverse[k + 1];
        } else {
          x = reverse[k - 1] + 1;
        }
        y = x - k;
        xIter = std::next(src, srcOffset + ((N - x) - 1));
        yIter = std::next(dst, dstOffset + ((M - y) - 1));
        for (; x < N && y < M && equalTo(*(xIter++), *(yIter++));) {
          x += 1;
          y += 1;
        }
        reverse[k] = x;
        head = {N - x, M - y};
        tail = {forward[delta - k], forward[delta - k] - (delta - k)};
        if (!odd && overlap(src, srcOffset, N, dst, dstOffset, M, head, tail)) {
          return 2 * d;
        }
      }
    }
    return 0;
  }

 private:
  IntIndexVector forward;
  IntIndexVector reverse;
  Compare equalTo;
};

}  // namespace mydiff

#endif