#include <iostream>
#include <cstring>
#include "teen_spirit.hpp"

class parser
{ public:
  parser()
  { // set up lexemes
    var_l(var_r.append('A', 'Z'), true, 1, TeenSpirit::times_mode::not_less)
                        .token.type = token_id_t::variable;
    space_l(" ")        .token.type = token_id_t::skip;
    foo_l("foo")        .token.type = token_id_t::variable;
    bar_l("bar")        .token.type = token_id_t::variable;
    set_l("set")        .token.type = token_id_t::command;
    get_l("get")        .token.type = token_id_t::command;
    enable_l("enable")  .token.type = token_id_t::value;
    disable_l("disable").token.type = token_id_t::value;
    end_l("\n")         .token.type = token_id_t::end;

    // set up grammars
    set_g(token_id_t::command)
         (token_id_t::skip)
         (token_id_t::variable)
         (token_id_t::skip)
         (token_id_t::value)
         (token_id_t::end)
         .set_callback(set_var_handler);

    get_g(token_id_t::command)
         (token_id_t::skip)
         (token_id_t::variable)
         (token_id_t::end)
         .set_callback(get_var_handler);

    // register 'em all
    // WARNING: order is important!
    main_parser(space_l)(foo_l)(bar_l)(set_l)(get_l) (enable_l) (disable_l)
               (var_l)(end_l)
               (set_g)(get_g); }

  void operator()(uint8_t byte) { main_parser(byte); }

  private:
  typedef enum { skip = 1, command, variable, value, end } token_id_t;
  TeenSpirit::range<1> var_r;

  TeenSpirit::lexeme<1, 1> space_l;
  TeenSpirit::lexeme<3, 3> foo_l;
  TeenSpirit::lexeme<3, 3> bar_l;
  TeenSpirit::lexeme<3, 3> set_l;
  TeenSpirit::lexeme<3, 3> get_l;
  TeenSpirit::lexeme<3, 9> var_l;
  TeenSpirit::lexeme<6, 6> enable_l;
  TeenSpirit::lexeme<7, 7> disable_l;
  TeenSpirit::lexeme<1, 1> end_l;

  TeenSpirit::grammar<4, 80> get_g;
  TeenSpirit::grammar<6, 80> set_g;

  TeenSpirit::parser main_parser;

  static void set_var_handler(TeenSpirit::token_t* tokens, void* obj)
  { std::cout << "[set]"; }

  static void get_var_handler(TeenSpirit::token_t* tokens, void* obj)
  { std::cout << "[get ";
    std::cout << tokens[2].size << " ";
    for (unsigned int i = 0; i < tokens[2].size; i++)
    { std::cout << (char)tokens[2].ptr[i]; }
    std::cout << ']'; } };

int main(int argc, char** argv)
{ std::cout << "Example of using uniparse library\n"
               "Uniparse is the simple, but not so efficient replace for "
               "tools like re2c, lex, Boost Spirit and others. "
               "It may be used for rapid development of simple parsers.\n";

  parser p;

  for (char ch : std::string("set foo enable\n")) { std::cout << ch; p(ch); }
  std::cout << '\n';

  for (char ch : std::string("get foo\n")) { std::cout << ch; p(ch); }
  std::cout << '\n';

  for (char ch : std::string("get FDG\n")) { std::cout << ch; p(ch); }
  std::cout << '\n';

  for (char ch : std::string("get KKND\n")) { std::cout << ch; p(ch); }
  std::cout << '\n';

  for (char ch : std::string("get wtf\n")) { std::cout << ch; p(ch); }
  std::cout << '\n';
  
  return 0; }
