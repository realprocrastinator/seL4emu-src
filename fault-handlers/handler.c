
#include <assert.h>
#include <stdio.h>
#include <sel4/sel4.h>
#include <utils/util.h>
#include <autoconf.h>

#define FAULTER_BADGE_VALUE     (0xBEEF)
#define PROGNAME                "Handler: "
/* We signal on this notification to let the fauler know when we're ready to
 * receive its fault message.
 */
extern seL4_CPtr sequencing_ep_cap;

extern seL4_CPtr faulter_fault_ep_cap;

extern seL4_CPtr handler_cspace_root;
extern seL4_CPtr badged_faulter_fault_ep_cap;

extern seL4_CPtr faulter_tcb_cap;
extern seL4_CPtr faulter_vspace_root;
extern seL4_CPtr faulter_cspace_root;

int main(void)
{
    int error;
    seL4_Word tmp_badge;
    seL4_CPtr foreign_badged_faulter_empty_slot_cap;
    seL4_CPtr foreign_faulter_capfault_cap;
    seL4_MessageInfo_t seq_msginfo;

    printf(PROGNAME "Handler thread running!\n"
           PROGNAME "About to wait for empty slot from faulter.\n");


    seq_msginfo = seL4_Recv(sequencing_ep_cap, &tmp_badge);
    foreign_badged_faulter_empty_slot_cap = seL4_GetMR(0);
    printf(PROGNAME "Received init sequence msg: slot in faulter's cspace is "
           "%lu.\n",
           foreign_badged_faulter_empty_slot_cap);



    /* Mint the fault ep with a badge */

    error = seL4_CNode_Mint(
        handler_cspace_root,
        badged_faulter_fault_ep_cap,
        seL4_WordBits,
        handler_cspace_root,
        faulter_fault_ep_cap,
        seL4_WordBits,
        seL4_AllRights, FAULTER_BADGE_VALUE);

    ZF_LOGF_IF(error != 0, PROGNAME "Failed to mint ep cap with badge!");
    printf(PROGNAME "Successfully minted fault handling ep into local cspace.\n");


    /* This step is only necessary on the master kernel. On the MCS kernel it
     * can be skipped because we do not need to copy the badged fault EP into
     * the faulting thread's cspace on the MCS kernel.
     */

    error = seL4_CNode_Copy(
        faulter_cspace_root,
        foreign_badged_faulter_empty_slot_cap,
        seL4_WordBits,
        handler_cspace_root,
        badged_faulter_fault_ep_cap,
        seL4_WordBits,
        seL4_AllRights);

    ZF_LOGF_IF(error != 0, PROGNAME "Failed to copy badged ep cap into faulter's cspace!");
    printf(PROGNAME "Successfully copied badged fault handling ep into "
           "faulter's cspace.\n"
           PROGNAME "(Only necessary on Master kernel.)\n");



    /* Here we need to keep in mind which CPtr we give the kernel. On the MCS
     * kernel, we must give a CPtr which can be resolved during the course of
     * this seL4_TCB_SetSpace syscall, from within our own CSpace.
     *
     * On the Master kernel, we must give a CPtr which can be resolved during
     * the generation of a fault message, from within the CSpace of the
     * (usually foreign) faulting thread.
     */

    error = seL4_TCB_SetSpace(
        faulter_tcb_cap,
        foreign_badged_faulter_empty_slot_cap,
        faulter_cspace_root,
        0,
        faulter_vspace_root,
        0);

    ZF_LOGF_IF(error != 0, PROGNAME "Failed to configure faulter's TCB with our fault ep!");
    printf(PROGNAME "Successfully registered badged fault handling ep with "
           "the kernel.\n"
           PROGNAME "About to wake the faulter thread.\n");

    /* Signal the faulter thread to try to touch the invalid cspace slot. */
    seL4_Reply(seL4_MessageInfo_new(0, 0, 0, 0));
    /* Now wait for the fault IPC message from the kernel. */
    seL4_Recv(faulter_fault_ep_cap, &tmp_badge);



    /* The IPC message for a cap fault contains the cap address of the slot
     * which generated the cap fault.
     *
     * We need to retrieve this slot address and fill that slot with a random
     * endpoint cap of our choosing so that the faulter thread can touch that
     * slot successfully.
     */

    foreign_faulter_capfault_cap = seL4_GetMR(seL4_CapFault_Addr);

    printf(PROGNAME "Received fault IPC message from the kernel.\n"
           PROGNAME "Fault occured at cap addr %lu within faulter's cspace.\n",
           foreign_faulter_capfault_cap);


    /* Handle the fault by copying an endpoint cap into the empty slot. It
     * doesn't matter which one: as long as we copy it in with receive rights
     * because the faulter is going to attempt to perform an seL4_NBRecv() on
     * it.
     */

    error = seL4_CNode_Copy(
        faulter_cspace_root,
        foreign_faulter_capfault_cap,
        seL4_WordBits,
        handler_cspace_root,
        sequencing_ep_cap,
        seL4_WordBits,
        seL4_AllRights);

    ZF_LOGF_IF(error != 0, PROGNAME "Failed to copy a cap into faulter's cspace to resolve the fault!");
    printf(PROGNAME "Successfully copied a cap into foreign faulting slot.\n"
           PROGNAME "About to resume the faulter thread.\n");


    seL4_Reply(seL4_MessageInfo_new(0, 0, 0, 0));

    printf(PROGNAME "Successfully resumed faulter thread.\n"
           PROGNAME "Finished execution.\n");


    return 0;
}