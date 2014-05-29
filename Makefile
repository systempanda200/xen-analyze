CFLAGS += -g -O2
CFLAGS += -fno-strict-aliasing
CFLAGS += -std=gnu99
CFLAGS += -Wall -Wstrict-prototypes
CFLAGS += -Wno-unused-value
CFLAGS += -Wdeclaration-after-statement

CFLAGS += -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE
CFLAGS += -mno-tls-direct-seg-refs
CFLAGS += -Werror

BIN := xenalyze dump-raw

xenalyze_OBJS := \
	mread.o \
	xenalyze.o

dump-raw_OBJS := \
	dump-raw.o

all: $(BIN)

xenalyze: $(xenalyze_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

dump-raw: $(dump-raw_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -MD -MP -c -o $@ $<

all_objs := $(foreach b,$(BIN),$($(b)_OBJS))
all_deps := $(all_objs:.o=.d)

.PHONY: clean
clean:
	$(RM) $(BIN) $(all_objs) $(all_deps)

-include $(all_deps)
