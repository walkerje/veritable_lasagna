message(STATUS "  Core module:")
set(VL_SOURCES_LOCAL)

vl_configure_component_header(Core vl_libconfig.h.in vl_libconfig.h)

# ------------------------------------------------------------------------------
# Core primitives
# ------------------------------------------------------------------------------
vl_add_source("vl_numtypes.c")
vl_add_source("vl_memory.c")
vl_add_source("vl_atomic_ptr.c")
vl_add_source("vl_compare.c")
vl_add_source("vl_hash.c")
vl_add_source("vl_algo.c")
vl_add_source("vl_rand.c")

# ------------------------------------------------------------------------------
# Containers and allocators
# ------------------------------------------------------------------------------
vl_add_source("vl_buffer.c")
vl_add_source("vl_pool.c")
vl_add_source("vl_arena.c")
vl_add_source("vl_stack.c")
vl_add_source("vl_queue.c")
vl_add_source("vl_deque.c")
vl_add_source("vl_linked_list.c")
vl_add_source("vl_set.c")
vl_add_source("vl_hashtable.c")

# ------------------------------------------------------------------------------
# Serialization and streams
# ------------------------------------------------------------------------------
vl_add_source("vl_msgpack.c")
vl_add_source("vl_msgpack_io.c")
vl_add_source("vl_stream.c")
vl_add_source("vl_stream_memory.c")
vl_add_source("vl_stream_filesys.c")

# ------------------------------------------------------------------------------
# Threading and synchronization
# ------------------------------------------------------------------------------
vl_add_source("vl_thread.c")
vl_add_source("vl_mutex.c")
vl_add_source("vl_srwlock.c")
vl_add_source("vl_condition.c")
vl_add_source("vl_semaphore.c")
vl_add_source("vl_async_pool.c")
vl_add_source("vl_async_queue.c")
vl_add_source("vl_thread_pool.c")

# ------------------------------------------------------------------------------
# Platform and system utilities
# ------------------------------------------------------------------------------
vl_add_source("vl_dynlib.c")
vl_add_source("vl_filesys.c")
vl_add_source("vl_socket.c")
vl_add_source("vl_log.c")

# ------------------------------------------------------------------------------
# SIMD
# ------------------------------------------------------------------------------
vl_add_source("vl_simd.c")
set(VL_SIMD_OBJECTS)

if("sse2" IN_LIST VL_SIMD_IMPLEMENTATIONS)
    vl_add_simd_implementation(sse2 platform/vl_simd_sse2.c)
endif()

if("avx2" IN_LIST VL_SIMD_IMPLEMENTATIONS)
    vl_add_simd_implementation(avx2 platform/vl_simd_avx2.c)
endif()

if("neon" IN_LIST VL_SIMD_IMPLEMENTATIONS)
    vl_add_simd_implementation(neon platform/vl_simd_neon.c)
endif()

if("neon64" IN_LIST VL_SIMD_IMPLEMENTATIONS)
    vl_add_simd_implementation(neon64 platform/vl_simd_neon64.c)
endif()

set(VL_CORE_SOURCES ${VL_SOURCES_LOCAL} ${VL_SIMD_OBJECTS} PARENT_SCOPE)