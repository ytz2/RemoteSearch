/*
 * thread_share.c
 *
 *  Created on: Nov 11, 2013
 *      Author: yanhualiu
 */

#include "thread_share.h"

/*initialize the key*/
void thread_init() {
	pthread_key_create(&line_buffer_key, free);
	pthread_key_create(&out_buffer_key, free);
	pthread_key_create(&err_buffer_key, free);
#ifndef __sun__
	pthread_key_create(&strerr_key, free);
#endif
}

