#include <iostream>
#include <cstring>
#include "uniparse.hpp"

// TODO: extend the example to check token converters
// TODO: extend the example to check complex lexeme behavior

class parser
{ public:
  parser()
  { space_l(" ")        .token.type = token_id_t::skip;
    foo_l("foo")        .token.type = token_id_t::variable;
    bar_l("bar")        .token.type = token_id_t::variable;
    set_l("set")        .token.type = token_id_t::command;
    get_l("get")        .token.type = token_id_t::command;
    enable_l("enable")  .token.type = token_id_t::value;
    disable_l("disable").token.type = token_id_t::value;
    end_l("\n")         .token.type = token_id_t::end;

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

    main_parser(space_l)(foo_l)(bar_l)(set_l)(get_l)(enable_l)(disable_l)(end_l)
               (set_g)(get_g); }

  void operator()(uint8_t byte) { main_parser(byte); }

  private:
  typedef enum { skip = 1, command, variable, value, end } token_id_t;
  uniparse::lexeme<1, 1> space_l;
  uniparse::lexeme<3, 3> foo_l;
  uniparse::lexeme<3, 3> bar_l;
  uniparse::lexeme<3, 3> set_l;
  uniparse::lexeme<3, 3> get_l;
  uniparse::lexeme<6, 6> enable_l;
  uniparse::lexeme<7, 7> disable_l;
  uniparse::lexeme<1, 1> end_l;

  uniparse::grammar<4, 80> get_g;
  uniparse::grammar<6, 80> set_g;

  uniparse::parser main_parser;

  static void set_var_handler(uniparse::token_t* tokens, void* obj)
  { std::cout << "[set]"; }

  static void get_var_handler(uniparse::token_t* tokens, void* obj)
  { std::cout << "[get]"; } };

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
  return 0; }
