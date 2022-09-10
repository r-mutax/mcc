CUR_MCC1_DIR	= $(shell pwd)

COMPILER		= cc
CFLAGS			= -std=c11 -g -static
TARGET			= mcc1
TARGETDIR		= $(CUR_MCC1_DIR)/../bin
SRCDIR			= $(CUR_MCC1_DIR)/src
SOURCES			= $(wildcard $(SRCDIR)/*.c)
INCDIR			= -I$(CUR_MCC1_DIR)/inc
OBJDIR			= $(CUR_MCC1_DIR)/load
OBJECTS			= $(addprefix $(OBJDIR)/,$(notdir $(SOURCES:.c=.o)))
DEPENDS			= $(OBJECTS:.o=.d)

TEST_DIR		= $(CUR_MCC1_DIR)/../test/mcc1

$(TARGET) : $(OBJECTS)
	-mkdir -p $(TARGETDIR)
	$(COMPILER) -o $(TARGETDIR)/$(TARGET) $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	-mkdir -p $(OBJDIR)
	$(COMPILER) $(CFLAGS) $(INCDIR) -o $@ -c $<

test:$(TARGET)
	$(TARGETDIR)/$(TARGET) -c $(TEST_DIR)/test.c -o $(TEST_DIR)/test.s
	cc -s -static $(TEST_DIR)/test.s $(TEST_DIR)/../test_func/foo.o $(TARGETDIR)/mlibc -o $(TEST_DIR)/a.out
	$(TEST_DIR)/a.out

clean:
	-rm -f $(OBJECTS) $(DEPENDS) $(TARGET)
	-rm $(TARGETDIR)/mcpp
	-rmdir $(OBJDIR)

.PHONY: clean
