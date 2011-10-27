BUILDTYPE ?= Release

all: out/Makefile
	tools/gyp_node -f make
	$(MAKE) -C out BUILDTYPE=$(BUILDTYPE)
	-ln -fs out/Release/node node
	-ln -fs out/Debug/node node_g

out/Release/node: all

out/Makefile: node.gyp deps/uv/uv.gyp

clean:
	rm -rf out

distclean:
	rm -rf out

.PHONY: clean distclean all
