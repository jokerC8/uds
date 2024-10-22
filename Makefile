target := uds_server

src := uds_stream.c \
	   uds_timer.c \
	   uds.c \
	   uds_main.c

obj := $(patsubst %.c, %.o, $(src))

CC := clang

CFLAGS += -g -O0 -Wall -Werror -fPIC

LDFLAGS +=

$(target):$(obj)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o:%.c
	$(CC) -o $@ -c $< $(CFLAGS)

.PHONY:clean

clean:
	@rm -rf $(obj) $(target)
