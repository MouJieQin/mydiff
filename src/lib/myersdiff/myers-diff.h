#ifndef _MYERS_DIFF_H_
#define _MYERS_DIFF_H_

#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>

namespace mydiff {

template <typename Iter>
using iter_dif_t = typename std::iterator_traits<Iter>::difference_type;

enum EDIT_SCRIPT { ES_RETAIN, ES_DELETE, ES_INSERT };

template <typename BIter>
using ses_t = std::vector<std::pair<EDIT_SCRIPT, iter_dif_t<BIter>>>;

template <typename BIter,
          typename Compare =
              std::equal_to<typename std::iterator_traits<BIter>::value_type>>
class MyersDiff {
 public:
  typedef typename std::iterator_traits<BIter>::value_type value_type;
  typedef typename std::iterator_traits<BIter>::difference_type difference_type;
  typedef difference_type diff_t;
  typedef std::pair<diff_t, diff_t> point_t;
  typedef ses_t<BIter> ses_t;

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

  diff_t absIndex(const diff_t offset, const diff_t index) {
    return offset + (index - 1);
  }

 public:
  diff_t shortestEditScript(BIter first1, BIter last1, BIter first2,
                            BIter last2, ses_t& ses) {
    return shortestEditScript(first1, 0, std::distance(first1, last1), first2,
                              0, std::distance(first2, last2), ses);
  }

  diff_t shortestEditScript(BIter first1, const diff_t srcOffset,
                            const diff_t N, BIter first2,
                            const diff_t dstOffset, const diff_t M,
                            ses_t& ses) {
    ses_t tmpSes;
    diff_t lcs = shortestEditScriptImple(first1, srcOffset, N, first2,
                                         dstOffset, M, tmpSes);
    ses.swap(tmpSes);
    return lcs;
  }

 private:
  diff_t shortestEditScriptImple(BIter src, const diff_t srcOffset,
                                 const diff_t N, BIter dst,
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
    if (d == 0) {
      for (diff_t i = head.first + 1; i <= tail.first; ++i) {
        ses.emplace_back(ES_RETAIN, srcOffset + i - 1);
      }
      return tail.first - head.first;
    } else if (d == 1) {
      diff_t xForward = 0, yForward = 0;
      BIter xIter = std::next(src, srcOffset);
      BIter yIter = std::next(dst, dstOffset);
      diff_t len = 0;
      for (; xForward < N && yForward < M && equalTo(*(xIter++), *(yIter++));
           ++len) {
        xForward += 1;
        yForward += 1;
        ses.emplace_back(ES_RETAIN, absIndex(srcOffset, xForward));
      }
      if (xForward == head.first) {
        ses.emplace_back(ES_INSERT, absIndex(dstOffset, head.second));
      } else {
        ses.emplace_back(ES_DELETE, absIndex(srcOffset, head.first));
      }
      for (diff_t i = head.first + 1; i <= tail.first; ++i) {
        ses.emplace_back(ES_RETAIN, absIndex(srcOffset, i));
      }
      return len + tail.first - head.first;
    } else {
      diff_t first = shortestEditScriptImple(src, srcOffset, head.first, dst,
                                             dstOffset, head.second, ses);
      for (diff_t i = head.first + 1; i <= tail.first; ++i) {
        ses.emplace_back(ES_RETAIN, absIndex(srcOffset, i));
      }
      diff_t last = shortestEditScriptImple(
          src, absIndex(srcOffset, tail.first + 1), N - tail.first, dst,
          absIndex(dstOffset, tail.second + 1), M - tail.second, ses);
      return first + last + tail.first - head.first;
    }
    return 0;
  }

