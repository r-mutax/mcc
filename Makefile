BIN_DIR		= ./bin
MCC			= $(BIN_DIR)/mcc
MCC1		= $(BIN_DIR)/mcc1
MCPP		= $(BIN_DIR)/mcpp

all:$(MCC) $(MCC1) $(MCPP)

$(MCC):
	(cd mcc; make;)

$(MCC1):
	(cd mcc1; make;)

$(MCPP):
	(cd mcpp; make;)

test:$(MCC) $(MCC1) $(MCPP)
	(cd mcc; make test;)

.PHONY: all test
