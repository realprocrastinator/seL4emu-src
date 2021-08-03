

#include <assert.h>
#include <sel4/sel4.h>
#include <stdio.h>
#include <utils/util.h>

// cslot containing IPC endpoint capability
extern seL4_CPtr endpoint;
// cslot containing a capability to the cnode of the server
extern seL4_CPtr cnode;
// empty cslot
extern seL4_CPtr free_slot;

int main(int c, char *argv[]) {

	seL4_Word sender;
    seL4_MessageInfo_t info = seL4_Recv(endpoint, &sender);
    while (1) {
	    seL4_Error error;
        if (sender == 0) {

             /* No badge! give this sender a badged copy of the endpoint */
             seL4_Word badge = seL4_GetMR(0);
             seL4_Error error = seL4_CNode_Mint(cnode, free_slot, seL4_WordBits,
                                                cnode, endpoint, seL4_WordBits,
                                                seL4_AllRights, badge);
             printf("Badged %lu\n", badge);

             // use cap transfer to send the badged cap in the reply
             seL4_SetCap(0, free_slot);
             info = seL4_MessageInfo_new(0, 0, 1, 0);

             /* reply to the sender and wait for the next message */
             seL4_Reply(info);

             /* now delete the transferred cap */
             error = seL4_CNode_Delete(cnode, free_slot, seL4_WordBits);
             assert(error == seL4_NoError);

             /* wait for the next message */
             info = seL4_Recv(endpoint, &sender);
        } else {

             for (int i = 0; i < seL4_MessageInfo_get_length(info); i++) {
                 printf("%c", (char) seL4_GetMR(i));
             }
             printf("\n");

              error = seL4_CNode_SaveCaller(cnode, free_slot, seL4_WordBits);
              assert(error == 0);
              info = seL4_Recv(endpoint, &sender);
              for (int i = 0; i < seL4_MessageInfo_get_length(info); i++) {
                 printf("%c", (char) seL4_GetMR(i));
              }
              printf("\n");
              seL4_Send(free_slot, seL4_MessageInfo_new(0, 0, 0, 0));

             info = seL4_ReplyRecv(endpoint, info, &sender);
        }
    }

    return 0;
}