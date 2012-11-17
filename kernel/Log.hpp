#ifndef LOG_H
#define LOG_H

/*!
 * \brief Buffered log utility, accessible to userspace
 */
class Log {
public:
	static void start();
	static void printf(const char *s, ...);
	static void puts(const char *s);
	static void write(const char *s, int size);
};

#endif
