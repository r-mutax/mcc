COMPILER	= cc
CFLAGS		= -std=c11 -g -static
TARGETDIR	= ./bin
TARGET		= $(TARGETDIR)/$(shell basename `readlink -f .`)
INCDIR		= -I./inc
SRCDIR		= ./src
SOURCES		= $(wildcard $(SRCDIR)/*.c)
OBJDIR		= ./load
OBJECTS		= $(addprefix $(OBJDIR)/,$(notdir $(SOURCES:.c=.o)))
DEPENDS		= $(OBJECTS:.o=.d)

LIBINCDIR	= -I./bin/stdlib/include
LIBSRCDIR	= ./src/stdlib
LIBSOURCES	= $(wildcard $(LIBSRCDIR)/*.c)
LIBOBJDIR	= ./load/stdlib
LIBOBJECTS	= $(addprefix $(LIBOBJDIR)/,$(notdir $(LIBSOURCES:.c=.o)))
LIBDEPENDS	= $(LIBOBJECTS:.o=.d)

$(TARGET): $(OBJECTS)
	-mkdir -p $(TARGETDIR)
	$(COMPILER) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	-mkdir -p $(OBJDIR)
	$(COMPILER) $(CFLAGS) $(INCDIR) -o $@ -c $<

stdlib: $(LIBOBJECTS)
	ar rc $(TARGETDIR)/mlibc $(LIBOBJECTS)

$(LIBOBJDIR)/%.o: $(LIBSRCDIR)/%.c
	$(COMPILER) $(CFLAGS) $(LIBINCDIR) -o $@ -c $< -static


clean:
	-rm -f $(OBJECTS) $(DEPENDS) $(TARGET)
	-rmdir $(TARGETDIR)/mcc
	-rmdir $(OBJDIR)

test:$(TARGET)
	./bin/mcc ./test/test.c > ./test/test.s
	cc -s -static ./test/test.s ./test/test_func/foo.o ./bin/mlibc -o ./test/a.out
	./test/a.out

.PHONY: clean test

-include $(DEPENDS)
	