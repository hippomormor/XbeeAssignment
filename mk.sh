#!/bin/bash

SHFS() {
  echo -e "\033[0;32mChecking for SSHFS..\033[0m"
  if [ "`which sshfs`" != "" ]; then
   echo -e "\033[1;36mSSHFS found at `which sshfs`\033[0m"
  else
   sudo apt-get -y install sshfs
  fi
  if [ -z "$2" ] || [ -z "$3" ] || [ -z "$4" ] || [ -z "$5" ] || [ -z "$6" ]; then
    echo -e "\033[1;31m\nPlease input target directory (Default: /app/bin): \033[0m"
    read -p "" remote_dir
    if [ "$remote_dir" == "" ]; then
      remote_dir="/app/bin"
    fi
    echo -e "\033[1;31m\nPlease input where to mount target application directory (Default: ./remote): \033[0m"
    read -p "" local_dir
    if [ "$local_dir" == "" ]; then
      local_dir="./remote"
    fi
    echo -e "\033[1;31mInput target IP:\033[0m"
    read -p "" IP
    if [ "$IP" = "" ]; then
      echo -e "\033[1;31mNo IP entered - Dropping to shell..\033[0m"
      exit 1
    fi
    echo -e "\033[1;31m\nInput target port (Default: 22):\033[0m"
    read -p "" port
    if [ "$port" = "" ]; then
      port=22
    fi
    echo -e "\033[1;31mInput target user (Default: root):\033[0m"
    read -p "" user
    if [ "$user" = "" ]; then
      user="root"
    fi
  else
    user="$2"
    IP="$3"
    port="$4"
    local_dir="$5"
    remote_dir="$6"
  fi
  sudo modprobe fuse 2>&-
  sudo addgroup fuse 2>&-
  sudo adduser "$USER" fuse 2>&- 1>&-
  sudo chown root:fuse /dev/fuse 2>&-
  sudo chmod +x /dev/fuse 2>&-
  mkdir "$local_dir" 2>&-
  sudo sed -Ei 's/#user_allow_other/user_allow_other/g' /etc/fuse.conf
  sshfs -p "$port" "$user"@"$IP":"$remote_dir" "$local_dir" -o allow_other
  echo -e "\033[1;33mDone\033[0m"
}

SSHFSunmount() {
  echo -e "\033[0;32mUnmounting "$2"..\033[0m"
  fusermount -u "$2"
  echo -e "\033[1;33mDone\033[0m"
  exit 0
}

Genwhitelist() {
  find ./ -name "*.c" > s1.ex
  echo -e "\033[1;36mTotal `cat s1.ex | wc -l` .c files\033[0m"
  find ./ -name "*.cpp" > s2.ex
  echo -e "\033[1;36mTotal `cat s2.ex | wc -l` .cpp files\033[0m"
  cat s1.ex s2.ex > s3.ex
  sort s3.ex > source.in
  rm s1.ex s2.ex s3.ex
  echo -e "\033[1;33mSource.in now generated (total `cat source.in | wc -l` files)\033[0m"
}

Genblacklist() {
  touch source.ex
  echo -e "\033[1;33mSource.ex now generated\033[0m"
}

Dispwhitelist() {
  if [ -e source.in ]; then
    echo -e "\033[1;36mCurrent white-list:\033[0m"
    cat source.in
    echo -e "\033[1;33mTotal `cat source.in | wc -l` files\033[0m"
  else
    echo -e "\033[1;31mNo white-list exist\033[0m"
  fi
}

Dispblacklist() {
  if [ -e source.ex ]; then
    echo -e "\033[1;36mCurrent black-list:\033[0m"
    cat source.ex
    echo -e "\033[1;33mTotal `cat source.ex | wc -l` files\033[0m"
  else
    echo -e "\033[1;31mNo black-list exist\033[0m"
  fi
}

Delblacklist() {
  if [ -e "source.ex" ]; then
    rm source.ex
    echo -e "\033[1;33mDeleted black-list\033[0m"
  else
    echo -e "\033[1;31mNo black-list exist\033[0m"
  fi
}

Delwhitelist() {
  if [ -e "source.in" ]; then
    rm source.in
    echo -e "\033[1;33mDeleted white-list\033[0m"
  else
    echo -e "\033[1;31mNo white-list exist\033[0m"
  fi
}

