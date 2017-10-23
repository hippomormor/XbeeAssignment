BUILD_NAME ?= wireless_project1
# Build variables
BUILD_DIR ?= ./build
SRC_DIRS ?= ./
IP ?= x.x.x.x
DBG ?= -w
TARGET_DIR ?= /
TARGET_EXE ?= $(BUILD_NAME)
TARGET_USR ?= pi
PORT ?= 22
MKDIR_P ?= mkdir -p
RSA_PUB ?= ~/.ssh/id_rsa.pub

# INCLUDES:
SRCS := main.cpp XBeeHandler.cpp 

# EXCLUDES:
EXCLUDES := 

# FILTER OUT EXCLUDES FROM SOURCES:
TMPSRCS := $(SRCS)
SRCS := $(filter-out $(EXCLUDES), $(TMPSRCS))

# OBJECT LIST:
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

# INCLUDE PATHS:
INCDIRS += $(shell find -name '*.h' -printf '%h\n' | sort -u)
INCDIRS += $(shell find -name '*.hpp' -printf '%h\n' | sort -u)
INC_FLAGS := $(addprefix -I,$(INCDIRS))
CPPFLAGS += $(INC_FLAGS) $(INC_DIRS) $(DBG)

# LINKER FLAGS:
LD_FLAGS_DEF := -lm -lpthread -lrt -lstdc++
LDFLAGS += $(LD_FLAGS) $(LD_FLAGS_DEF)

# DEFAULT COMPILERs:
GCC ?= 
GXX ?= 
GCC_NATIVE ?= /usr/bin/gcc
GXX_NATIVE ?= /usr/bin/g++

# COMPILER FLAGS
CFLAGS += $(C_FLAGS)
CXXFLAGS += -std=c++11 $(CXX_FLAGS)

# MAKE DEFAULT
.PHONY: default
default: check_arm check_headers $(BUILD_DIR)/$(BUILD_NAME) ; @echo "\033[1;33mBuild finished\033[0m"

# MAKE NATIVE
.PHONY: native
native: check_cc check_headers $(BUILD_DIR)/$(BUILD_NAME); @echo "\033[1;33mBuild finished\033[0m"
native: GCC := $(GCC_NATIVE)
native: GXX := $(GXX_NATIVE)
	

# LINKER INSTRUCTIONS:
$(BUILD_DIR)/$(BUILD_NAME): $(OBJS)
	@echo "\033[0;32m[Linker]\033[0m \033[0;33mLinking object files into binary executable:\033[0m"
	@echo $(GXX) "<object files>" -o $@ $(LDFLAGS)
	@$(GXX) $(OBJS) -o $@ $(LDFLAGS)
	@echo "\033[1;33mDone\n\033[0m\n\033[1;36mTarget built successfully!\n\033[0m$(BUILD_DIR)/$(BUILD_NAME)\n\033[0m"

