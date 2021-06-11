<!--
     Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)

     SPDX-License-Identifier: CC-BY-SA-4.0
-->

# libsel4nanopb

A interface between sel4 IPC buffers and nanopb.
### Nanopb
Nanopb is a small code-size Protocol Buffers implementation in ANSI C.
The nanopb generator is implemented as a plug-in for the Google's own protoc compiler.
Therefore, you need [Google's Protocol Buffers](https://developers.google.com/protocol-buffers/docs/reference/overview) installed in order to run the generator.

### This library
Nanopb uses streams for accessing the data in encoded format. This library implements a simple wrapper
which allows you to treat the IPC message buffer as a protobuf stream.
```
Message message = Message_init_zero;
message.somepart = content;
pb_ostream_t output = pb_ostream_from_IPC(0);
if (!pb_encode_delimited(&output, Control_fields, &ctrlmsg))
{
    ZF_LOGE("Encoding failed: %s\n", PB_GET_ERROR(&output));
}
```
