#ifndef TEEN_SPIRIT_HPP
#define TEEN_SPIRIT_HPP

#include <cstdint>
#include <limits>

#ifdef TS_DEBUG
#include <cstdio>
#define TOKEN_PRINT(tok) std::printf("<%d/%d>", tok.type, tok.size)
#else
#define TOKEN_PRINT(tok)
#endif

namespace TeenSpirit {

class basic_range
{ public:
  typedef struct { uint8_t start; uint8_t end; } pair;
  virtual pair operator[](unsigned int pos) = 0;
  virtual unsigned int size() = 0; };

template <unsigned int V>
class range : public basic_range
{ public:
  range() : count(0) {}

  virtual pair operator[](unsigned int pos) override
  { if (pos >= count || pos >= V) { pair p = { 0, 0 }; return p; }
    return pairs[pos]; }

  virtual unsigned int size() override { return count; }

  range& append(uint8_t start, uint8_t end)
  { pairs[count].start = start; pairs[count].end = end; count++;
    return *this; }

  private: pair pairs[V]; unsigned int count; };

template <typename T, unsigned int V>
class fixed_buffer
{ public:
  fixed_buffer() : count(0) {}
  fixed_buffer& reset() { count = 0; return *this; }
  fixed_buffer& append(T item)
  { if (count < V) { mem[count] = item; count++; } return *this; }
  fixed_buffer& pop() { count--; return *this; }

  T mem[V];
  unsigned int count; };

typedef struct
{ uint8_t* ptr;
  unsigned int size;
  unsigned int type; // 0 -> empty token
} token_t;

namespace ascii {
unsigned int pow(unsigned int base, unsigned int pow)
{ unsigned int result = 1;
  for (unsigned int i = 0; i < pow; i++) { result *= base; }
  return result; }

unsigned int hex2int(token_t& tok)
{ if (!tok.size || !tok.ptr) { return 0; }
  unsigned int result = 0;
  for (unsigned int i = 0; i < tok.size; i++)
  { uint8_t num = 0;
    switch (tok.ptr[i])
    { case '0': num = 0x00; break;
      case '1': num = 0x01; break;
      case '2': num = 0x02; break;
      case '3': num = 0x03; break;
      case '4': num = 0x04; break;
      case '5': num = 0x05; break;
      case '6': num = 0x06; break;
      case '7': num = 0x07; break;
      case '8': num = 0x08; break;
      case '9': num = 0x09; break;
      case 'a': num = 0x0a; break;
      case 'b': num = 0x0b; break;
      case 'c': num = 0x0c; break;
      case 'd': num = 0x0d; break;
      case 'e': num = 0x0e; break;
      case 'f': num = 0x0f; break;
      case 'A': num = 0x0a; break;
      case 'B': num = 0x0b; break;
      case 'C': num = 0x0c; break;
      case 'D': num = 0x0d; break;
      case 'E': num = 0x0e; break;
      case 'F': num = 0x0f; break;
      default: return 0; }
      result += num * pow(2, 4 * (tok.size - i - 1)); }
  return result; }
}

class basic_lexeme
{ public:
  virtual bool process(uint8_t byte) = 0;
  token_t token;
  void* obj;
  void (*callback)(token_t token, void* obj);
  unsigned int stage;
  unsigned int curr_times;
  basic_lexeme* next; };

enum class times_mode { equal, not_more, not_less };

template <unsigned int VR, unsigned int VB>
class lexeme : public basic_lexeme
{ public:
  lexeme(void (*callback)(token_t token, void* obj) = nullptr,
         unsigned int token_type = 0,
         void* obj = nullptr)
  { stage = 0;
    curr_times = 0;
    token.type = token_type;
    token.ptr = nullptr;
    token.size = 0;
    tok_buf.reset();
    this->callback = callback;
    this->obj = obj;
    for (rule_data_t& r : rules) { r.t = rule_type::no_rule; }
    next = nullptr; }

  lexeme& set_callback(void (*callback)(token_t token, void* obj),
                       unsigned int token_type = 0, 
                       void* obj = nullptr)
  { token.type = token_type;
    token.ptr = nullptr;
    token.size = 0;
    this->obj = obj;
    this->callback = callback;
    return* this; }

