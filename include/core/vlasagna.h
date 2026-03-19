/**
 * ██    ██ ██       █████  ███████  █████   ██████  ███    ██  █████
 * ██    ██ ██      ██   ██ ██      ██   ██ ██       ████   ██ ██   ██
 * ██    ██ ██      ███████ ███████ ███████ ██   ███ ██ ██  ██ ███████
 *  ██  ██  ██      ██   ██      ██ ██   ██ ██    ██ ██  ██ ██ ██   ██
 *   ████   ███████ ██   ██ ███████ ██   ██  ██████  ██   ████ ██   ██
 * ====---: A Data Structure and Algorithms library for C11.  :---====
 *
 * Copyright 2026 Jesse Walker, released under the MIT license.
 * Git Repository:  https://github.com/walkerje/veritable_lasagna
 * \private
 */

#ifndef VLASAGNA_H
#define VLASAGNA_H

/**
 * Configuration, Types, and Low-Level Memory
 */
#include "vl/vl_half.h"
#include "vl/vl_libconfig.h"
#include "vl/vl_memory.h"
#include "vl/vl_nibble.h"
#include "vl/vl_numtypes.h"

/**
 * Low-level utilities
 */
#include "vl/vl_algo.h"
#include "vl/vl_compare.h"
#include "vl/vl_hash.h"

/**
 * Data Structures
 */
#include "vl/vl_arena.h"
#include "vl/vl_buffer.h"
#include "vl/vl_deque.h"
#include "vl/vl_hashtable.h"
#include "vl/vl_linked_list.h"
#include "vl/vl_pool.h"
#include "vl/vl_queue.h"
#include "vl/vl_set.h"
#include "vl/vl_stack.h"

/**
 * Random
 */
#include "vl/vl_rand.h"

/**
 * Concurrency, Atomics, and Synchronization
 */
#include "vl/vl_atomic.h"
#include "vl/vl_atomic_ptr.h"
#include "vl/vl_condition.h"
#include "vl/vl_mutex.h"
#include "vl/vl_semaphore.h"
#include "vl/vl_srwlock.h"
#include "vl/vl_thread.h"

/**
 * Asynchronous Structures
 */
#include "vl/vl_async_pool.h"
#include "vl/vl_async_queue.h"
#include "vl/vl_thread_pool.h"

/**
 * Logging and Streams (I/O abstraction)
 */
#include "vl/vl_log.h"
#include "vl/vl_stream.h"
#include "vl/vl_stream_filesys.h"
#include "vl/vl_stream_memory.h"

/**
 * Platform / OS Abstractions
 */
#include "vl/vl_ansi_term.h"
#include "vl/vl_dynlib.h"
#include "vl/vl_filesys.h"
#include "vl/vl_socket.h"

/**
 * SIMD
 */
#include "vl/vl_simd.h"

/**
 * Serialization & Deserialization
 */
#include "vl/vl_msgpack.h"
#include "vl/vl_msgpack_io.h"

#endif
