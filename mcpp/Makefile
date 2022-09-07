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

TEST_S_DIR		= $(TEST_DIR)/self_host
TEST_I_INC		= $(CUR_MCPP_DIR)/../mcc1/inc
TEST_I_CC1_DIR	= $(CUR_MCPP_DIR)/../mcc1/src
TEST_I_CC1_SRC	= $(wildcard $(TEST_I_CC1_DIR)/*.c)
TEST_I_SRC		= $(TEST_I_CC1_SRC) $(SOURCES)
TEST_I_1		= $(addprefix $(TEST_S_DIR)/,$(notdir $(TEST_I_SRC:.c=1.i)))
TEST_I_2		= $(addprefix $(TEST_S_DIR)/,$(notdir $(TEST_I_SRC:.c=2.i)))
TEST_I_DIFF		= $(addprefix $(TEST_S_DIR)/,$(notdir $(TEST_I_SRC:.c=.diff)))


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

$(TEST_S_DIR)/%1.i: $(SRCDIR)/%.c
	######### EXEC MCPP $< #########
	-mkdir -p $(TEST_DIR)/self_host
	$(TARGETDIR)/$(TARGET) $< -i $(INCDIR) -o $@

$(TEST_S_DIR)/%2.i: $(SRCDIR)/%.c
	######### EXEC cc $< #########
	-mkdir -p $(TEST_DIR)/self_host
	cc -E -P -std=c11 $< -I$(INCDIR) -o $@

$(TEST_S_DIR)/%1.i: $(TEST_I_CC1_DIR)/%.c
	######### EXEC MCPP $< #########
	-mkdir -p $(TEST_DIR)/self_host
	$(TARGETDIR)/$(TARGET) $< -i $(TEST_I_INC) -o $@

$(TEST_S_DIR)/%2.i: $(TEST_I_CC1_DIR)/%.c
	######### EXEC cc $< #########
	-mkdir -p $(TEST_DIR)/self_host
	cc -E -P -std=c11 $< -I$(TEST_I_INC) -o $@

$(TEST_S_DIR)/%.diff: $(SRCDIR)/%.c
	diff -Bw $(subst .c,1.i,$(subst $(SRCDIR),$(TEST_S_DIR),$<)) $(subst .c,2.i, $(subst $(SRCDIR),$(TEST_S_DIR),$<)) 

$(TEST_S_DIR)/%.diff: $(TEST_I_CC1_DIR)/%.c
	diff -Bw $(subst .c,1.i,$(subst $(TEST_I_CC1_DIR),$(TEST_S_DIR),$<)) $(subst .c,2.i, $(subst $(TEST_I_CC1_DIR),$(TEST_S_DIR),$<)) 

test_self_host:clean_self_host $(TARGET) $(TEST_I_1) $(TEST_I_2) $(TEST_I_DIFF)
	echo test complete

clean_self_host:
	-rm -f $(TEST_I_1)

clean:
	-rm -f $(OBJECTS) $(DEPENDS) $(TARGET)
	-rm $(TARGETDIR)/mcpp
	-rmdir $(OBJDIR)

.PHONY: clean test test_std test_self_host clean_self_host
