#ifndef GATTM_INT_H_
#define GATTM_INT_H_

/**
 ****************************************************************************************
 * @addtogroup GATTM_INT Task
 * @ingroup GATTM
 * @brief Internal function required for GATTM usage
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "ke_task.h"
#include "gattm_task.h"

#include "../attm/attm_db.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// number of GATT Process
#define GATTM_IDX_MAX                                 BLE_CONNECTION_MAX


/// GATT database default features
#define GATT_DB_DEFAULT_FEAT         0x0001
/// GATT database Service changed feature
#define GATT_DB_SVC_CHG_FEAT         0x000E



/// states of GATT task
enum gattm_state_id
{
    /// idle state
    GATTM_IDLE,
    /// busy state
    GATTM_BUSY,
    /// Number of defined states.
    GATTM_STATE_MAX
};



/// GATT General Information Manager
struct gattm_env_tag
{
    #if (BLE_ATTS)
    /// Environment data needed by attribute database
    struct attm_db  db;

    /// GATT service start handle
    uint16_t svc_start_hdl;
    #endif // (BLE_ATTS)
};

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */
extern struct gattm_env_tag gattm_env;


/*
 * FUNCTIONS DECLARATIONS
 ****************************************************************************************
 */



/*
 * TASK DESCRIPTOR DECLARATIONS
 ****************************************************************************************
 */


/// @} GATTM_INT
#endif // GATTM_INT_H_
