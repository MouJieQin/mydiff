#ifndef _MYERS_DIFF_H_
#define _MYERS_DIFF_H_

#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>

namespace mydiff {

enum EDIT_SCRIPT { ES_RETAIN, ES_DELETE, ES_INSERT };

template <typename Iter>
using iter_dif_t = typename std::iterator_traits<Iter>::difference_type;

template <typename BIter>
using ses_t = std::vector<std::pair<EDIT_SCRIPT, iter_dif_t<BIter>>>;

template <typename BIter, typename EqualTo>
iter_dif_t<BIter> shortestEditScript(BIter first1,
                                     const iter_dif_t<BIter> srcOffset,
                                     const iter_dif_t<BIter> N, BIter first2,
                                     const iter_dif_t<BIter> dstOffset,
                                     const iter_dif_t<BIter> M,
                                     ses_t<BIter>& ses, const EqualTo& equalTo);

template <typename BIter, typename EqualTo>
class MyersDiff {
  friend iter_dif_t<BIter> shortestEditScript<BIter, EqualTo>(
      BIter first1, const iter_dif_t<BIter> srcOffset,
      const iter_dif_t<BIter> N, BIter first2,
      const iter_dif_t<BIter> dstOffset, const iter_dif_t<BIter> M,
      ses_t<BIter>& ses, const EqualTo& equalTo);

 private:
  typedef typename std::iterator_traits<BIter>::value_type value_type;
  typedef typename std::iterator_traits<BIter>::difference_type difference_type;
  typedef difference_type diff_t;
  typedef std::pair<diff_t, diff_t> point_t;
  typedef ses_t<BIter> ses_t;

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

 private:
  MyersDiff(const diff_t maxPath) : forward(maxPath), reverse(maxPath){};

  diff_t shortestEditScript(BIter first1, const diff_t srcOffset,
                            const diff_t N, BIter first2,
                            const diff_t dstOffset, const diff_t M, ses_t& ses,
                            const EqualTo& equalTo) {
    ses_t tmpSes;
    shortestEditScriptImple(first1, srcOffset, N, first2, dstOffset, M, tmpSes,
                            equalTo);
    diff_t lcs = ((M + N) - tmpSes.size()) / 2;
    ses.swap(tmpSes);
    return lcs;
  }

 private:
  diff_t absIndex(const diff_t offset, const diff_t index) {
    return offset + (index - 1);
  }

