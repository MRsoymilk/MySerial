#ifndef FUNCDEF_H
#define FUNCDEF_H
#include "keydef.h"
// FUNCTION ===================================================================

#include "../common/mysetting.h"
#define SETTING_GET(group, key, ...) MY_SETTING.getValue(group, key, ##__VA_ARGS__)
#define SETTING_SET(group, key, value) MY_SETTING.setValue(group, key, value)

// FUNCTION ===================================================================

#endif
