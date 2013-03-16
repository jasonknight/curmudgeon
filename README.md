# Curmudgeon

> **curmudgeon** - an ill tempered, cantankerous person

Curmudgeon is web framework built entirely in C. No C++. I decided to do this for a few reasons:

* I hate slow things, after 5 years of Rails, I was pretty effing fed up.
* Everyone said it would be hard, impossible, couldn't work.
* No one, to my knowledge has ever really tried. (RAPHTERs, while cool, is not really the same)

Curmudgeon is not a wrapper around FCGI, though I intend mainly for it to be deployed that
way, it is more like a web framework that you have come to expect in the since that it
provides a powerful API for creating applications, but has the ability to abstract away
or generate the mundane repetitive boilerplate required for all applications. Especially
in C.

There is nothing fundamentally wrong with C, in fact it's a pretty simple and straight-forward
language, however most people hate it because you have to do **everything** yourself.
Curmudgeon tries to help that by doing a lot of things for you.

Curmudgeon has a *suggestion over configuration* way of doing things. There is absolutely
nothing that prevents you from doing things your way, however; there is a *suggested* structure
to your applications.

