# uniparse

Main idea of this parser is make it useful in a large class of devices. 
It's not planned for high performance, just simple replacement for Boost.Spirit
for a simple applications like parsing configuration files or text user
interfaces. As Boost.Spirit it's header-only library, but it may be used even
on a bare metal projects without any kind of dynamic memory.

It's just an SLR parser engine, so you can implement simple left-recursive
grammars only without any lookaheads. For example, there is no way to implement
calculator with braces. But RPN is okay, because it's left-recursive itself.

# Warning

I'm really don't interested in this repository at present moment, maybe it
would be abandoned.
