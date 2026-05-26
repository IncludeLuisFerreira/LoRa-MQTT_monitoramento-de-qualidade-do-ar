#include "Package_handle.h"

#include <stdint.h>
#include <stdbool.h>

bool is_valid_package_size(uint8_t packageSize) {
    return (packageSize == DATA_SENSOR_PK) || (packageSize == CHANGE_PARAM_PK);
}