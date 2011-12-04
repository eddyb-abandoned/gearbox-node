GEAR_CC=$(patsubst %.gear,%.cc,$(wildcard src/*.gear src/modules/*.gear)) $(patsubst %.js,%.cc,$(wildcard src/*.js src/modules/*.js))

all: $(GEAR_CC)

tools/gear2cc/gear2cc.js: tools/gear2cc/aze2js.js tools/gear2cc/gear2cc.aze
	@echo Converting tools/gear2cc/gear2cc.aze -\> tools/gear2cc/gear2cc.js
	@node tools/gear2cc/aze2js tools/gear2cc/gear2cc.aze tools/gear2cc/gear2cc.js

%.cc: %.gear tools/gear2cc/gear2cc.js
	@echo Converting $< -\> $@
	@node tools/gear2cc/gear2cc $<
%.cc: %.js tools/gear2cc/gear2cc.js
	@echo Converting $< -\> $@
	@node tools/gear2cc/gear2cc $<
.PHONY: all
