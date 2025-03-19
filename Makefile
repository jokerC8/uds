TARGET := uds_server

SRC := $(wildcard *.c)

OBJDIR := objs
OBJ := $(patsubst %.c, $(OBJDIR)/%.o, $(SRC))

CC := clang

CFLAGS += -g -O0 -Wall -fPIC

LDFLAGS += -ljson-c

$(TARGET):$(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o:%.c
	$(CC) -o $@ -c $< $(CFLAGS)

$(OBJ): | $(OBJDIR)
$(OBJDIR):
	@mkdir -p $(OBJDIR)

.PHONY:clean
clean:
	@rm -rf $(OBJDIR) $(TARGET)
