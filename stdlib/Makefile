CUR_LIB_DIR		= $(shell pwd)

MCC			= $(CUR_LIB_DIR)/../bin/mcc
TARGETDIR	= $(CUR_LIB_DIR)/../bin
TARGET		= mlibc
INCDIR		= -i $(CUR_LIB_DIR)/inc
SRCDIR		= $(CUR_LIB_DIR)/src
SOURCES		= $(wildcard $(SRCDIR)/*.c)
OBJDIR		= $(CUR_LIB_DIR)/load
OBJECTS		= $(addprefix $(OBJDIR)/,$(notdir $(SOURCES:.c=.o)))
ASMDIR		= $(CUR_LIB_DIR)/asm
ASMBLERS	= $(addprefix $(ASMDIR)/,$(notdir $(SOURCES:.c=.s)))

$(TARGET) : $(OBJECTS)
	rm $(TARGETDIR)/$(TARGET) -f
	ar rcs $(TARGETDIR)/$(TARGET) $(OBJECTS)

$(OBJDIR)/%.o: $(ASMDIR)/%.s
	-mkdir -p $(OBJDIR)
	as $< -o $@

$(ASMDIR)/%.s: $(SRCDIR)/%.c
	-mkdir -p $(ASMDIR)
	$(MCC) -c $< $(INCDIR) -o $@