# faststrings
Fast Safe C Strings and Record IO

Keeping the current length of C strings separate from the string itself, instead of utilising
either a binary zero or a carriage return or line feed at the end of a string, reduces
unnecessary work.

Repeated testing indicatess that the CPU reduction or speed increases are approximately 4 times
over the standard strlen and strcpy C functions.

And the Record Input/Output routines aare similar to IBM ZOS, and offer speeds about 4 times
faster than fgets and similar routines.

These routines stop the accidental overwriting of storage or buffer overruns in C and C++.

Additionally, power ussaage is reduced.
