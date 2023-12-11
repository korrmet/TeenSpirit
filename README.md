# Teen Spirit

This is a quite simple SLR parser. It has no perfect performance and it has
no perfect size, but it is simple. Idea is same as in Boost.Spirit, but without
paying so many attention to BNF.

This parser no need the dynamic memory, it can be fully statically defined so
it may be useful in embedded applications.

As it's just a little brother of Boost.Spirit, only simple SLR parser,
it have not any lookahead logic, it have not any recursion at all, it can't
correctly parse even infix calculator, but it perfect for parsing configuration
files or command input.
