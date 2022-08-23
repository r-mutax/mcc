CUR_MCPP_DIR	= $(shell pwd)

COMPILER		= cc
CFLAGS			= -std=c11 -g -static
TARGET			= mcc
TARGETDIR		= $(CUR_MCPP_DIR)/../bin
SRCDIR			= $(CUR_MCPP_DIR)/src
SOURCES			= $(wildcard $(SRCDIR)/*.c)
INCDIR			= -I$(CUR_MCPP_DIR)/inc
OBJDIR			= $(CUR_MCPP_DIR)/load
OBJECTS			= $(addprefix $(OBJDIR)/,$(notdir $(SOURCES:.c=.o)))
DEPENDS			= $(OBJECTS:.o=.d)

TEST_DIR		= $(CUR_MCPP_DIR)/../test/mcc


$(TARGET) : $(OBJECTS)
	-mkdir -p $(TARGETDIR)
	$(COMPILER) -o $(TARGETDIR)/$(TARGET) $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	-mkdir -p $(OBJDIR)
	$(COMPILER) $(CFLAGS) $(INCDIR) -o $@ -c $<

test:$(TARGET)
	$(TARGETDIR)/$(TARGET) -c $(TEST_DIR)/test.c -i ../stdlib/inc -o $(TEST_DIR)/test.s
	cc -s -static $(TEST_DIR)/test.s $(TEST_DIR)/../test_func/foo.o -o $(TEST_DIR)/a.out
	$(TEST_DIR)/a.out
	
clean:
	-rm -f $(OBJECTS) $(DEPENDS) $(TARGET)
	-rm $(TARGETDIR)/mcpp
	-rmdir $(OBJDIR)

.PHONY: clean test
