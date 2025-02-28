#ifndef UTILS_MODULE_H
#define UTILS_MODULE_H

#define CHECK_ERROR(rc, msg) \
    if ((rc) != ESP_OK) { \
        ESP_LOGE(TAG, "%s, error code: %d", (msg), (rc)); \
        return rc; \
    }

#define SECONDS_FRACTIONS_TO_US(fractions) ((fractions) * 1000000 / 256)
#define US_TO_MS(ms) ((ms) / 1000)
#define MS_TO_US(ms) ((ms) * 1000)
#define SECONDS_TO_US(ms) ((ms) * 1000 * 1000)

#endif