AddFileToblacklist() {
  # cross-check that the file requested already exists
  if [ -e "source.ex" ]; then
    while read p; do
      if [ "$p" == "$1" ]; then
        echo -e "\033[1;31m$p already exists - ignored\033[0m"
        return
      fi
    done < source.ex
  fi
  echo $1 >> source.ex
  echo -e "\033[1;33mAdded $1 to black-list (now `cat source.ex | wc -l` entries)\033[0m"
}

RemoveFileFromblacklist() {
  if [ -e "source.ex" ]; then
    if [ -z `cat source.ex | grep -w "$1"` ]; then
      echo -e "\033[1;31mFile does not exist in black-list\033[0m"
      return
    fi
    cat source.ex | grep -vw "$1" > newblacklist.ex
    mv newblacklist.ex source.ex
    echo -e "\033[1;33mRemoved $1 from black-list (now `cat source.ex | wc -l` entries)\033[0m"
  else
    echo -e "\033[1;31mNo black-list exist\033[0m"
  fi
}

AddFileTowhitelist() {
  # cross-check that the file requested already exists
  if [ -e "source.in" ]; then
    while read p; do
      if [ "$p" == "$1" ]; then
        echo -e "\033[1;31m$p already exists - ignored\033[0m"
        return
      fi
    done < source.in
  fi
  echo $1 >> source.in
  echo -e "\033[1;33mAdded $1 to white-list (now `cat source.in | wc -l` entries)\033[0m"
}

RemoveFileFromwhitelist() {
  if [ -e "source.in" ]; then
    if [ -z `cat source.in | grep -w "$1"` ]; then
      echo -e "\033[1;31mFile does not exist in white-list\033[0m"
      return
    fi
    cat source.in | grep -vw "$1" > newwhitelist.in
    mv newwhitelist.in source.in
    echo -e "\033[1;33mRemoved $1 from white-list (now `cat source.in | wc -l` entries)\033[0m"
  else
    echo -e "\033[1;31mNo white-list exist\033[0m"
  fi
}

SearchArmCompiler() {
  echo -e "\033[0;32mSearching for cross compilers..\033[0m"
  gccpath=$(locate -b 'arm-linux-gnueabihf-gcc' | grep "04.00.00.04" | grep "arm-linux-gnueabihf-gcc" | sort -u | head -n 1)
  gxxpath=$(locate -b 'arm-linux-gnueabihf-g++' | grep "04.00.00.04" | grep "arm-linux-gnueabihf-g++" | sort -u | head -n 1)
  if [ "$gccpath" == "" ] || [ "$gxxpath" == "" ]; then
    echo -e "\033[1;31mUnable to find compiler\033[0m\n"
  else
    echo -e "\033[1;36mC compiler set to $gccpath \nC++ compiler set to $gxxpath\033[0m\n"
  fi
}

SearchNativeCompiler() {
  echo -e "\033[0;32mSearching for native compilers..\033[0m"
  gccnativepath=$(which gcc)
  gxxnativepath=$(which g++)
  if [ "$gccnativepath" == "" ] || [ "$gxxnativepath" == "" ]; then
    echo -e "\033[1;31mUnable to find compiler\033[0m\n"
  else
    echo -e "\033[1;36mC compiler set to $gccnativepath \nC++ compiler set to $gxxnativepath\033[0m\n"
  fi
}

CompileIndividually() {
  out_file="$1"
  if [ "$2" = "ARM" ]; then
    SearchArmCompiler
    gcc="$gccpath"
    gxx="$gxxpath"
  elif [ "$2" = "X86" ]; then
    SearchNativeCompiler
    gcc="$gccnativepath"
    gxx="$gxxnativepath"
  else
    echo -e "\033[1;31mUnsupported architecture\033[0m"
    exit 1
  fi
  find . -name "*.o" -exec rm -f {} \;
  header_dirs=$(find -name '*.h' -printf '%h\n' | sort -u)
  hpp_dirs=$(find -name '*.hpp' -printf '%h\n' | sort -u)
  deps=$(printf " -I%s/" $header_dirs)
  deps_hpp=$(printf " -I%s/" $hpp_dirs)
  skipped=0
  while read p; do
    if [ -n "`cat source.ex | grep -w $p`" ]; then
      echo -e "\033[0;32mskipping\033[0m $p"
      ((skipped++))
      continue
    fi
    if [[ $p == *".c" ]]; then
    echo -e "\033[0;32mCompiling C\033[0m" $p
    $gcc $deps $deps_hpp -O0 -g3 -w -c -fmessage-length=0 $p
    else
    echo -e "\033[0;32mCompiling C++\033[0m" $p
    $gxx $deps $deps_hpp -O0 -g3 -w -c -std=c++11 -fmessage-length=0 $p
    fi
  done < source.in
  find . -name "*.o" > objects.all
  echo -e "\033[0;32mLinking... \033[0m(`cat objects.all | wc -l` objects)"
  $gxx -o $out_file @objects.all -lm -std=c++11 -lncurses -lpthread -lrt -lstdc++
  rm objects.all
  find . -name "*.o" -exec rm -f {} \;
  echo -e "$skipped skipped"
  echo -e "\033[1;33mDone\n\033[0m"
}

