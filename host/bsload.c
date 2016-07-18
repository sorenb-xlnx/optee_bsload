/*
 * Copyright (c) 2016, Xilinx Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <err.h>
#include <tee_client_api.h>
#include <zynqmp_bsload.h>

static void bs_disk2mem(TEEC_TempMemoryReference *mem, const char *bsfile)
{
	int fd;
	FILE *fin = fopen(bsfile, "r");

	if (!fin)
		return;

	fseek(fin, 0L, SEEK_END);
	mem->size = ftell(fin);

	if (!mem->size)
		return;

	rewind(fin);

	fd = fileno(fin);

	mem->buffer = mmap(NULL, mem->size, PROT_READ, MAP_SHARED, fd, 0);

	fclose(fin);
}

static void bs_free(TEEC_TempMemoryReference *mem)
{
	munmap(mem->buffer, mem->size);
}

int main(int argc, char *argv[])
{
	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_UUID uuid = ZYNQMP_BSLOAD_UUID;
	uint32_t err_origin;
	const char *bsfile = "bs.bit";

	if (argc > 1) {
		bsfile = argv[1];
	}

	/* Initialize a context connecting us to the TEE */
	res = TEEC_InitializeContext(NULL, &ctx);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

	/* Open a session to the "bsload" TA */
	res = TEEC_OpenSession(&ctx, &sess, &uuid,
			       TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_Opensession failed with code %#x origin %#x",
			res, err_origin);

	/*
	 * Execute a function in the TA by invoking it, in this case
	 * we're incrementing a number.
	 *
	 * The value of command ID part and how the parameters are
	 * interpreted is part of the interface provided by the TA.
	 */

	/*
	 * Prepare the argument. Pass a value in the first parameter,
	 * the remaining three parameters are unused.
	 */
	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_NONE,
					 TEEC_NONE, TEEC_NONE);
	bs_disk2mem(&op.params[0].tmpref, bsfile);
	if (!op.params[0].tmpref.buffer)
		errx(1, "bs_disk2mem failed");

#ifdef DEBUG
	{
		uint32_t *data = op.params[0].tmpref.buffer;
		printf("size:%zu, data:%#x\n", op.params[0].tmpref.size, *data);
	}
#endif
	printf("Invoking TA to load bitstream '%s'\n", bsfile);
	res = TEEC_InvokeCommand(&sess, ZYNQMP_BSLOAD_CMD_LOAD_BITSTREAM, &op,
				 &err_origin);
	bs_free(&op.params[0].tmpref);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InvokeCommand failed with code %#x origin %#x",
			res, err_origin);
	else
		printf("bitstream loaded\n");

	/*
	 * We're done with the TA, close the session and
	 * destroy the context.
	 */

	TEEC_CloseSession(&sess);

	TEEC_FinalizeContext(&ctx);

	return 0;
}
