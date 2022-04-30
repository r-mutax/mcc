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

$(TARGET): $(OBJECTS)
	-mkdir -p $(TARGETDIR)
	$(COMPILER) -o $@ $^

$(OBJECTS): $(SOURCES)
	-mkdir -p $(OBJDIR)
	$(COMPILER) $(CFLAGS) $(INCDIR) -o $@ -c $<

clean:
	-rm -f $(OBJECTS) $(DEPENDS) $(TARGET)
	-rmdir $(TARGETDIR)
	-rmdir $(OBJDIR)

.PHONY: clean

-include $(DEPENDS)
	