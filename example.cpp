#include <iostream>
#include <cstring>
// #define TS_DEBUG
#include "teen_spirit.hpp"

// BUG: skips are not handled properly
// BUG: there is concurrency problem between lexemes in parser

class cmd_parser
{ public:
  cmd_parser()
  { // set up lexemes

    // space_l(' ',
    //         TS_EQUAL, TS_REPEAT_TIMES(1), TeenSpirit::times_mode::not_less)
    //        .token.type = token_id_t::skip;
    space_l(" ").token.type = token_id_t::skip;

    cmd_l(cmd_r.append('a', 'z'),
          TS_EQUAL, TS_REPEAT_TIMES(1), TeenSpirit::times_mode::not_less)
         .token.type = token_id_t::command;
    
    name_l(name_r.append('A', 'Z'),
           TS_EQUAL, TS_REPEAT_TIMES(1), TeenSpirit::times_mode::not_less)
          .token.type = token_id_t::variable;

    value_l('#')
           (value_r.append('0', '9').append('a', 'f').append('A', 'F'),
            TS_EQUAL, TS_REPEAT_TIMES(1), TeenSpirit::times_mode::not_less)
           .token.type = token_id_t::value;

    end_l("\n").token.type = token_id_t::end;

    // set up grammars
    set_g(token_id_t::command)
         (token_id_t::skip)
         (token_id_t::variable)
         (token_id_t::skip)
         (token_id_t::value)
         (token_id_t::end)
         .set_callback(set_var_handler, this);

    get_g(token_id_t::command)
         (token_id_t::skip)
         (token_id_t::variable)
         (token_id_t::end)
         .set_callback(get_var_handler, this);

    // register 'em all in the parser (order is important!)
    parser(cmd_l)(name_l)(value_l)(space_l)(end_l)(set_g)(get_g); }

  void operator()(uint8_t byte) { parser(byte); }

  private:
  TeenSpirit::parser parser;

  typedef enum { skip = 1, command, variable, value, end } token_id_t;
  TeenSpirit::range<1> cmd_r;
  TeenSpirit::range<2> name_r;
  TeenSpirit::range<3> value_r;

  TeenSpirit::lexeme<1, 1> space_l;
  TeenSpirit::lexeme<1, 80> cmd_l;
  TeenSpirit::lexeme<1, 80> name_l;
  TeenSpirit::lexeme<3, 80> value_l;
  TeenSpirit::lexeme<1, 1> end_l;

  TeenSpirit::grammar<4, 80> get_g;
  TeenSpirit::grammar<6, 80> set_g;

  static void set_var_handler(TeenSpirit::token_t* tokens, void* obj)
  { std::cout << "[set]"; }

  static void get_var_handler(TeenSpirit::token_t* tokens, void* obj)
  { std::cout << "[get]"; }

};

int main(int argc, char** argv)
{ cmd_parser p;

  std::string input = "set FOO #fc\n"
                      "get BAR\n";

  for (char ch : input) { std::cout << ch; p(ch); } std::cout << '\n';
  
  return 0; }
