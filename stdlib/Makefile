
COMPILER	= ../bin/mcc
TARGETDIR	= ../bin
TARGET		= mlibc
INCDIR		= -i ./inc
SRCDIR		= ./src
SOURCES		= $(wildcard $(SRCDIR)/*.c)
OBJDIR		= ./load
OBJECTS		= $(addprefix $(OBJDIR)/,$(notdir $(SOURCES:.c=.o)))
ASMDIR		= ./asm
ASMBLERS	= $(addprefix $(ASMDIR)/,$(notdir $(SOURCES:.c=.s)))

$(TARGET) : $(OBJECTS)
	rm $(TARGETDIR)/$(TARGET)
	ar rcs $(TARGETDIR)/$(TARGET) $(OBJECTS)

$(OBJDIR)/%.o: $(ASMDIR)/%.s
	-mkdir -p $(OBJDIR)
	as $< -o $@

$(ASMDIR)/%.s: $(SRCDIR)/%.c
	-mkdir -p $(ASMDIR)
	$(COMPILER) -c $< $(INCDIR) -o $@