  lexeme& operator()(const char* c_str)
  { unsigned int count = 0;
    while (*c_str)
    { rules[count].t = rule_type::equal;
      rules[count].d = *(uint8_t*)c_str;
      rules[count].times = 1;
      rules[count].mode = times_mode::equal;
      count++; c_str++; }
    rules[count].t = rule_type::no_rule;
    return *this; }

#define TS_EQUAL true
#define TS_NEQUAL false
#define TS_REPEAT_TIMES(num) num
  lexeme& operator()(uint8_t data, bool equal = true, unsigned int times = 1,
                     times_mode mode = times_mode::equal)
  { unsigned int count = 0;
    bool found = false;
    for (unsigned int i = 0; i < VB; i++)
    { if (rules[i].t == rule_type::no_rule)
      { found = true; count = i; break; } }

    if (!found) { return *this; }

    rules[count].d = data;
    rules[count].t = equal ? rule_type::equal : rule_type::not_equal;
    rules[count].times = times;
    rules[count].mode = mode;

    return *this; }

  lexeme& operator()(basic_range& r, bool equal = true, unsigned int times = 1,
                     times_mode mode = times_mode::equal)
  { unsigned int count = 0;
    bool found = false;
    for (unsigned int i = 0; i < VB; i++)
    { if (rules[i].t == rule_type::no_rule)
      { found = true; count = i; break; } }

    if (!found) { return *this; }

    rules[count].r = &r;
    rules[count].t = equal ? rule_type::range : rule_type::not_range;
    rules[count].times = times;
    rules[count].mode = mode;

    return *this; }

  lexeme& any(unsigned int times = 1, times_mode mode = times_mode::equal)
  { unsigned int count = 0;
    bool found = false;
    for (unsigned int i = 0; i < VB; i++)
    { if (rules[i].t == rule_type::no_rule)
      { found = true; count = i; break; } }

    if (!found) { return *this; }

    rules[count].t = rule_type::any;
    rules[count].times = times;
    rules[count].mode = mode;

    return *this; }

  private:
  void reset_parsing() { stage = 0; curr_times = 0; tok_buf.reset(); }
  
  void next_stage() { curr_times = 0; stage++; }
  
  bool check_range(uint8_t byte)
  { for (unsigned int i = 0; i < rules[stage].r->size(); i++)
    { basic_range::pair p = (*rules[stage].r)[i];
      if (byte >= p.start && byte <= p.end) { return true; } }
    return false; }

  void switch_stage(bool check_result)
  { if (check_result)
    { curr_times++;
      
      if (rules[stage].mode == times_mode::equal)
      { if (curr_times < rules[stage].times) { /* do nothing */ }
        else if (curr_times == rules[stage].times) { next_stage(); }
        else { reset_parsing(); } }

      else if (rules[stage].mode == times_mode::not_less)
      { if (curr_times < rules[stage].times) { /* do nothing */ }
        else { /* do nothing */ } }
      
      else if (rules[stage].mode == times_mode::not_more)
      { if (curr_times > rules[stage].times) { reset_parsing(); }
        else { /* do nothing */ } }
      
      else { reset_parsing(); } }
    else
    { if (rules[stage].mode == times_mode::equal) { reset_parsing(); }
      
      else if (rules[stage].mode == times_mode::not_more)
      { if (curr_times > rules[stage].times) { reset_parsing(); }
        else { if (curr_times) { tok_buf.pop(); next_stage(); }
               else { reset_parsing(); } } }
      
      else if (rules[stage].mode == times_mode::not_less)
      { if (curr_times < rules[stage].times) { reset_parsing(); }
        else { tok_buf.pop(); next_stage(); } }
      
      else { reset_parsing(); } } }

  virtual bool process(uint8_t byte) override
  { tok_buf.append(byte);

    switch (rules[stage].t)
    { case rule_type::any:       switch_stage(true);                   break;
      case rule_type::equal:     switch_stage(byte == rules[stage].d); break;
      case rule_type::not_equal: switch_stage(byte != rules[stage].d); break;
      case rule_type::range:     switch_stage(check_range(byte));      break;
      case rule_type::not_range: switch_stage(!check_range(byte));     break;
      default: reset_parsing(); return false; }

    if (stage >= VR || rules[stage].t == rule_type::no_rule)
    { curr_times = 0;
      stage = 0;
      if (callback)
      { token.ptr = tok_buf.mem; token.size = tok_buf.count;
        callback(token, obj); }
      tok_buf.reset();
      return true; }
    
    return false; }

