/**
 * param.h - Configuration parameters for tx-manager
 */

#ifndef TX_PARAM_H
#define TX_PARAM_H

/* Default maximum active transactions */
#define TX_DEFAULT_MAX_ACTIVE_TXS 64

/* Initial write set capacity */
#define TX_WRITE_SET_INIT_CAPACITY 16

/* Version key separator */
#define TX_VERSION_SEP '\x00'

/* WAL sync on commit by default */
#define TX_DEFAULT_SYNC_ON_COMMIT true

#endif /* TX_PARAM_H */
