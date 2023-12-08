#include <iostream>
#include <cstring>
#include "uniparse.hpp"

// TODO: extend the example to check token converters
// TODO: check for parsing different times_mode
// BUG: odd letter ignored, false-positive command recognition
//      reason: odd letter not recognized by any of lexemes and syntax parser
//              do not receives this token. seems to be this behavior can't
//              be fixed because by design we can't catch trash by explicit
//              lexeme for it because there is no priorities. need to check it

class parser
{ public:
  parser()
  { // set up lexemes
    var_l(var_r.append('A', 'Z'), true, 3, uniparse::times_mode::equal)
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
    main_parser(space_l)(foo_l)(bar_l)(set_l)(get_l)(enable_l)(disable_l)(end_l)
               (var_l)(set_g)(get_g); }

  void operator()(uint8_t byte) { main_parser(byte); }

  private:
  typedef enum { skip = 1, command, variable, value, end } token_id_t;
  uniparse::range<1> var_r;

  uniparse::lexeme<1, 1> space_l;
  uniparse::lexeme<3, 3> foo_l;
  uniparse::lexeme<3, 3> bar_l;
  uniparse::lexeme<3, 3> set_l;
  uniparse::lexeme<3, 3> get_l;
  uniparse::lexeme<3, 3> var_l;
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

  for (char ch : std::string("get FDG\n")) { std::cout << ch; p(ch); }
  std::cout << '\n';

  for (char ch : std::string("get KKND\n")) { std::cout << ch; p(ch); }
  std::cout << '\n';
  return 0; }
