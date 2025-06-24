#ifndef KEYDEF_H
#define KEYDEF_H

#include <QString>

const QString CFG_GROUP_PROGRAM = "Program";
const QString CFG_PROGRAM_LANGUAGE = "language";
const QString CFG_PROGRAM_THEME = "theme";
const QString CFG_GROUP_SERIAL = "Serial";
const QString CFG_SERIAL_PORT_NAME = "port_name";
const QString CFG_SERIAL_BAUD_RATE = "baud_rate";
const QString CFG_SERIAL_SEND_FORMAT = "serial_send_format";
const QString VAL_SERIAL_SEND_NORMAL = "normal";
const QString VAL_SERIAL_SEND_HEX = "hex";
const QString VAL_SERIAL_SEND_HEX_TRANSLATE = "hex_translate";
const QString CFG_SERIAL_SHOW_SEND = "show_send";
const QString CFG_SERIAL_HEX_DISPLAY = "hex_display";
const QString CFG_SERIAL_DEBUG_PORT = "debug_port";
const QString CFG_SERIAL_CYCLE = "cycle";
const QString CFG_SERIAL_SEND_PAGE = "send_page";
const QString VAL_PAGE_SINGLE = "single";
const QString VAL_PAGE_MULTIPE = "multipe";
const QString VAL_ENABLE = "enable";
const QString VAL_DISABLE = "disable";
const QString CFG_GROUP_HISTROY = "History";
const QString CFG_HISTORY_SINGLE_SEND = "single_send";
const QString CFG_HISTORY_MULT = "mult";
const QString CFG_MULT_LABEL = "label";
const QString CFG_GROUP_SETTING = "Setting";
const QString CFG_SETTING_UPDATE_URL = "update_url";
const QString CFG_SETTING_UPDATE_CHECK = "update_check";
const QString CFG_SETTING_UPDATE_TIP = "update_tip";
const QString CFG_GROUP_SERVER = "Server";
const QString CFG_SERVER_ENABLE = "enable";
const QString CFG_SERVER_PORT = "port";
const QString CFG_SERVER_LOG = "log";
const QString CFG_GROUP_SIMULATE = "Simulate";
const QString CFG_SIMULATE_HEAD = "head";
const QString CFG_SIMULATE_TAIL = "tail";
const QString CFG_SIMULATE_FILE = "file";
const QString CFG_GROUP_PLOT = "Plot";
const QString CFG_PLOT_OFFSET14 = "offset14";
const QString CFG_PLOT_OFFSET24 = "offset24";
const QString CFG_PLOT_ALGORITHM = "algorithm";
const QString CFG_PLOT_FITTING_K = "fitting_k";
const QString CFG_PLOT_FITTING_B = "fitting_b";
const QString CFG_PLOT_FITTING_CONVERSION = "fitting_conversion";
const QString CFG_GROUP_DATA = "Data";
const QString CFG_DATA_LIMIT = "limit";
const QString CFG_GROUP_CORRECTION = "Correction";
const QString CFG_CORRECTION_ALGORITHM = "algorithm";
const QString CFG_CORRECTION_ROUND = "round";
const QString FRAME_HEADER = "header";
const QString FRAME_FOOTER = "footer";
#endif
