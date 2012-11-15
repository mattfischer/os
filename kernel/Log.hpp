#ifndef LOG_H
#define LOG_H

/*!
 * \brief Buffered log utility, accessible to userspace
 */
class Log {
public:
	static void start();
	static void puts(const char *s);
};

#endif
