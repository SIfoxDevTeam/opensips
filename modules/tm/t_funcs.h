/*
 * $Id$
 *
 * Copyright (C) 2001-2003 FhG Fokus
 *
 * This file is part of opensips, a free SIP server.
 *
 * opensips is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version
 *
 * opensips is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * History:
 * --------
 *  2003-02-18  updated various function prototypes (andrei)
 *  2003-03-10  removed ifdef _OBSO & made redefined all the *UNREF* macros
 *               in a non-gcc specific way (andrei)
 *  2003-03-13  now send_pr_buffer will be called w/ function/line info
 *               only when compiling w/ -DEXTRA_DEBUG (andrei)
 *  2003-03-31  200 for INVITE/UAS resent even for UDP (jiri)
 *  2007-01-25  DNS failover at transaction level added (bogdan)
 */



#ifndef _T_FUNCS_H
#define _T_FUNCS_H

#include <errno.h>
#include <netdb.h>

#include "../../mem/shm_mem.h"
#include "../../parser/msg_parser.h"
#include "../../globals.h"
#include "../../udp_server.h"
#include "../../msg_translator.h"
#include "../../timer.h"
#include "../../forward.h"
#include "../../mem/mem.h"
#include "../../md5utils.h"
#include "../../ip_addr.h"
#include "../../parser/parse_uri.h"
#include "../../usr_avp.h"

struct s_table;
struct timer;
struct entry;
struct cell;
struct retr_buf;

#include "t_lookup.h"
#include "config.h"
#include "lock.h"
#include "timer.h"
#include "sip_msg.h"
#include "h_table.h"
#include "ut.h"

extern int noisy_ctimer;


/* t_reply_to flags */
#define TM_T_REPLY_repl_FLAG     (1<<0)
#define TM_T_REPLY_no100_FLAG    (1<<1)
#define TM_T_REPLY_noerr_FLAG    (1<<2)
#define TM_T_REPLY_nodnsfo_FLAG  (1<<3)
#define TM_T_REPLY_reason_FLAG   (1<<4)


/* send a private buffer: utilize a retransmission structure
   but take a separate buffer not referred by it; healthy
   for reducing time spend in REPLIES locks
*/


/* send a buffer -- 'PR' means private, i.e., it is assumed noone
   else can affect the buffer during sending time
*/
#ifdef EXTRA_DEBUG
int send_pr_buffer( struct retr_buf *rb,
	void *buf, int len, char* file, const char *function, int line );
#define SEND_PR_BUFFER(_rb,_bf,_le ) \
	send_pr_buffer( (_rb), (_bf), (_le), __FILE__,  __FUNCTION__, __LINE__ )
#else
int send_pr_buffer( struct retr_buf *rb, void *buf, int len);
#define SEND_PR_BUFFER(_rb,_bf,_le ) \
	send_pr_buffer( (_rb), (_bf), (_le))
#endif

#define SEND_BUFFER( _rb ) \
	SEND_PR_BUFFER( (_rb) , (_rb)->buffer.s , (_rb)->buffer.len )


#define UNREF_UNSAFE(_T_cell) do { \
	((_T_cell)->ref_count--);\
	LM_DBG("UNREF_UNSAFE: [%p] after is %d\n",_T_cell, (_T_cell)->ref_count);\
	}while(0)

#define REF(_T_cell) do{ \
	LOCK_HASH( (_T_cell)->hash_index ); \
	REF_UNSAFE(_T_cell); \
	UNLOCK_HASH( (_T_cell)->hash_index ); }while(0)

#define UNREF(_T_cell) do{ \
	LOCK_HASH( (_T_cell)->hash_index ); \
	UNREF_UNSAFE(_T_cell); \
	UNLOCK_HASH( (_T_cell)->hash_index ); }while(0)
#define REF_UNSAFE(_T_cell) do {\
	((_T_cell)->ref_count++);\
	LM_DBG("REF_UNSAFE:[%p] after is %d\n",_T_cell, (_T_cell)->ref_count);\
	}while(0)
#define INIT_REF_UNSAFE(_T_cell) ((_T_cell)->ref_count=1)
#define IS_REFFED_UNSAFE(_T_cell) ((_T_cell)->ref_count!=0)

inline void set_fr_retr(struct retr_buf *rb, int retr );
inline void start_retr(struct retr_buf *rb);
inline void force_retr(struct retr_buf *rb);

void tm_shutdown();

/* function returns:
 *       1 - a new transaction was created
 *      -1 - error, including retransmission
 */
int  t_add_transaction( struct sip_msg* p_msg  );


/* returns 1 if everything was OK or -1 for error */
int t_release_transaction( struct cell *trans );


int get_ip_and_port_from_uri( str* uri , unsigned int *param_ip,
	unsigned int *param_port);


void put_on_wait(  struct cell  *Trans  );


void cleanup_localcancel_timers( struct cell *t );


int t_relay_to( struct sip_msg  *p_msg, struct proxy_l *proxy, int replicate);


#endif

