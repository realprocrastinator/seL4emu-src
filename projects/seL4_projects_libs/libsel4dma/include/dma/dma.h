/*
 * Copyright 2017, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
/* Author: alex.kroh@nicta.com.au */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <dma/dma.h>
#include <platsupport/io.h>

struct dma_allocator;
typedef struct dma_mem *dma_mem_t;

typedef void *vaddr_t;
typedef uintptr_t paddr_t;

/**
 * Describes a chunk of memory for DMA use.
 */
struct dma_mem_descriptor {
    /** The virtual address of the memory.
        It is the applications responsibility to perform the mapping operation. */
    uintptr_t vaddr;
    /// The physical address of the memory.
    uintptr_t paddr;
    /** Boolian representation of the cacheability of the frames in this
        descriptor. */
    int       cached;
    /// The size of each frame (2^frame_size_bits bytes)
    int       size_bits;
    /// This field is unused and may be used by the bulk allocator
    void     *alloc_cookie;
    /// This field is unused and may be used by the application.
    void     *cookie;
};


/**
 * DMA access flags.
 * DMA access patterns are important to the DMA memory manager because cache
 * maintenance operations may affect (corrupt) adjacent buffers. This is due
 * to cach line size limits and software flushing granularity (perhaps the OS
 * interface limits us to perform maintenance operations on a per page basis).
 *
 * These flags will generally lead to multiple memory pools where each request
 * will be allocated from the appropriate pool.
 */
enum dma_flags {
    /// Host always reads, never writes (invalidate only)
    DMAF_HR,
    /// Host always writes, never reads (clean only)
    DMAF_HW,
    /// Host performs both read and write (clean/invalidate)
    DMAF_HRW,
    /// Explicitly request uncached DMA memory
    DMAF_COHERENT
};


/**
 * A callback for providing DMA memory to the allocator.
 * @param[in]  min_size the minimum size for the allocation
 * @param[in]  cached   0 if the requested memory should not be cached.
 * @param[out] dma_desc On return, this structure should be filled with a
 *                      description of the memory that has been provided.
 * @return              0 on success
 */
typedef int (*dma_morecore_fn)(size_t min_size, int cached,
                               struct dma_mem_descriptor *dma_desc);

/**
 * Initialises a new DMA allocator for use with io_ops.
 * @param[in]  morecore A function to use when the allocator requires more DMA
 *                      memory. NULL if the allocator should not request more
 *                      memory.
 * @param[in]  cache_op Operations to use for cleaning/invalidating the cache
 * @param[out] dma_man  libplatsupport DMA manager structure to populate
 * @return              0 on success
 */
int dma_dmaman_init(dma_morecore_fn morecore, ps_dma_cache_op_fn_t cache_ops,
                    ps_dma_man_t *dma_man);


/**
 * Explicitly provides memory to the DMA allocator.
 * @param[in] dma_desc A description of the memory provided.
 * @return             0 on success
 */
int dma_provide_mem(struct dma_allocator *allocator,
                    struct dma_mem_descriptor dma_desc);

/**
 * If possible, reclaim some memory from the DMA allocator.
 * @param[in]  allocator A handle to the allocator to reclaim memory from
 * @param[out] dma_desc  If the call is successful, this structure will
 *                       be filled with a description of the memeory that
 *                       has been released from the allocator.
 * @return               0 on success, non-zero indicates that a suitable
 *                       reclaim candidate could not be found.
 */
int dma_reclaim_mem(struct dma_allocator *allocator,
                    struct dma_mem_descriptor *dma_desc);



/******************
 * Legacy API
 ******************/

/**
 * Initialises a new DMA allocator.
 * @param[in] morecore A function to use when the allocator requires more DMA
 *                     memory. NULL if the allocator should not request more
 *                     memory.
 * @return             A reference to a new DMA allocator instance.
 */
struct dma_allocator *dma_allocator_init(dma_morecore_fn morecore);

/**
 * Retrieve the virtual address of allocated DMA memory.
 * @param[in] dma_mem       A handle to DMA memory.
 * @return                  The virtual address of the DMA memory in question.
 */
vaddr_t dma_vaddr(dma_mem_t dma_mem);

/**
 * Retrieve the physical address of allocated DMA memory.
 * @param[in] dma_mem       A handle to DMA memory.
 * @return                  The physical address of the DMA memory in question.
 */
paddr_t dma_paddr(dma_mem_t dma_mem);

/**
 * Flush DMA memory out to RAM by virtual address
 * @param[in] dma_mem A handle to allocated DMA memory.
 * @param[in] vstart  The staring virtual address of the flush.
 * @param[in] vend    One greater than the last virtual address to be
 *                    flushed.
 */
void dma_clean(dma_mem_t dma_mem, vaddr_t vstart, vaddr_t vend);

/**
 * Invalidate DMA memory from cache by virtual address
 * @param[in] dma_mem A handle to allocated DMA memory.
 * @param[in] vstart  The staring virtual address of the invalidation.
 * @param[in] vend    One greater than the last virtual address to be
 *                    invalidated.
 */
void dma_invalidate(dma_mem_t dma_mem, vaddr_t vstart, vaddr_t vend);

/**
 * Flush DMA memory out to RAM and invalidate the caches by virtual address.
 * @param[in] dma_mem A handle to allocated DMA memory.
 * @param[in] vstart  The staring virtual address of the clean/invalidate.
 * @param[in] vend    One greater than the last virtual address to be
 *                          cleaned/invalidated.
 */
void dma_cleaninvalidate(dma_mem_t dma_mem, vaddr_t vstart, vaddr_t vend);

/**
 * Allocate DMA memory.
 * @param[in]  allocator The DMA allocator instance to use for the allocation.
 * @param[in]  size      The allocation size.
 * @param[in]  align     The minimum alignment (in bytes) of the allocated
 *                       region.
 * @param[in]  flags     The allocation properties of the request.
 * @param[out] dma_mem   If the call is successful and dma_mem is not NULL,
 *                       dma_mem will contain a handle to the allocated memory.
 * @return               The virtual address of the allocated DMA memory, NULL
 *                       on failure.
 */
vaddr_t dma_alloc(struct dma_allocator *allocator, size_t size, int align,
                  enum dma_flags flags, dma_mem_t *dma_mem);

/**
 * Free DMA memory by virtual address.
 * @param[in] dma_mem   A handle to the DMA memory to free.
 */
void dma_free(dma_mem_t dma_mem);


/**
 * Retrieve the DMA memory handle from a given physical address.
 * The performance of this operation is likely to be poor, but it may be useful
 * for rapid prototyping or debugging.
 * @param[in] allocator The allocator managing the memory.
 * @param[in] paddr     The physical address of the memory in question
 * @return              The DMA memory handle associated with the
 *                      provided physical address. NULL if the physical
 *                      address is not managed by the provided allocator.
 */
dma_mem_t dma_plookup(struct dma_allocator *allocator, paddr_t paddr);

/**
 * Retrieve the DMA memory handle from a given virtual address.
 * The performance of this operation is likely to be poor, but it may be useful
 * for rapid prototyping or debugging.
 * @param[in] allocator The allocator managing the memory.
 * @param[in] vaddr     The virtual address of the memory in question
 * @return              The DMA memory handle associated with the
 *                      provided virtual address. NULL if the physical
 *                      address is not managed by the provided allocator.
 */
dma_mem_t dma_vlookup(struct dma_allocator *allocator, vaddr_t vaddr);