# C BUILD INSTRUCTIONS:
$(BUILD_DIR)/%.c.o: %.c
	@echo "\033[0;32m[C] \033[0m \033[0;33mCompiling:\033[0m" $<
	@$(MKDIR_P) $(dir $@)
	$(GCC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
	@echo "\033[1;33mDone\n\033[0m"

# C++ BUILD INSTRUCTIONS:
$(BUILD_DIR)/%.cpp.o: %.cpp
	@echo "\033[0;32m[C++] \033[0m \033[0;33mCompiling:\033[0m" $<
	@$(MKDIR_P) $(dir $@)
	$(GXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
	@echo "\033[1;33mDone\n\033[0m"

# MAKE INSTALL
.PHONY: install
install: default
install: transfer
install: rexecute

# MAKE CLEAN
.PHONY: clean
clean:
	find . -name "*.o" -exec rm -f {} \;
	@echo "\033[1;33mClean done\033[0m"

# MAKE CLEAN_ALL
.PHONY: clean_all
clean_all:
	$(RM) -r $(BUILD_DIR) *.bin *.bak *.log*
	find . -name "*.o" -exec rm -f {} \;
	@echo "\033[1;33mClean done\033[0m"

# MAKE CLEAN_TARGET
.PHONY: clean_target
clean_target:
	@if [ "$(IP)" = "x.x.x.x" ]; then \
	echo "\033[1;31mIP argument must be set\033[0m";\
	exit 1;\
	fi
	@echo "cd $(TARGET_DIR) && $(RM) -r $(TARGET_EXE) *.bin *.bak *.log*"
	@ssh -p $(PORT) -t $(TARGET_USR)@$(IP) "cd $(TARGET_DIR) && $(RM) -r $(TARGET_EXE) *.bin *.bak *.log*" || true
	@echo "\033[1;33mClean done\033[0m"
	
# MAKE HELP
.PHONY: help
help:
	@echo "                      \n"
	@echo "[TARGET]         [DEFAULT ARGS]"
	@echo "make                                       - Build for AM57x target with all warnings off"
	@echo "                 DBG=-w                    - Compiler warning flags (-Wall for all)"
	@echo "                 LD_FLAGS=-ldeflibs        - Add additional libraries (DEFAULT=-lm\ -lncurses\ -lpthread\ -lrt\ -lstdc++)"
	@echo "                 INC_DIRS=                 - Manually add additional include paths"
	@echo "                 BUILD_NAME=struers.app    - Application name - binary executable file-name (build output binary)"
	@echo "                 BUILD_DIR=./build         - Build directory - path to directory of binary executable (build output path)"
	@echo "                 C_FLAGS=                  - C compiler flags (DEFAULT=none)"
	@echo "                 CXX_FLAGS=-std=c++11      - C++ compiler flags"
	@echo "                 GCC=arm-gnueabihf-gcc     - C ARM compiler"
	@echo "                 GXX=arm-gnueabihf-g++     - C++ ARM compiler"
	@echo "make clean                                 - Remove object files from project root-directory including sub-directories"
	@echo "make clean_all                             - Remove object files as in 'make clean' and delete build directory"
	@echo "                 BUILD_DIR=./build         - "
	@echo "make clean_target                          - Removes *.log, *.bin, *.bak and application from target application directory"
	@echo "                 IP=x.x.x.x                - Target SSH IP address"
	@echo "                 PORT=22                   - Target SSH port"
	@echo "                 TARGET_USR=root           - Target SSH user"
	@echo "                 TARGET_DIR=/app/bin       - Target application directory - path to directory of executable binary on target file-system"
	@echo "                 TARGET_EXE=struers.app    - Target executable name - executable binary file-name on target file-system"
	@echo "make execute                               - Setup virtual CAN and execute application locally (X86/X86_64)"
	@echo "                 BUILD_NAME=struers.app    - "
	@echo "                 BUILD_DIR=./build         - "
	@echo "make install                               - Build for AM57x target, transfer application to target and remote execute"
	@echo "                 IP=x.x.x.x                - "
	@echo "                 PORT=22                   - "
	@echo "                 TARGET_USR=root           - "
	@echo "                 TARGET_DIR=/app/bin       - "
	@echo "                 TARGET_EXE=struers.app    - "
	@echo "                 BIN_ONLY=true             - Only transfer binary executable"
	@echo "                 DEBUG_CFG=debug.cfg       - debug.cfg location"
	@echo "                 ROTATE_SH=rotate.sh       - rotate.sh location"
	@echo "                 BUILD_NAME=struers.app    - "
	@echo "                 BUILD_DIR=./build         - "
	@echo "make rexecute                              - Remote execute application"
	@echo "                 IP=x.x.x.x                - "
	@echo "                 PORT=22                   - "
	@echo "                 TARGET_USR=root           - "
	@echo "                 TARGET_DIR=/app/bin       - "
	@echo "                 TARGET_EXE=struers.app    - "
	@echo "make rsa                                   - Setup SSH RSA public key on host and target"
	@echo "                 IP=x.x.x.x                - "
	@echo "                 PORT=22                   - "
	@echo "                 TARGET_USR=root           - "
	@echo "                 RSA_PUB=~/.ssh/id_rsa.pub - SSH RSA public key location"
	@echo "make native                                   - Build for X86/X86_64 with all warnings off"
	@echo "                 DBG=-w                    - "
	@echo "                 LD_FLAGS=-ldeflibs        - "
	@echo "                 INC_DIRS=                 - "
	@echo "                 BUILD_NAME=struers.app    - "
	@echo "                 BUILD_DIR=./build         - "
	@echo "                 C_FLAGS=                  - "
	@echo "                 CXX_FLAGS=-std=c++11      - "
	@echo "                 GCC_NATIVE=/usr/bin/gcc      - C X86/X86_64 compiler"
	@echo "                 GXX_NATIVE=/usr/bin/g++      - C++ X86/X86_64 compiler"
	@echo "make transfer                              - Transfer application to target"
	@echo "                 IP=x.x.x.x                - "
	@echo "                 PORT=22                   - "
	@echo "                 TARGET_USR=root           - "
	@echo "                 TARGET_DIR=/app/bin       - "
	@echo "                 TARGET_EXE=struers.app    - "
	@echo "                 BIN_ONLY=true             - "
	@echo "                 DEBUG_CFG=debug.cfg       - "
	@echo "                 ROTATE_SH=rotate.sh       - "
	@echo "                 BUILD_NAME=struers.app    - "
	@echo "                 BUILD_DIR=./build         - \n"


# CHECK FOR GCC AND G++ COMPILERS AND RESOLVE
.PHONY: check_cc
check_cc:
	@if [ "$(GCC)" = "" ]; then \
	echo "\033[1;31mUnable to find GCC compiler\033[0m";\
	stty -echo;\
	read -p "Fetch with APT? [Y/n]" read_ans;\
	stty echo;\
	echo "";\
	if [ "$$read_ans" = "Y" ] || [ "$$read_ans" = "y" ] || [ "$$read_ans" = "" ]; then \
	sudo apt-get -y install gcc-5;\
	else \
	exit 1;\
	fi;\
	fi
	@if [ "$(GXX)" = "" ]; then \
	echo "\033[1;31mUnable to find G++ compiler\033[0m";\
	stty -echo;\
	read -p "Fetch with APT? [Y/n]" read_ans;\
	stty echo;\
	echo "";\
	if [ "$$read_ans" = "Y" ] || [ "$$read_ans" = "y" ] || [ "$$read_ans" = "" ]; then \
	sudo apt-get -y install g++-5;\
	else \
	exit 1;\
	fi;\
	fi
	$(eval GCC ?= $(shell which gcc))
	$(eval GXX ?= $(shell which g++))
	@echo "\033[1;36mC compiler set to $(GCC_NATIVE) \nC++ compiler set to $(GXX_NATIVE)\033[0m"

# CHECK FOR RSA PUBLIC KEY AND RESOLVE. THEN SEND RSA PUBLIC KEY TO TARGET
.PHONY: rsa
rsa:
	@if [ "$(IP)" = "x.x.x.x" ]; then \
	echo "\033[1;31mIP argument must be set\033[0m";\
	exit 1;\
	fi
	$(eval RSA_PUB2 := $(shell cat $(RSA_PUB) 2>&-))
	@if [ "$(RSA_PUB2)" = "" ]; then \
	echo "\033[1;31mNo RSA public key found at $(RSA_PUB)\n\033[0;32mCreating key.. (Press enter to all)\033[0m";\
	ssh-keygen -t rsa;\
	fi
	@echo "\033[0;32mRemounting target disk to -RW\033[0m"
	@ssh -p $(PORT) -t $(TARGET_USR)@$(IP) "mount -n -o remount -rw /" || true
	@echo "\033[0;32mTransfering RSA public key to target..\033[0m"
	@scp -P $(PORT) $(RSA_PUB) $(TARGET_USR)@$(IP):~/pubtmp || true
	@echo "\033[0;32mSetting up authorized keys on target..\033[0m"
	@ssh -p $(PORT) -t $(TARGET_USR)@$(IP) "mkdir -p .ssh && cat pubtmp >> ~/.ssh/authorized_keys && rm pubtmp && chmod 700 ~/.ssh/authorized_keys" || true
	@echo "\033[1;33mRSA setup finished\033[0m"

# TRANSFER APPLICATION TO TARGET
.PHONY: transfer
transfer:
	@if [ "$(IP)" = "x.x.x.x" ]; then \
	echo "\033[1;31mIP argument must be set\033[0m";\
	exit 1;\
	fi
	@echo "\033[0;32mTransfering application to target..\033[0m"
	@scp -P $(PORT) $(BUILD_DIR)/$(BUILD_NAME) $(TARGET_USR)@$(IP):$(TARGET_DIR)/$(TARGET_EXE) || true
	@scp -P $(PORT) $(TARGET_EXE) $(TARGET_USR)@$(IP):$(TARGET_DIR)/ || true
	@echo "\033[1;33mTransfer finished\033[0m"

# EXECUTE APPLICATION LOCALLY
.PHONY: execute
execute: native
execute:
	@sudo $(BUILD_DIR)/$(BUILD_NAME) || true
	@echo "\033[1;33mExecute finished\033[0m"

# EXECUTE APPLICATION REMOTELY
.PHONY: rexecute
rexecute:
	@if [ "$(IP)" = "x.x.x.x" ]; then \
	echo "\033[1;31mIP argument must be set\033[0m";\
	exit 1;\
	fi
	@echo "\033[0;32mRemote executing..\033[0m"
	@ssh -p $(PORT) -t $(TARGET_USR)@$(IP) "cd $(TARGET_DIR) && ./$(TARGET_EXE)" || true
	@echo "\033[1;33mRemote execute finished\033[0m"

# INCLUDE-DEPENDENCY CHECK FOR ARM
.PHONY: check_arm
check_arm:
	@if [ "$(GCC)" = "" ]; then \
	echo "\033[1;31mUnable to find arm-linux-gnueabihf-gcc compiler\033[0m";\
	exit 1;\
	elif [ "$(GXX)" = "" ]; then \
	echo "\033[1;31mUnable to find arm-linux-gnueabihf-g++ compiler\033[0m";\
	exit 1;\
	fi
	@echo "\033[1;36mC compiler set to $(GCC) \nC++ compiler set to $(GXX)\033[0m"

# CHECK HEADER FILES FOR DIFFERENCE SINCE LAST COMPILE
.PHONY: check_headers
check_headers:
	$(eval HEADERS_H := $(shell find -name '*.h' -newer $(BUILD_DIR)/$(BUILD_NAME) 2>&-))
	$(eval HEADERS_HPP := $(shell find -name '*.hpp' -newer $(BUILD_DIR)/$(BUILD_NAME) 2>&-))
	@if [ "$(HEADERS_H)" != "" ] || [ "$(HEADERS_HPP)" != "" ]; then \
	echo "\033[0;32mChanges in *.h/*.hpp files detected - recompiling..\033[0m";\
	find . -name "*.o" -exec rm -f {} \;;\
	fi

