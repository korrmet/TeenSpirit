#ifndef UNIPARSE_HPP
#define UNIPARSE_HPP

#include <cstdint>
#include <limits>
#include <regex>

// TODO: implement missing lexeme rules
// TODO: add token converters

namespace uniparse {

template <typename T, unsigned int V>
class fixed_buffer
{ public:
  fixed_buffer() : count(0) {}
  fixed_buffer& reset() { count = 0; return *this; }
  fixed_buffer& append(T item)
  { if (count < V) { mem[count] = item; count++; } return *this; }

  T mem[V];
  unsigned int count; };

typedef struct
{ uint8_t* ptr;
  unsigned int size;
  unsigned int type; // 0 -> empty token
} token_t;

class basic_lexeme
{ public:
  virtual bool process(uint8_t byte) = 0;
  token_t token;
  void* obj;
  void (*callback)(token_t token, void* obj);
  unsigned int stage;
  unsigned int curr_times;
  basic_lexeme* next; };

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
      rules[count].d1 = *(uint8_t*)c_str;
      rules[count].times = 1;
      count++; c_str++; }
    rules[count].t = rule_type::no_rule;
    return *this; }

  private:
  virtual bool process(uint8_t byte) override
  { tok_buf.append(byte);
    switch (rules[stage].t)
    { case rule_type::no_rule:
      { stage = 0; curr_times = 0; tok_buf.reset();
      } return false;

      case rule_type::any:
      { curr_times++;
        if (curr_times >= rules[stage].times) { curr_times = 0; stage++; }
      } break;

      case rule_type::equal:
      { if (byte == rules[stage].d1)
        { curr_times++;
          if (curr_times >= rules[stage].times) { curr_times = 0; stage++; } }
        else { stage = 0; curr_times = 0; tok_buf.reset(); return false; }
      } break;

      default: break; }

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
  { rule_type t; uint8_t d1; uint8_t d2; unsigned int times; } rule_data_t;
  rule_data_t rules[VR]; 
  fixed_buffer<uint8_t, VB> tok_buf; };

class lexer
{ public:
  lexer() : lexemes(nullptr) {}

  void operator()(uint8_t byte)
  { basic_lexeme* iter = lexemes;
    bool triggered = false;
    while (iter)
    { if (iter->process(byte)) { triggered = true; } iter = iter->next; }
  
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

  private:
  basic_lexeme* lexemes; };

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

  private:
  basic_grammar* grammars; };

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
  { syntax(grammar);
    return *this; }

  private:
  static void lexeme_match(token_t token, void* obj)
  { parser& that = *(parser*)obj;
    that.syntax(token); }

  lexer lexicon; syntaxer syntax; };

}

#endif // UNIPARSE_HPP
