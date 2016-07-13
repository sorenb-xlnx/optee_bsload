
export V?=0

.PHONY: all
all:
	make -C host CROSS_COMPILE=$(HOST_CROSS_COMPILE)

.PHONY: clean
clean:
	make -C host clean
