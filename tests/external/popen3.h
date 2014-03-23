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

#include <unistd.h>

/*
 * Runs command in another process, with full remote interaction capabilities.
 * Be aware that command is passed to sh -c, so shell expansion will occur.
 * Writing to *writefd will write to the command's stdin.  Reading from *readfd
 * will read from the command's stdout.  Reading from *errfd will read from the
 * command's stderr.  If NULL is passed for writefd, readfd, or errfd, then the
 * command's stdin, stdout, or stderr will not be changed.  Returns the child
 * PID on success, -1 on error.
 */
pid_t popen3(char *command, int *writefd, int *readfd, int *errfd);

