/*
 * Yanhua Liu (ytz2) CS820
 * global.h
 *
 *  Created on: Nov 11, 2013
 *      Author: yanhualiu
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_
#include "ospenv.h"
#include <pthread.h>
extern pthread_key_t line_buffer_key; // key to bind a line buffer
extern pthread_key_t out_buffer_key; // key to bind a line buffer
extern pthread_key_t err_buffer_key; // key to bind a line buffer
extern pthread_once_t init_done; // once key


#endif /* GLOBAL_H_ */
