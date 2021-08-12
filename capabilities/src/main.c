
#include <stdio.h>
#include <sel4/sel4.h>
#include <sel4platsupport/bootinfo.h>
#include <utils/util.h>


int main(int argc, char *argv[]) {

    printf("Hello seL4! We are goint to test Cnode functionalities now\n");

    /* parse the location of the seL4_BootInfo data structure from
    the environment variables set up by the default crt0.S */
    seL4_BootInfo *info = platsupport_get_bootinfo();

    size_t initial_cnode_object_size = BIT(info->initThreadCNodeSizeBits);
    printf("Initial CNode is %zu slots in size\n", initial_cnode_object_size);

    size_t num_initial_cnode_slots = initial_cnode_object_size / (1u << seL4_SlotBits);
    printf("The CSpace has %zu CSlots\n", num_initial_cnode_slots);

    /* Copy cnode to the first slot */

    printf("\nClient trying to copy cnode to the first slot.\n");
    seL4_CPtr first_free_slot = info->empty.start;
    seL4_Error error = seL4_CNode_Copy(seL4_CapInitThreadCNode, first_free_slot, seL4_WordBits,
                                       seL4_CapInitThreadCNode, seL4_CapInitThreadTCB, seL4_WordBits,
                                       seL4_AllRights);
    ZF_LOGF_IF(error, "Failed to copy cap!");
    printf("Client successfully copied cnode to the first slot.\n");

    /* Copy cnode to the last slot */
    
    seL4_CPtr last_slot = info->empty.end - 1;
    
    printf("\nClient trying to copy cnode to the last slot.\n");
    /* use seL4_CNode_Copy to make another copy of the initial TCB capability to the last slot in the CSpace */
    error = seL4_CNode_Copy(seL4_CapInitThreadCNode, last_slot, seL4_WordBits,
                      seL4_CapInitThreadCNode, first_free_slot, seL4_WordBits, seL4_AllRights);
    ZF_LOGF_IF(error, "Failed to copy cap!");
    printf("Client successfully copied cnode to the last slot.\n");
    
    /* set the priority of the root task */

    printf("\nClient trying to set TCB priority using the copied cap.\n");
    error = seL4_TCB_SetPriority(last_slot, last_slot, 10);
    ZF_LOGF_IF(error, "Failed to set priority");
    printf("Client successfully set the priority using the copied cap.\n");
    
    /* revoke the cap */

    printf("\nClient trying to revoke TCB capabilities.\n");
    // delete the created TCB capabilities
    seL4_CNode_Revoke(seL4_CapInitThreadCNode, seL4_CapInitThreadTCB, seL4_WordBits);

    /* test if revoke is successful by calling move cap on an empty slot
     * we shoudld get seL4FailedLookup here! */

    /* try cnode move */

    printf("\nClient trying to move cap.\n");
    error = seL4_CNode_Move(seL4_CapInitThreadCNode, first_free_slot, seL4_WordBits,
                            seL4_CapInitThreadCNode, first_free_slot, seL4_WordBits);
    ZF_LOGF_IF(error != seL4_FailedLookup, "first_free_slot is not empty");
    
    // check last_slot is empty 
    error = seL4_CNode_Move(seL4_CapInitThreadCNode, last_slot, seL4_WordBits,
                            seL4_CapInitThreadCNode, last_slot, seL4_WordBits);
    ZF_LOGF_IF(error != seL4_FailedLookup, "last_slot is not empty");
    
    printf("Finger Crossed! Everything works fine. :)\n");

    printf("\nSuspending current thread\n");
    seL4_TCB_Suspend(seL4_CapInitThreadTCB);
    ZF_LOGF("Failed to suspend current thread\n");

    return 0;
}