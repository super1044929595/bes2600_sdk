
#include "app_bt.h"

#include "app_tota_if.h"




int8_t app_tota_get_rssi_value()
{
    return app_bt_get_rssi();
}


// To adapt different chips