  diff_t shortestEditScriptImple(BIter src, const diff_t srcOffset,
                                 const diff_t N, BIter dst,
                                 const diff_t dstOffset, const diff_t M,
                                 ses_t& ses, const EqualTo& equalTo) {
    if (M == 0 && N > 0) {
      for (diff_t i = 0; i < N; ++i) {
        ses.emplace_back(ES_DELETE, srcOffset + i);
      }
      return N;
    }
    if (N == 0 && M > 0) {
      for (diff_t i = 0; i < M; ++i) {
        ses.emplace_back(ES_INSERT, dstOffset + i);
      }
      return M;
    }
    if (N == 0 && M == 0) {
      return 0;
    }
    point_t head, tail;
    diff_t d = findMiddleSnake(src, srcOffset, N, dst, dstOffset, M, head, tail,
                               equalTo);
    if (d == 0) {
      for (diff_t i = head.first + 1; i <= tail.first; ++i) {
        ses.emplace_back(ES_RETAIN, absIndex(srcOffset, i));
      }
      return 0;
    } else if (d == 1) {
      diff_t xForward = 0, yForward = 0;
      BIter xIter = std::next(src, srcOffset);
      BIter yIter = std::next(dst, dstOffset);
      for (; xForward < N && yForward < M && equalTo(*(xIter++), *(yIter++));) {
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
      return tail.first - head.first;
    } else {
      diff_t first =
          shortestEditScriptImple(src, srcOffset, head.first, dst, dstOffset,
                                  head.second, ses, equalTo);
      for (diff_t i = head.first + 1; i <= tail.first; ++i) {
        ses.emplace_back(ES_RETAIN, absIndex(srcOffset, i));
      }
      diff_t last = shortestEditScriptImple(
          src, absIndex(srcOffset, tail.first + 1), N - tail.first, dst,
          absIndex(dstOffset, tail.second + 1), M - tail.second, ses, equalTo);
      return first + last + tail.first - head.first;
    }
    return 0;
  }

  diff_t findMiddleSnake(BIter src, const diff_t srcOffset, const diff_t N,
                         BIter dst, const diff_t dstOffset, const diff_t M,
                         point_t& head, point_t& tail, const EqualTo& equalTo) {
    diff_t last_x, last_y;
    diff_t x, y;
    diff_t kForward, kReverse;
    diff_t delta = N - M;
    diff_t ceilHalfD = (N + M + 1) / 2;
    forward.reset(ceilHalfD);
    reverse.reset(ceilHalfD);
    BIter xIter, yIter;
    bool odd = ((delta & 1) == 1);
    if (odd) {
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
          xIter = std::next(src, absIndex(srcOffset, x));
          yIter = std::next(dst, absIndex(dstOffset, y));
          for (; x < N && y < M && equalTo(*(++xIter), *(++yIter));) {
            x += 1;
            y += 1;
          }
          forward[k] = x;
          kReverse = delta - k;
          if (kReverse >= -(d - 1) && kReverse <= (d - 1)) {
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
          } else {
            x = reverse[k - 1] + 1;
          }
          y = x - k;
          xIter = std::next(src, absIndex(srcOffset, (N - x)));
          yIter = std::next(dst, absIndex(dstOffset, (M - y)));
          for (; x < N && y < M && equalTo(*(xIter--), *(yIter--));) {
            x += 1;
            y += 1;
          }
          reverse[k] = x;
        }
      }
    } else {
      for (diff_t d = 0; d <= ceilHalfD; ++d) {
        for (diff_t k = -d; k <= d; k += 2) {
          if (k == -d || (k != d && forward[k - 1] < forward[k + 1])) {
            x = forward[k + 1];
          } else {
            x = forward[k - 1] + 1;
          }
          y = x - k;
          xIter = std::next(src, absIndex(srcOffset, x));
          yIter = std::next(dst, absIndex(dstOffset, y));
          for (; x < N && y < M && equalTo(*(++xIter), *(++yIter));) {
            x += 1;
            y += 1;
          }
          forward[k] = x;
        }
        for (diff_t k = -d; k <= d; k += 2) {
          if (k == -d || (k != d && reverse[k - 1] < reverse[k + 1])) {
            x = reverse[k + 1];
          } else {
            x = reverse[k - 1] + 1;
          }
          y = x - k;
          last_x = x;
          last_y = y;
          xIter = std::next(src, absIndex(srcOffset, (N - x)));
          yIter = std::next(dst, absIndex(dstOffset, (M - y)));
          for (; x < N && y < M && equalTo(*(xIter--), *(yIter--));) {
            x += 1;
            y += 1;
          }
          reverse[k] = x;
          if (k >= delta - d && k <= delta + d) {
            kForward = delta - k;
            if ((N - x) <= forward[kForward]) {
              head = {N - x, M - y};
              tail = {N - last_x, M - last_y};
              return 2 * d;
            }
          }
        }
      }
    }
    return 0;
  }

 private:
  IntIndexVector forward;
  IntIndexVector reverse;
};

template <typename BIter, typename EqualTo>
iter_dif_t<BIter> shortestEditScript(BIter first1, BIter last1, BIter first2,
                                     BIter last2, ses_t<BIter>& ses,
                                     const EqualTo& comp) {
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

template <typename BIter, typename EqualTo>
iter_dif_t<BIter> shortestEditScript(
    BIter first1, const iter_dif_t<BIter> srcOffset, const iter_dif_t<BIter> N,
    BIter first2, const iter_dif_t<BIter> dstOffset, const iter_dif_t<BIter> M,
    ses_t<BIter>& ses, const EqualTo& equalTo) {
  iter_dif_t<BIter> maxPath = (N + M + 1) / 2;
  MyersDiff<BIter, EqualTo> mydiff(maxPath);
  return mydiff.shortestEditScript(first1, srcOffset, N, first2, dstOffset, M,
                                   ses, equalTo);
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