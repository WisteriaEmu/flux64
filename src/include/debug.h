#ifndef __DEBUG_H_
#define __DEBUG_H_

#include <stdio.h>

#define SET_DEBUG_CHANNEL(channel) \
    static const char* _default_debug_channel = channel;

#define log_debug(msg, ...) \
    fprintf(stdout, "[DEBUG][%s]: " msg "\n", _default_debug_channel, ##__VA_ARGS__)

#define log_warn(msg, ...) \
    fprintf(stderr, "[WARN][%s]: " msg "\n", _default_debug_channel, ##__VA_ARGS__)

#define log_err(msg, ...) \
    fprintf(stderr, "[ERR][%s]: " msg "\n", _default_debug_channel, ##__VA_ARGS__)

#define log_trace(msg, ...) \
    fprintf(stdout, "[TRACE][%s][%s][%s][%d]: " msg "\n", _default_debug_channel, __FILE__, __func__, __LINE__, ##__VA_ARGS__)

#define log_fixme(msg, ...) \
    fprintf(stdout, "[FIXME][%s]: " msg "\n", _default_debug_channel, ##__VA_ARGS__)

#define log_dump(msg, ...) \
    fprintf(stdout, "[DUMP][%s]: " msg "\n", _default_debug_channel, ##__VA_ARGS__)

#endif /* __DEBUG_H_ */
