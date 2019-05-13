#include <fstream>
#include "lib/myersdiff/myers-diff.h"

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
  int offset = 0;
  for (const auto &p : ses) {
    if (p.first == mydiff::ES_DELETE) {
      for (; offset != p.second; ++offset) {
        std::cout << "  " << src[offset] << std::endl;
      }
      std::cout << "- " << src[p.second] << std::endl;
      offset += 1;
    } else {
      std::cout << "+ " << dst[p.second] << std::endl;
      offset += 1;
    }
  }
  for (; offset != src.size(); ++offset) {
    std::cout << "  " << src[offset] << std::endl;
  }
  return 0;
}
