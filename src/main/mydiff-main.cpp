// #include <google/profiler.h>
#ifdef GPERF
#include <google/profiler.h>
#endif
#include <fstream>
#include "lib/mydiff/myers-diff.h"

bool tv(const std::string &file, std::vector<std::string> &vf) {
  std::ifstream inf(file);
  if (!inf.is_open()) {
    std::cerr << "open error on " << file << std::endl;
    return false;
  }
  std::string buf;
  for (; getline(inf, buf);) {
    vf.push_back(buf);
  }
  inf.close();
  return true;
}

int main(int argc, char **argv) {
#ifdef GPERF
  ProfilerStart("mydiff.prof");
#endif
  if (argc != 3) {
    std::cerr << "usage: mydiff orcfile dstfile" << std::endl;
    return 1;
  }
  std::string srcf(argv[1]);
  std::string dstf(argv[2]);
  std::vector<std::string> src, dst;
  if (!tv(srcf, src)) {
    return 1;
  }
  if (!tv(dstf, dst)) {
    return 1;
  }
  mydiff::ses_t<std::vector<std::string>::iterator> ses;

  auto lcs =
      shortestEditScript(src.begin(), src.end(), dst.begin(), dst.end(), ses);
  std::cout << "LCS: " << lcs << std::endl;
  std::cout << "SES: " << ses.size() << std::endl;
  for (const auto &p : ses) {
    if (p.first == mydiff::ES_RETAIN) {
      std::cout << "" << src[p.second] << "\n";
    } else if (p.first == mydiff::ES_DELETE) {
      // std::cout << "-" << src[p.second] << "\n";
    } else {
      // sstd::cout << "+" << dst[p.second] << "\n";
      std::cout << "" << dst[p.second] << "\n";
    }
  }
  std::cout << std::flush;

  // for (const auto &p : ses) {
  //   if (p.first == mydiff::ES_DELETE) {
  //     for (; offset < p.second.first; ++offset) {
  //       std::cout << " " << src[offset-1] << "\n";
  //     }
  //     std::cout << "-" << src[p.second.second-1] << "\n";
  //     offset += 1;
  //   } else {
  //     if (offset <= p.second.first) {
  //       for (; offset != p.second.first; ++offset) {
  //         std::cout << " " << src[offset-1] << "\n";
  //       }
  //       offset += 1;
  //     }
  //     std::cout << "+" << dst[p.second.second-1] << "\n";
  //   }
  // }
  // for (size_t offset_ = (size_t)offset; offset_ <= src.size(); ++offset_) {
  //   std::cout << src[offset_-1] << "\n";
  // }
  // std::cout << std::flush;
#ifdef GPERF
  ProfilerStop();
#endif
  return 0;
}
