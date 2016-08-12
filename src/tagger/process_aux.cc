#include "process_aux.hh"

#ifndef TEST_process_aux_cc

namespace finnposaux
{
  void parse_aux_field(const std::string &s, StringPairVector &lemma_dict)
  {
    size_t edge_pos = s.find("')]");

    if (edge_pos == std::string::npos)
      { throw InvalidField(); }

    parse_lemma_dict(s.substr(0,edge_pos + 3), lemma_dict);
  }

  void parse_lemma_dict(const std::string &s, StringPairVector &lemma_dict)
  {
    if (s.size() < 2)
      { throw InvalidField(); }

    if (s[0] != '[' or s[s.size() - 1] != ']')
      { throw InvalidField(); }

    std::string ss = s.substr(1).substr(0, s.size() - 2);

    size_t sep_pos;

    while ((sep_pos = ss.find("), (")) != std::string::npos)
      {
	std::string str = ss.substr(0, sep_pos + 1);
	StringPair sp;
	parse_string_pair(str, sp);
	lemma_dict.push_back(sp);
	ss = ss.substr(sep_pos + 3);
      }

    StringPair sp;
    parse_string_pair(ss, sp);
    lemma_dict.push_back(sp);
  }

  void parse_string_pair(const std::string &s, StringPair &p)
  {
    if (s.size() < 2)
      { throw InvalidField(); }

    if (s[0] != '(' or s[s.size() - 1] != ')')
      { throw InvalidField(); }

    size_t sep_pos = s.find("\', \'");
    if (sep_pos == std::string::npos)
      { throw InvalidField(); }

    std::string str1 = s.substr(1, sep_pos);
    std::string str2 = s.substr(sep_pos + 3, s.size() - sep_pos - 4);
    
    std::string res1;
    std::string res2;

    parse_string(str1,res1);
    parse_string(str2,res2);

    p.first = res1;
    p.second = res2;
  }

  void parse_string(const std::string &s, std::string &str)
  {
    if (s.size() < 2)
      { throw InvalidField(); }

    if (s[0] != '\'' or s[s.size() - 1] != '\'')
      { throw InvalidField(); }

    if (s.find(' ') != std::string::npos)
      { throw InvalidField(); }

    str = s.substr(1).substr(0, s.size() - 2);
  }
};

#else // TEST_process_aux_cc

#include <cassert>

int main(void)
{
  std::string str("'kun'");
  std::string res;
  finnposaux::parse_string(str, res);
  assert(res == "kun");

  str = "('[POS=PARTICLE]|[SUBCAT=CONJUNCTION]|[CONJ=ADVERBIAL]', 'kun')";
  finnposaux::StringPair res_pair;
  finnposaux::parse_string_pair(str, res_pair);
  finnposaux::StringPair sp("[POS=PARTICLE]|[SUBCAT=CONJUNCTION]|[CONJ=ADVERBIAL]", "kun");
  assert(res_pair == sp);

  str = "[('[POS=PARTICLE]|[SUBCAT=CONJUNCTION]|[CONJ=ADVERBIAL]', 'kun'), ('[POS=PARTICLE]', 'kun'), ('', '')]";
  finnposaux::StringPairVector res_lemma_dict;
  finnposaux::parse_lemma_dict(str, res_lemma_dict);
  finnposaux::StringPairVector lemma_dict;
  lemma_dict.push_back(finnposaux::StringPair("[POS=PARTICLE]|[SUBCAT=CONJUNCTION]|[CONJ=ADVERBIAL]", "kun"));
  lemma_dict.push_back(finnposaux::StringPair("[POS=PARTICLE]", "kun"));
  lemma_dict.push_back(finnposaux::StringPair("", ""));
  assert(lemma_dict == res_lemma_dict);

  str = "[('[POS=PARTICLE]|[SUBCAT=CONJUNCTION]|[CONJ=ADVERBIAL]', 'kun'), ('[POS=PARTICLE]', 'kun'), ('', '')] and useless crap";
  res_lemma_dict.clear();
  finnposaux::parse_aux_field(str, res_lemma_dict);
  assert(res_lemma_dict == lemma_dict);
}

#endif // TEST_process_aux_cc
