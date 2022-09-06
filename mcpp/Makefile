CUR_MCPP_DIR	= $(shell pwd)

COMPILER		= cc
CFLAGS			= -std=c11 -g -static
TARGET			= mcpp
TARGETDIR		= $(CUR_MCPP_DIR)/../bin
SRCDIR			= $(CUR_MCPP_DIR)/src
SOURCES			= $(wildcard $(SRCDIR)/*.c)
INCDIR			= $(CUR_MCPP_DIR)/inc
OBJDIR			= $(CUR_MCPP_DIR)/load
OBJECTS			= $(addprefix $(OBJDIR)/,$(notdir $(SOURCES:.c=.o)))
DEPENDS			= $(OBJECTS:.o=.d)

TEST_DIR		= $(CUR_MCPP_DIR)/../test/mcpp


$(TARGET) : $(OBJECTS)
	-mkdir -p $(TARGETDIR)
	$(COMPILER) -o $(TARGETDIR)/$(TARGET) $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	-mkdir -p $(OBJDIR)
	$(COMPILER) $(CFLAGS) -I$(INCDIR) -o $@ -c $<

test:$(TARGET)
	$(TARGETDIR)/$(TARGET) $(TEST_DIR)/tst_preprocess.h -i $(TEST_DIR)/inc -o $(TEST_DIR)/output.h
	diff -Bw $(TEST_DIR)/correct_output.h $(TEST_DIR)/output.h

test_std:$(TARGET)
	$(TARGETDIR)/$(TARGET) $(TEST_DIR)/stdlib/tst_stdlib_include.h -o $(TEST_DIR)/stdlib/std_output.h
	cc -E -P -std=c11 $(TEST_DIR)/stdlib/tst_stdlib_include.h -o $(TEST_DIR)/stdlib/correct_std_output.h
	diff -Bw $(TEST_DIR)/stdlib/correct_std_output.h $(TEST_DIR)/stdlib/std_output.h

test_self_host:$(TARGET)
	# self host test at mcpp.
	-mkdir -p $(TEST_DIR)/self_host
	$(TARGETDIR)/$(TARGET) $(SRCDIR)/main.c -I $(INCDIR) -o $(TEST_DIR)/self_host/main1.i
	cc -E -P -std=c11 $(SRCDIR)/main.c -I$(INCDIR) -o $(TEST_DIR)/self_host/main2.i
	diff -Bw $(TEST_DIR)/self_host/main1.i $(TEST_DIR)/self_host/main2.i

	# $(TARGETDIR)/$(TARGET) $(SRCDIR)/expr.c -I $(INCDIR) -o $(TEST_DIR)/self_host/expr1.i
	# cc -E -P -std=c11 $(SRCDIR)/expr.c -I$(INCDIR) -o $(TEST_DIR)/self_host/expr2.i
	# diff -Bw $(TEST_DIR)/self_host/expr1.i $(TEST_DIR)/self_host/expr2.i

clean:
	-rm -f $(OBJECTS) $(DEPENDS) $(TARGET)
	-rm $(TARGETDIR)/mcpp
	-rmdir $(OBJDIR)

.PHONY: clean test
