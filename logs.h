#define LOG_FILE "logs/running.log"

struct log_info{
    int logFd;
    int logOpened;
};

struct log_info logInfo = {
    -1,
    0
};

void log_init();
void log_v(const char* msg);
void log_i(const char* msg);
void log_d(const char* msg);
void log_w(const char* msg);
void log_e(const char* msg);
void log_close();
