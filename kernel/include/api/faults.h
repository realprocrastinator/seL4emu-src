/*
 * Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#pragma once

#include <object/structures.h>
#include <types.h>

word_t setMRs_fault(tcb_t *sender, tcb_t *receiver, word_t *receiveIPCBuffer);
word_t Arch_setMRs_fault(tcb_t *sender, tcb_t *receiver, word_t *receiveIPCBuffer, word_t faultType);

bool_t handleFaultReply(tcb_t *receiver, tcb_t *sender);
bool_t Arch_handleFaultReply(tcb_t *receiver, tcb_t *sender, word_t faultType);

#ifdef CONFIG_SEL4_USE_EMULATION
#include <machine/registerset.h>

unsigned int setMRs_lookup_failure(tcb_t *receiver, word_t *receiveIPCBuffer,
                                   lookup_fault_t luf, unsigned int offset);

void copyMRsFaultReply(tcb_t *sender, tcb_t *receiver, 
                       MessageID_t id, word_t length);

void copyMRsFault(tcb_t *sender, tcb_t *receiver, MessageID_t id,
                  word_t length, word_t *receiveIPCBuffer);
#endif