 private:
  diff_t findMiddleSnake(BIter src, const diff_t srcOffset, const diff_t N,
                         BIter dst, const diff_t dstOffset, const diff_t M,
                         point_t& head, point_t& tail) {
    diff_t last_x, last_y;
    diff_t x, y;
    diff_t kForward, kReverse;
    diff_t delta = N - M;
    bool odd = ((delta & 1) == 1);
    diff_t ceilHalfD = (N + M + 1) / 2;
    forward.reset(ceilHalfD);
    reverse.reset(ceilHalfD);
    BIter xIter, yIter;
    for (diff_t d = 0; d <= ceilHalfD; ++d) {
      for (diff_t k = -d; k <= d; k += 2) {
        if (k == -d || (k != d && forward[k - 1] < forward[k + 1])) {
          x = forward[k + 1];
        } else {
          x = forward[k - 1] + 1;
        }
        y = x - k;
        last_x = x;
        last_y = y;
        xIter = std::next(src, srcOffset + (x - 1));
        yIter = std::next(dst, dstOffset + (y - 1));
        for (; x < N && y < M && equalTo(*(++xIter), *(++yIter));) {
          x += 1;
          y += 1;
        }
        forward[k] = x;
        kReverse = delta - k;
        if (odd && kReverse >= -(d - 1) && kReverse <= (d - 1)) {
          if ((N - reverse[kReverse]) <= x) {
            tail = {x, y};
            head = {last_x, last_y};
            return d + (d - 1);
          }
        }
      }
      for (diff_t k = -d; k <= d; k += 2) {
        if (k == -d || (k != d && reverse[k - 1] < reverse[k + 1])) {
          x = reverse[k + 1];
          last_x = x;
          last_y = k + 1 - last_x;
        } else {
          x = reverse[k - 1] + 1;
          last_x = reverse[k - 1];
          last_y = k - 1 - last_x;
        }
        y = x - k;
        last_x = x;
        last_y = y;
        xIter = std::next(src, srcOffset + ((N - x) - 1));
        yIter = std::next(dst, dstOffset + ((M - y) - 1));
        for (; x < N && y < M && equalTo(*(xIter--), *(yIter--));) {
          x += 1;
          y += 1;
        }
        reverse[k] = x;
        if (!odd && k >= delta - d && k <= delta + d) {
          kForward = delta - k;
          if ((N - x) <= forward[kForward]) {
            head = {N - x, M - y};
            tail = {N - last_x, M - last_y};
            return 2 * d;
          }
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

template <typename BIter, typename Compare>
iter_dif_t<BIter> shortestEditScript(BIter first1, BIter last1, BIter first2,
                                     BIter last2, ses_t<BIter>& ses,
                                     const Compare& comp) {
  return shortestEditScript(first1, 0, std::distance(first1, last1), first2, 0,
                            std::distance(first2, last2), ses, comp);
}

template <typename BIter>
iter_dif_t<BIter> shortestEditScript(BIter first1, BIter last1, BIter first2,
                                     BIter last2, ses_t<BIter>& ses) {
  return shortestEditScript(
      first1, last1, first2, last2, ses,
      std::equal_to<typename std::iterator_traits<BIter>::value_type>());
}

template <typename BIter, typename Compare>
iter_dif_t<BIter> shortestEditScript(BIter first1,
                                     const iter_dif_t<BIter> srcOffset,
                                     const iter_dif_t<BIter> N, BIter first2,
                                     const iter_dif_t<BIter> dstOffset,
                                     const iter_dif_t<BIter> M,
                                     ses_t<BIter>& ses, const Compare& comp) {
  iter_dif_t<BIter> maxPath = (N + M + 1) / 2;
  MyersDiff<BIter, Compare> mydiff(maxPath);
  return mydiff.shortestEditScript(first1, srcOffset, N, first2, dstOffset, M,
                                   ses);
}

template <typename BIter>
iter_dif_t<BIter> shortestEditScript(BIter first1,
                                     const iter_dif_t<BIter> srcOffset,
                                     const iter_dif_t<BIter> N, BIter first2,
                                     const iter_dif_t<BIter> dstOffset,
                                     const iter_dif_t<BIter> M,
                                     ses_t<BIter>& ses) {
  return shortestEditScript(
      first1, srcOffset, N, first2, dstOffset, M, ses,
      std::equal_to<typename std::iterator_traits<BIter>::value_type>());
}
}  // namespace mydiff

#endif