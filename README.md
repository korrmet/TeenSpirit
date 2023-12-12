# Teen Spirit

Simple LR(1) parser for embedded systems.

TeenSpirit is a simple LR(1) parser written in C++ that's inspired by Boost.Spirit. It doesn't require any dynamic memory allocation and can be used in embedded applications. The library is header-only. The programmer has full control over memory management.

The library can also be used to parse command input or configuration files.

The library doesn't support complex grammars like infix notation or type inference, so you may need to use a different parser for those cases. However, it provides a simple and easy-to-use interface for parsing text or even binary streams and can be a good starting point for many applications.
