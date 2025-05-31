/**
 * \file
 * \copyright Copyright 2025 Jesse Walker, released under the MIT license.
 * A Data Structure and Algorithms library for C11.
 *
 * Git Repository:  https://github.com/walkerje/veritable_lasagna
 * Homepage:        https://bitshiftmountain.com/projects/vl/docs
 */

/**
 * ██    ██ ██       █████  ███████  █████   ██████  ███    ██  █████
 * ██    ██ ██      ██   ██ ██      ██   ██ ██       ████   ██ ██   ██
 * ██    ██ ██      ███████ ███████ ███████ ██   ███ ██ ██  ██ ███████
 *  ██  ██  ██      ██   ██      ██ ██   ██ ██    ██ ██  ██ ██ ██   ██
 *   ████   ███████ ██   ██ ███████ ██   ██  ██████  ██   ████ ██   ██
 */

#ifndef VLASAGNA_H
#define VLASAGNA_H

/**
 * Configuration, Types, and Low-Level Memory
 */
#include "vl/vl_libconfig.h"
#include "vl/vl_numtypes.h"
#include "vl/vl_memory.h"

/**
 * Data Structures & Algorithms
 */
#include "vl/vl_buffer.h"
#include "vl/vl_hashtable.h"
#include "vl/vl_arena.h"
#include "vl/vl_pool.h"
#include "vl/vl_linked_list.h"
#include "vl/vl_set.h"
#include "vl/vl_stack.h"
#include "vl/vl_queue.h"
#include "vl/vl_deque.h"
#include "vl/vl_rand.h"

/**
 * Asynchronous Structures
 */
#include "vl/vl_atomic.h"
#include "vl/vl_atomic_ptr.h"
#include "vl/vl_thread.h"
#include "vl/vl_mutex.h"
#include "vl/vl_srwlock.h"
#include "vl/vl_semaphore.h"
#include "vl/vl_async_pool.h"
#include "vl/vl_async_queue.h"

/**
 * Serialization & Deserialization
 */
#include "vl/vl_msgpack.h"
#include "vl/vl_msgpack_io.h"

#endif