#ifndef LOGGING_H_INCLUDED
#define LOGGING_H_INCLUDED

#include "GLogger.h"

#define TRACE(...) GLogger::spam(__VA_ARGS__)
#define CHATTER(...) GLogger::chatter(__VA_ARGS__)
#define INFO(...) GLogger::info(__VA_ARGS__)
#define WARN(...) GLogger::warning(__VA_ARGS__)
#if defined(ERROR)
#undef ERROR
#endif
#define ERROR(...) GLogger::error(__VA_ARGS__)
#define FATAL(s,...) \
  GLogger::critical("%s(%d): " s, __FILE__, __LINE__, ##__VA_ARGS__)

#define F_TRACE(s,...) \
  GLogger::spam("%s(%d): " s, __FILE__, __LINE__, ##__VA_ARGS__)
#define F_CHATTER(s,...) \
  GLogger::chatter("%s(%d): " s, __FILE__, __LINE__, ##__VA_ARGS__)
#define F_INFO(s,...) \
  GLogger::info("%s(%d): " s, __FILE__, __LINE__, ##__VA_ARGS__)
#define F_WARN(s,...) \
  GLogger::warning("%s(%d): " s, __FILE__, __LINE__, ##__VA_ARGS__)
#define F_ERROR(s,...) \
  GLogger::error("%s(%d): " s, __FILE__, __LINE__, ##__VA_ARGS__)
#define F_FATAL(s,...) \
  GLogger::critical("%s(%d): " s, __FILE__, __LINE__, ##__VA_ARGS__)


#ifdef _MSC_VER

#define P_TRACE(s,...) \
  GLogger::spam("%s(%d): " s, __FUNCSIG__, __LINE__, ##__VA_ARGS__)
#define P_CHATTER(s,...) \
  GLogger::chatter("%s(%d): " s, __FUNCSIG__, __LINE__, ##__VA_ARGS__)
#define P_INFO(s,...) \
  GLogger::info("%s(%d): " s, __FUNCSIG__, __LINE__, ##__VA_ARGS__)
#define P_WARN(s,...) \
  GLogger::warning("%s(%d): " s, __FUNCSIG__, __LINE__, ##__VA_ARGS__)
#define P_ERROR(s,...) \
  GLogger::error("%s(%d): " s, __FUNCSIG__, __LINE__, ##__VA_ARGS__)
#define P_FATAL(s,...) \
  GLogger::critical("%s(%d): " s, __FUNCSIG__, __LINE__, ##__VA_ARGS__)

#else //  _MSC_VER

#define P_TRACE(s,...) \
  GLogger::spam("%s(%d): " s, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)
#define P_CHATTER(s,...) \
  GLogger::chatter("%s(%d): " s, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)
#define P_INFO(s,...) \
  GLogger::info("%s(%d): " s, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)
#define P_WARN(s,...) \
  GLogger::warning("%s(%d): " s, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)
#define P_ERROR(s,...) \
  GLogger::error("%s(%d): " s, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)
#define P_FATAL(s,...) \
  GLogger::critical("%s(%d): " s, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)

#endif //  _MSC_VER

#endif // LOGGING_H_INCLUDED

