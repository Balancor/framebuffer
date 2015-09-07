#include <sys/cdefs.h>
#include <stdarg.h>

#define LOG_DIRE "logs"
#define LOG_FILE "running.log"

struct log_info{
    int logFd;
    int logOpened;
};

struct log_info logInfo;

void log_init(void);
int  log_get_level(void);
void log_set_level(int level);
void log_write(int level, const char* fmt, ...) __attribute__((format(printf, 2, 3)));
void log_vwrite(int level, const char* fmt, va_list ap);

#define LOG_ERROR_LEVEL     3
#define LOG_WARNING_LEVEL   4
#define LOG_NOTICE_LEVEL    5
#define LOG_INFO_LEVEL      6
#define LOG_DEBUG_LEVEL     7

#define LOG_DEFAULT_LEVEL 7

#define LOG_ERROR(tag, x...)    log_write(LOG_ERROR_LEVEL, tag ": " x)
#define LOG_WARNING(tag, x...)  log_write(LOG_WARNING_LEVEL, tag ": " x)
#define LOG_NOTICE(tag, x...)   log_write(LOG_NOTICE_LEVEL, tag ": " x)
#define LOG_INFO(tag, x...)     log_write(LOG_INFO_LEVEL, tag ": " x)
#define LOG_DEBUG(tag, x...)    log_write(LOG_DEBUG_LEVEL, tag ": " x)

#define ERROR(x...)         LOG_ERROR("shape", x)
#define INFO(x...)          LOG_INFO("shape", x)
#define NOTICE(x...)        LOG_NOTICE("shape", x)