blacklistCommands() {
  if [ -n "$2" ]; then
    for Opt_B_mod in "$@"
    do
      if [ "$1" = "-Ba" ] && [ "$1" != "$Opt_B_mod" ]; then
        AddFileToblacklist $Opt_B_mod
      elif [ "$1" = "-Br" ] && [ "$1" != "$Opt_B_mod" ]; then
        RemoveFileFromblacklist $Opt_B_mod
      fi
    done
  fi
  exit 0
}

whitelistCommands() {
  if [ -n "$2" ]; then
    for Opt_B_mod in "$@"
    do
      if [ "$1" = "-Wa" ] && [ "$1" != "$Opt_B_mod" ]; then
        AddFileTowhitelist $Opt_B_mod
      elif [ "$1" = "-Wr" ] && [ "$1" != "$Opt_B_mod" ]; then
        RemoveFileFromwhitelist $Opt_B_mod
      fi
    done
  fi
  exit 0
}

BuildCMAKE() {
  echo -e "\033[0;32mRunning CMAKE..\033[0m" $p
  mkdir -p build
  cd build
  cmake ..
  make
  cd ..
  echo -e "\033[1;33mDone - build located in ./build\033[0m"
}

##### main program

main() {
  no_args=0
  if [ $# -eq 0 ]; then
    no_args=1
  fi

  err=0
  while [ -n "$1" ]; do
    case "$1" in
      -C)
        Opt_C=1;;
      -W)
        Opt_W=1;;
      -Wl)
        Opt_Wl=1;;
      -Wx)
        Opt_Wx=1;;
      -G)
        Opt_G=1;;
      -B)
        Opt_B=1;;
      -Ba | -Br)
        blacklistCommands "$@";;
      -Wa | -Wr)
        whitelistCommands "$@";;
      -Bl)
        Opt_Bl=1;;
      -Bx)
        Opt_Bx=1;;
      -Ci_X86)
        Opt_Ci_X86=1;;
      -Ci_ARM)
        Opt_Ci_ARM=1;;
      -R)
        SSHFS "$@"
        exit 0;;
      -Ru)
        SSHFSunmount "$@";;
      *) err=1;;
    esac
    shift
  done

  if [ "$err" -ne 0 ] || [ "$no_args" = 1 ]; then
    echo "Tool for compiling code for ARM & X86/X86_64"
    echo " "
    echo "Major options:"
    echo " -B Generate empty black-list"
    echo " -Ba <module> add module to black-list"
    echo " -Br <module> remove module from black-list"
    echo " -C Compile with CMAKE using CMakeLists.txt"
    echo " -Ci_ARM Compile individually for ARM (without makefile)"
    echo " -Ci_X86 Compile individually for X86/X86_64 (without makefile)"
    echo " -G Generate Makefile"
    echo " -R <user> <x.x.x.x> <port> <local path> <target path> Setup remote SSHFS directory"
    echo " -Ru <path> Unmount SSHFS directory"
    echo " -W Update/generate new white-list (preserve black-list)"
    echo " -Wa <module> add module to white-list"
    echo " -Wr <module> remove module from white-list"
    echo " "
    echo "Maintainence options:"
    echo " -Bl Output black-list to stdout"
    echo " -Bx Discard existing black-list"
    echo " -Wl Output white-list to stdout"
    echo " -Wx Discard existing white-list"
    echo " "
    exit 1
  fi

  if [ "$Opt_B" = 1 ]; then
    Genblacklist
    exit 0

  elif [ "$Opt_W" = 1 ]; then
    Genwhitelist  
    exit 0
  
  elif [ "$Opt_Wl" = 1 ]; then
    Dispwhitelist
    exit 0
  
  elif [ "$Opt_Bl" = 1 ]; then
    Dispblacklist
    exit 0
  
  elif [ "$Opt_Bx" = 1 ]; then
    Delblacklist
    exit 0

  elif [ "$Opt_Wx" = 1 ]; then
    Delwhitelist
    exit 0

  elif [ "$Opt_Ci_ARM" = 1 ]; then
    CompileIndividually "app" "ARM"
    exit 0

  elif [ "$Opt_Ci_X86" = 1 ]; then
    CompileIndividually "app" "X86"
    exit 0

  elif [ "$Opt_C" = 1 ]; then
    BuildCMAKE
    exit 0

  elif [ "$Opt_G" = 1 ]; then
    SearchArmCompiler
    SearchNativeCompiler
    GenerateMakefile
    exit 0
  fi

  exit 0
}

