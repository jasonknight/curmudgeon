
# Curmudgeon

> **curmudgeon** - an ill tempered, cantankerous person

Curmudgeon is web framework built entirely in C. No C++. I decided to do this for a few reasons:

* I hate slow things, after 5 years of Rails, I was pretty effing fed up.
* Everyone said it would be hard, impossible, couldn't work.
* No one, to my knowledge has ever really tried. (RAPHTERs, while cool, is not really the same)

Curmudgeon is not a wrapper around FCGI, though I intend mainly for it to be deployed that
way, it is more like a web framework that you have come to expect in the sense that it
provides a powerful API for creating applications, but has the ability to abstract away
or generate the mundane repetitive boilerplate required for all applications. Especially
in C.

There is nothing fundamentally wrong with C, in fact it's a pretty simple and straight-forward
language, however; most people hate it because you have to do **everything** yourself.
Curmudgeon tries to help that by doing a lot of things for you.

Curmudgeon has a *suggestion over configuration* way of doing things. There is absolutely
nothing that prevents you from doing things your way, however; there is a *suggested* structure
to your applications.

# How it works

The basic idea is similar to what you would find in a nodejs app. Curmudgeon is built around the idea
of events and handlers, but not in the libev sense of the word. Curmudgeon aims to be as dependency free
as humanly possible. Ideally you would statically compile the application and deploy it on a server
without much care of what is there besides libc and a server that supports Fast CGI.

Currently, Curmudgeon only depends on libc in static mode, and pcre and other general C libs.

```c
cur_register_event(&app,"hello_world.*", 0,hello_world);
// or
cur_register_event(&app,"/hello_world\\/(?P<name>.*)/i", 0,hello_world);
```

Curmudgeon supports PCRE handler definitions including named captures. Still a bit experimental, the PCRE handling
code is currently the largest most complex part of the entire framework. PCRE is a scurry scurry thing,
but it's very fast, and people are used to Perl style regex.
