# popen3 taken from https://gist.github.com/nitrogenlogic/1022231

```
/*
 * This implementation of popen3() was created from scratch in June of 2011.  It
 * is less likely to leak file descriptors if an error occurs than the 2007
 * version and has been tested under valgrind.  It also differs from the 2007
 * version in its behavior if one of the file descriptor parameters is NULL.
 * Instead of closing the corresponding stream, it is left unmodified (typically
 * sharing the same terminal as the parent process).  It also lacks the
 * non-blocking option present in the 2007 version.
 *
 * No warranty of correctness, safety, performance, security, or usability is
 * given.  This implementation is released into the public domain, but if used
 * in an open source application, attribution would be appreciated.
 *
 * Mike Bourgeous
 * https://github.com/nitrogenlogic
 */
```