# generate makefile dependant on source.in and source.ex

GenerateMakefile() {
includes=source.in
excludes=source.ex


cat <<EOT > makefile
################################################################################
####################### AUTOMATICALLY GENERATED MAKEFILE #######################
################################################################################

# BUILD VARIABLES:
EOT

if [ -z $name ] ; then
  name=wireless_project1
fi

echo -e "\033[1;36mApplication name:\n\033[0m$name"
echo "BUILD_NAME ?= $name" > makefile

cat <<EOT >> makefile
# Build variables
BUILD_DIR ?= ./build
SRC_DIRS ?= ./
IP ?= x.x.x.x
DBG ?= -w
TARGET_DIR ?= /
TARGET_EXE ?= \$(BUILD_NAME)
TARGET_USR ?= pi
PORT ?= 22
MKDIR_P ?= mkdir -p
RSA_PUB ?= ~/.ssh/id_rsa.pub

EOT

index=0
while read line; do
  in+="$line "
done < $includes

echo -e "\033[1;36m\nSource includes:\n\033[0m$in"

index=0
while read line; do
  ex+="$line "
done < $excludes
echo -e "\033[1;36m\nSource excludes:\n\033[0m$ex"

echo -e "\033[1;36m\nInclude directories:\033[0m"
find -name '*.h' -printf '%h\n' | sort -u
find -name '*.hpp' -printf '%h\n' | sort -u

cat <<EOT >> makefile
# INCLUDES:
SRCS := $in

# EXCLUDES:
EXCLUDES := $ex

# FILTER OUT EXCLUDES FROM SOURCES:
TMPSRCS := \$(SRCS)
SRCS := \$(filter-out \$(EXCLUDES), \$(TMPSRCS))

EOT

cat <<EOT >> makefile
# OBJECT LIST:
OBJS := \$(SRCS:%=\$(BUILD_DIR)/%.o)

EOT

cat <<EOT >> makefile
# INCLUDE PATHS:
INCDIRS += \$(shell find -name '*.h' -printf '%h\n' | sort -u)
INCDIRS += \$(shell find -name '*.hpp' -printf '%h\n' | sort -u)
INC_FLAGS := \$(addprefix -I,\$(INCDIRS))
CPPFLAGS += \$(INC_FLAGS) \$(INC_DIRS) \$(DBG)

EOT

cat <<EOT >> makefile
# LINKER FLAGS:
LD_FLAGS_DEF := -lm -lpthread -lrt -lstdc++
LDFLAGS += \$(LD_FLAGS) \$(LD_FLAGS_DEF)

# DEFAULT COMPILERs:
GCC ?= $gccpath
GXX ?= $gxxpath
GCC_NATIVE ?= $gccnativepath
GXX_NATIVE ?= $gxxnativepath

# COMPILER FLAGS
CFLAGS += \$(C_FLAGS)
CXXFLAGS += -std=c++11 \$(CXX_FLAGS)

# MAKE DEFAULT
.PHONY: default
default: check_arm check_headers \$(BUILD_DIR)/\$(BUILD_NAME) ; @echo "\033[1;33mBuild finished\033[0m"

# MAKE NATIVE
.PHONY: native
native: check_cc check_headers \$(BUILD_DIR)/\$(BUILD_NAME); @echo "\033[1;33mBuild finished\033[0m"
native: GCC := \$(GCC_NATIVE)
native: GXX := \$(GXX_NATIVE)
	

# LINKER INSTRUCTIONS:
\$(BUILD_DIR)/\$(BUILD_NAME): \$(OBJS)
	@echo "\033[0;32m[Linker]\033[0m \033[0;33mLinking object files into binary executable:\033[0m"
	@echo \$(GXX) "<object files>" -o \$@ \$(LDFLAGS)
	@\$(GXX) \$(OBJS) -o \$@ \$(LDFLAGS)
	@echo "\033[1;33mDone\n\033[0m\n\033[1;36mTarget built successfully!\n\033[0m\$(BUILD_DIR)/\$(BUILD_NAME)\n\033[0m"

# C BUILD INSTRUCTIONS:
\$(BUILD_DIR)/%.c.o: %.c
	@echo "\033[0;32m[C] \033[0m \033[0;33mCompiling:\033[0m" \$<
	@\$(MKDIR_P) \$(dir \$@)
	\$(GCC) \$(CPPFLAGS) \$(CFLAGS) -c \$< -o \$@
	@echo "\033[1;33mDone\n\033[0m"

# C++ BUILD INSTRUCTIONS:
\$(BUILD_DIR)/%.cpp.o: %.cpp
	@echo "\033[0;32m[C++] \033[0m \033[0;33mCompiling:\033[0m" \$<
	@\$(MKDIR_P) \$(dir \$@)
	\$(GXX) \$(CPPFLAGS) \$(CXXFLAGS) -c \$< -o \$@
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
	\$(RM) -r \$(BUILD_DIR) *.bin *.bak *.log*
	find . -name "*.o" -exec rm -f {} \;
	@echo "\033[1;33mClean done\033[0m"

# MAKE CLEAN_TARGET
.PHONY: clean_target
clean_target:
	@if [ "\$(IP)" = "x.x.x.x" ]; then \\
	echo "\033[1;31mIP argument must be set\033[0m";\\
	exit 1;\\
	fi
	@echo "cd \$(TARGET_DIR) && \$(RM) -r \$(TARGET_EXE) *.bin *.bak *.log*"
	@ssh -p \$(PORT) -t \$(TARGET_USR)@\$(IP) "cd \$(TARGET_DIR) && \$(RM) -r \$(TARGET_EXE) *.bin *.bak *.log*" || true
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
	@if [ "\$(GCC)" = "" ]; then \\
	echo "\033[1;31mUnable to find GCC compiler\033[0m";\\
	stty -echo;\\
	read -p "Fetch with APT? [Y/n]" read_ans;\\
	stty echo;\\
	echo "";\\
	if [ "\$\$read_ans" = "Y" ] || [ "\$\$read_ans" = "y" ] || [ "\$\$read_ans" = "" ]; then \\
	sudo apt-get -y install gcc-5;\\
	else \\
	exit 1;\\
	fi;\\
	fi
	@if [ "\$(GXX)" = "" ]; then \\
	echo "\033[1;31mUnable to find G++ compiler\033[0m";\\
	stty -echo;\\
	read -p "Fetch with APT? [Y/n]" read_ans;\\
	stty echo;\\
	echo "";\\
	if [ "\$\$read_ans" = "Y" ] || [ "\$\$read_ans" = "y" ] || [ "\$\$read_ans" = "" ]; then \\
	sudo apt-get -y install g++-5;\\
	else \\
	exit 1;\\
	fi;\\
	fi
	\$(eval GCC ?= \$(shell which gcc))
	\$(eval GXX ?= \$(shell which g++))
	@echo "\033[1;36mC compiler set to \$(GCC_NATIVE) \nC++ compiler set to \$(GXX_NATIVE)\033[0m"

# CHECK FOR RSA PUBLIC KEY AND RESOLVE. THEN SEND RSA PUBLIC KEY TO TARGET
.PHONY: rsa
rsa:
	@if [ "\$(IP)" = "x.x.x.x" ]; then \\
	echo "\033[1;31mIP argument must be set\033[0m";\\
	exit 1;\\
	fi
	\$(eval RSA_PUB2 := \$(shell cat \$(RSA_PUB) 2>&-))
	@if [ "\$(RSA_PUB2)" = "" ]; then \\
	echo "\033[1;31mNo RSA public key found at \$(RSA_PUB)\n\033[0;32mCreating key.. (Press enter to all)\033[0m";\\
	ssh-keygen -t rsa;\\
	fi
	@echo "\033[0;32mRemounting target disk to -RW\033[0m"
	@ssh -p \$(PORT) -t \$(TARGET_USR)@\$(IP) "mount -n -o remount -rw /" || true
	@echo "\033[0;32mTransfering RSA public key to target..\033[0m"
	@scp -P \$(PORT) \$(RSA_PUB) \$(TARGET_USR)@\$(IP):~/pubtmp || true
	@echo "\033[0;32mSetting up authorized keys on target..\033[0m"
	@ssh -p \$(PORT) -t \$(TARGET_USR)@\$(IP) "mkdir -p .ssh && cat pubtmp >> ~/.ssh/authorized_keys && rm pubtmp && chmod 700 ~/.ssh/authorized_keys" || true
	@echo "\033[1;33mRSA setup finished\033[0m"

# TRANSFER APPLICATION TO TARGET
.PHONY: transfer
transfer:
	@if [ "\$(IP)" = "x.x.x.x" ]; then \\
	echo "\033[1;31mIP argument must be set\033[0m";\\
	exit 1;\\
	fi
	@echo "\033[0;32mTransfering application to target..\033[0m"
	@scp -P \$(PORT) \$(BUILD_DIR)/\$(BUILD_NAME) \$(TARGET_USR)@\$(IP):\$(TARGET_DIR)/\$(TARGET_EXE) || true
	@scp -P \$(PORT) \$(TARGET_EXE) \$(TARGET_USR)@\$(IP):\$(TARGET_DIR)/ || true
	@echo "\033[1;33mTransfer finished\033[0m"

# EXECUTE APPLICATION LOCALLY
.PHONY: execute
execute: native
execute:
	@sudo \$(BUILD_DIR)/\$(BUILD_NAME) || true
	@echo "\033[1;33mExecute finished\033[0m"

# EXECUTE APPLICATION REMOTELY
.PHONY: rexecute
rexecute:
	@if [ "\$(IP)" = "x.x.x.x" ]; then \\
	echo "\033[1;31mIP argument must be set\033[0m";\\
	exit 1;\\
	fi
	@echo "\033[0;32mRemote executing..\033[0m"
	@ssh -p \$(PORT) -t \$(TARGET_USR)@\$(IP) "cd \$(TARGET_DIR) && ./\$(TARGET_EXE)" || true
	@echo "\033[1;33mRemote execute finished\033[0m"

# INCLUDE-DEPENDENCY CHECK FOR ARM
.PHONY: check_arm
check_arm:
	@if [ "\$(GCC)" = "" ]; then \\
	echo "\033[1;31mUnable to find arm-linux-gnueabihf-gcc compiler\033[0m";\\
	exit 1;\\
	elif [ "\$(GXX)" = "" ]; then \\
	echo "\033[1;31mUnable to find arm-linux-gnueabihf-g++ compiler\033[0m";\\
	exit 1;\\
	fi
	@echo "\033[1;36mC compiler set to \$(GCC) \nC++ compiler set to \$(GXX)\033[0m"

# CHECK HEADER FILES FOR DIFFERENCE SINCE LAST COMPILE
.PHONY: check_headers
check_headers:
	\$(eval HEADERS_H := \$(shell find -name '*.h' -newer \$(BUILD_DIR)/\$(BUILD_NAME) 2>&-))
	\$(eval HEADERS_HPP := \$(shell find -name '*.hpp' -newer \$(BUILD_DIR)/\$(BUILD_NAME) 2>&-))
	@if [ "\$(HEADERS_H)" != "" ] || [ "\$(HEADERS_HPP)" != "" ]; then \\
	echo "\033[0;32mChanges in *.h/*.hpp files detected - recompiling..\033[0m";\\
	find . -name "*.o" -exec rm -f {} \;;\\
	fi

EOT

echo -e "\033[1;33m\nMakefile generated - Run 'make help' for more information. Run 'make rsa' to setup SSH RSA public key on host and target\033[0m"
}


# Run main() at bottom when all code has been read
# Pass shell arguments $@
main "$@"
