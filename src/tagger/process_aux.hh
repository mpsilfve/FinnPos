#ifndef HEADER_process_aux_hh
#define HEADER_process_aux_hh

#include <utility>
#include <string>
#include <vector>
#include <map>

namespace finnposaux
{
  typedef std::pair<std::string, std::string> StringPair;
  typedef std::vector<StringPair> StringPairVector;

  struct InvalidField
  {};
  
  void parse_aux_field(const std::string &s, StringPairVector &lemma_dict);
  void parse_lemma_dict(const std::string &s, StringPairVector &lemma_dict);
  void parse_string_pair(const std::string &s, StringPair &p);
  void parse_string(const std::string &s, std::string &str);
}

#endif // HEADER_process_aux_hh
