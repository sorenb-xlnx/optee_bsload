# Introduction
This is the user-space portion for loading an FPGA bitstream on an [Xilinx Zynq
UltraScale+
MPSOC](http:////www.xilinx.com/products/silicon-devices/soc/zynq-ultrascale-mpsoc.html)
in combination with [OP-TEE](https://github.com/op-tee).

The trusted OS part is implemented as static TA within OP-TEE (not upstream as
of now).

# Build Instructions
Building the use-space application requires the [TEE client
library](https://github.com/OP-TEE/optee_client) and the TA devkit from an
OP-TEE build.

## Environment Variables
*   `HOST_CROSS_COMPILE`: Toolchain for the architecture the application will run on
*   `TEEC_EXPORT`: TEE client library exports
*   `TA_DEV_KIT_DIR`: Directory of the TA devkit

## Build Commands
```
make
```

# Running the Application
```
./bsload [<bitstream_file>]
```
**Options**
*   `bitstream_file`: Bitstream to load (bs.bit)

# Preparing the Bitstream
The TA expects a bitstream in bin format. A .bit file can be converted to a .bin
using the *bootgen* utility following
[this AR](http://www.xilinx.com/support/answers/46913.html).

# Known Issues
* The TA assumes that all necessary pre-processing of the bitstream is done in
user-space. Currently that requires user-space to convert the endianess of the
.bit file. This step is currently not implemented in the user-space application.
Hence, the bitstream passed to this application needs to be prepared accordingly
beforehand.

# Todos
* Add bitstream processing to application (see [Known Issues](#known-issues))

# Credits
* build and application boilerplate has been copied from https://github.com/jenswi-linaro/lcu14_optee_hello_world
