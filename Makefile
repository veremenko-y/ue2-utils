all:
	make -C cmd
	make -C cmd/as

clean:
	make -C cmd clean
	make -C cmd/as clean