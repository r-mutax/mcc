CUR_MCPP_DIR	= $(shell pwd)

COMPILER		= cc
CFLAGS			= -std=c11 -g -static
TARGET			= mcpp
TARGETDIR		= $(CUR_MCPP_DIR)/../bin
SRCDIR			= $(CUR_MCPP_DIR)/src
SOURCES			= $(wildcard $(SRCDIR)/*.c)
INCDIR			= -I$(CUR_MCPP_DIR)/inc
OBJDIR			= $(CUR_MCPP_DIR)/load
OBJECTS			= $(addprefix $(OBJDIR)/,$(notdir $(SOURCES:.c=.o)))
DEPENDS			= $(OBJECTS:.o=.d)

TEST_DIR		= $(CUR_MCPP_DIR)/../test/mcpp


$(TARGET) : $(OBJECTS)
	-mkdir -p $(TARGETDIR)
	$(COMPILER) -o $(TARGETDIR)/$(TARGET) $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	-mkdir -p $(OBJDIR)
	$(COMPILER) $(CFLAGS) $(INCDIR) -o $@ -c $<

test:$(TARGET)
	$(TARGETDIR)/$(TARGET) $(TEST_DIR)/tst_preprocess.h -i $(TEST_DIR)/inc -o $(TEST_DIR)/output.h
	diff -Bw $(TEST_DIR)/correct_output.h $(TEST_DIR)/output.h

clean:
	-rm -f $(OBJECTS) $(DEPENDS) $(TARGET)
	-rm $(TARGETDIR)/mcpp
	-rmdir $(OBJDIR)

.PHONY: clean test
