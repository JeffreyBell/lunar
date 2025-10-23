# lunar

The first game I ever played was LUNAR.BAS on my high school's PDP-8/e, but it turns out that this was a port of the original by Jim Storer in FOCAL.  Original code is at https://www.cs.brandeis.edu/~storer/LunarLander/LunarLander.html

There is an online version of the game at https://lunar69.uber.space/

I'm starting with the C port of the original focal game as translated by Kristopher Johnson at https://github.com/kristopherjohnson/lunar-c @kristopherjohnson

My plan is to
* Get rid of the gotos
* Add wrapper to search for better results.

So far I have:
* Modernized the variable names.
* Extracted several methods.
* Added some tests.

