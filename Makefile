target := uds_server

src := uds_stream.c \
	   uds_timer.c \
	   uds_utils.c \
	   uds_service_10.c \
	   uds_service_11.c \
	   uds_service_14.c \
	   uds_service_19.c \
	   uds_service_22.c \
	   uds_service_27.c \
	   uds_service_28.c \
	   uds_service_2e.c \
	   uds_service_2f.c \
	   uds_service_31.c \
	   uds_service_34.c \
	   uds_service_36.c \
	   uds_service_37.c \
	   uds_service_38.c \
	   uds_service_3e.c \
	   uds_service_85.c \
	   uds_service_filter.c \
	   uds.c \
	   uds_main.c

obj := $(patsubst %.c, %.o, $(src))

CC := clang

CFLAGS += -g -O0 -Wall -fPIC

LDFLAGS += -ljson-c

$(target):$(obj)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o:%.c
	$(CC) -o $@ -c $< $(CFLAGS)

.PHONY:clean

clean:
	@rm -rf $(obj) $(target)