  enum class rule_type { no_rule, any, equal, not_equal, range, not_range };
  typedef struct
  { rule_type t; uint8_t d; basic_range* r; unsigned int times;
    times_mode mode; } rule_data_t;
  rule_data_t rules[VR]; 
  fixed_buffer<uint8_t, VB> tok_buf; };

class lexer
{ public:
  lexer() : lexemes(nullptr) {}

  void operator()(uint8_t byte)
  { basic_lexeme* iter = lexemes;
    bool triggered = false;
    
    while (iter)
    { if (iter->process(byte)) { triggered = true; }
      iter = iter->next; }

    if (triggered)
    { iter = lexemes;
      while (iter)
      { iter->stage = 0; iter->curr_times = 0; iter = iter->next; } } }
  
  lexer& operator()(basic_lexeme& l)
  { if (!lexemes) { lexemes = &l; return *this; }

    basic_lexeme* last = lexemes;
    while (last->next) { last = last->next; }
    last->next = &l;

    return *this; }

  private: basic_lexeme* lexemes; };

class basic_grammar
{ public:
  virtual bool process(token_t tok) = 0;
  unsigned int stage;
  void* obj;
  void (*callback)(token_t* tokens, void* obj);
  basic_grammar* next; };

template <unsigned int VT, unsigned int VB>
class grammar : public basic_grammar
{ public:
  grammar(void (*callback)(token_t* tokens, void* obj) = nullptr,
          void* obj = nullptr)
  { tok_buf.reset();
    stage = 0;
    this->callback = callback;
    this->obj = obj;
    for (token_t& t : tokens) { t.type = 0; }
    next = nullptr; }

  grammar& set_callback(void (*callback)(token_t* tokens, void* obj),
                        void* obj = nullptr)
  { this->obj = obj;
    this->callback = callback;
    return* this; }

  grammar& operator()(unsigned int type)
  { for (unsigned int i = 0; i < VT; i++)
    { if (!tokens[i].type) { tokens[i].type = type; break; } }
    return *this; }

  private:
  virtual bool process(token_t tok) override
  { if (tokens[stage].type == tok.type)
    { tokens[stage].type = tok.type;
      tokens[stage].size = tok.size;
      tokens[stage].ptr = &tok_buf.mem[tok_buf.count];
      for (unsigned int i = 0; i < tok.size; i++)
      { tok_buf.append(tok.ptr[i]); }
      stage++;
      if (stage >= VT || !tokens[stage].type)
      { if (callback) { callback(tokens, obj); stage = 0; return true; } } }
    else { stage = 0; tok_buf.reset(); }
    return false; }

  token_t tokens[VT];
  fixed_buffer<uint8_t, VB> tok_buf; };

class syntaxer
{ public:
  syntaxer() : grammars(nullptr) {}

  void operator()(token_t token)
  { basic_grammar* iter = grammars;
    bool triggered = false;
    while (iter)
    { if (iter->process(token)) { triggered = true; } iter = iter->next; }
  
    if (triggered) 
    { iter = grammars;
      while (iter) { iter->stage = 0; iter = iter->next; } } }

  syntaxer& operator()(basic_grammar& g)
  { if (!grammars) { grammars = &g; return *this; }

    basic_grammar* last = grammars;
    while (last->next) { last = last->next; }
    last->next = &g;

    return *this; }

  private: basic_grammar* grammars; };

class parser
{ public:
  parser() {}

  parser& operator()(uint8_t byte) { lexicon(byte); return *this; }

  parser& operator()(basic_lexeme& lexeme)
  { lexeme.callback = lexeme_match;
    lexeme.obj = this;
    lexicon(lexeme);
    return *this; }
  
  parser& operator()(basic_grammar& grammar)
  { syntax(grammar); return *this; }

  private:
  static void lexeme_match(token_t token, void* obj)
  { TOKEN_PRINT(token); parser& that = *(parser*)obj; that.syntax(token); }

  lexer lexicon; syntaxer syntax; };

}

#endif // TEEN_SPIRIT_HPP
