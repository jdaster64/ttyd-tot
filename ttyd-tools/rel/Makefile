#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

ifeq ($(strip $(TOT_TTYDTOOLS)),)
$(error "Please set TOT_TTYDTOOLS in your environment. export TOT_TTYDTOOLS=<path to>ttyd-tools")
endif

include $(DEVKITPPC)/gamecube_rules

export ELF2REL	:=	$(TOT_TTYDTOOLS)/bin/elf2rel
export GCIPACK	:=	python $(TOT_TTYDTOOLS)/gcipack/gcipack.py
export DEBUG_SYMBOLS	:=	python $(TOT_TTYDTOOLS)/scripts/output_debug_symbols.py
export UPDATE_STRINGS	:=	python $(TOT_TTYDTOOLS)/scripts/update_text_files.py
export UPDATE_ICON_BIN  :=	python $(TOT_TTYDTOOLS)/scripts/update_icon_bin.py

ifeq ($(VERSION),)
all: us_full
us:
	@$(MAKE) --no-print-directory VERSION=us
	@$(MAKE) --no-print-directory copy_files VERSION=us
us_full:
	@$(MAKE) --no-print-directory VERSION=us
	@$(MAKE) --no-print-directory debug_symbols VERSION=us
	@$(MAKE) --no-print-directory update_strings VERSION=us
	@$(MAKE) --no-print-directory update_icon_bin VERSION=us
	@$(MAKE) --no-print-directory copy_files VERSION=us
jp:
	@$(MAKE) --no-print-directory VERSION=jp
eu:
	@$(MAKE) --no-print-directory VERSION=eu

clean:
	@$(MAKE) --no-print-directory clean_target VERSION=us
	@$(MAKE) --no-print-directory clean_target VERSION=eu
	@$(MAKE) --no-print-directory clean_target VERSION=jp

.PHONY: all clean us jp eu
else

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
TARGET		:=	$(notdir $(CURDIR)).$(VERSION)
BUILD		:=	build.$(VERSION)
SOURCES		:=	source $(wildcard source/*)
DATA		:=	data  
INCLUDES	:=	include

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------

MACHDEP		= -mno-sdata -mgcn -DGEKKO -mcpu=750 -meabi -mhard-float

CFLAGS		= -nostdlib -ffreestanding -ffunction-sections -fdata-sections -g -Os -Wall -Werror -Wno-address-of-packed-member $(MACHDEP) $(INCLUDE)
CXXFLAGS	= -fno-exceptions -fno-rtti -std=gnu++17 $(CFLAGS)

LDFLAGS		= -r -e _prolog -u _prolog -u _epilog -u _unresolved -Wl,--gc-sections -nostdlib -g $(MACHDEP) -Wl,-Map,$(notdir $@).map

# Platform options
ifeq ($(VERSION),us)
	CFLAGS += -DTTYD_US
	GAMECODE = "G8ME"
else ifeq ($(VERSION),jp)
	CFLAGS += -DTTYD_JP
	GAMECODE = "G8MJ"
else ifeq ($(VERSION),eu)
	CFLAGS += -DTTYD_EU
	GAMECODE = "G8MP"
endif


#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS	:= -lm -lgcc

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:=

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
			$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# automatically build a list of object files for our project
#---------------------------------------------------------------------------------
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
sFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES_BIN	:=	$(addsuffix .o,$(BINFILES))
export OFILES_SOURCES := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(sFILES:.s=.o) $(SFILES:.S=.o)
export OFILES := $(OFILES_BIN) $(OFILES_SOURCES)

export HFILES := $(addsuffix .h,$(subst .,_,$(BINFILES)))

# For REL linking
export LDFILES		:= $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.ld)))
export MAPFILE		:= $(CURDIR)/include/ttyd.$(VERSION).lst
export BANNERFILE	:= $(CURDIR)/banner.raw
export ICONFILE		:= $(CURDIR)/icon.raw

#---------------------------------------------------------------------------------
# build a list of include paths
#---------------------------------------------------------------------------------
export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD) \
			-I$(LIBOGC_INC)

#---------------------------------------------------------------------------------
# build a list of library paths
#---------------------------------------------------------------------------------
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
			-L$(LIBOGC_LIB)

export OUTPUT	:=	$(CURDIR)/$(TARGET)
.PHONY: $(BUILD) clean_target debug_symbols

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean_target:
	@echo clean ... $(VERSION)
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).dol $(OUTPUT).rel $(OUTPUT).gci

#---------------------------------------------------------------------------------
debug_symbols:
	@echo generating debug symbols ...
	@$(DEBUG_SYMBOLS) --in_elf=$(OUTPUT).elf --in_rel=$(OUTPUT).rel --out_dir=$(CURDIR)

#---------------------------------------------------------------------------------
update_strings:
	@echo updating text strings ...
	@$(UPDATE_STRINGS) --in_msg_dir=$(TOT_TTYDTOOLS)/bin/backup_files/msg/US --out_msg_dir=$(TOT_TTYDTOOLS)/bin/outputs/msg/US

#---------------------------------------------------------------------------------
update_icon_bin:
	@echo updating icon.bin data ...
	@$(UPDATE_ICON_BIN) --in_icon_bin=$(TOT_TTYDTOOLS)/bin/backup_files/icon.bin --out_dir=$(TOT_TTYDTOOLS)/bin/outputs/

#---------------------------------------------------------------------------------
copy_files:
	@echo copying .rel to output folder ...
	@cp $(OUTPUT).rel $(TOT_TTYDTOOLS)/bin/outputs/rel/gon.rel -f

#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
# $(OUTPUT).gci: $(OUTPUT).rel $(BANNERFILE) $(ICONFILE)
$(OUTPUT).rel: $(OUTPUT).elf $(MAPFILE)
$(OUTPUT).elf: $(LDFILES) $(OFILES)

$(OFILES_SOURCES) : $(HFILES)

# REL linking
%.rel: %.elf
	@echo output ... $(notdir $@)
	@$(ELF2REL) $< -s $(MAPFILE) --rel-id 8
	
# %.gci: %.rel
# @echo packing ... $(notdir $@)
# @$(GCIPACK) $< "rel" "Paper Mario" "TTYD Tower of Trials" $(BANNERFILE) $(ICONFILE) $(GAMECODE)
	
#---------------------------------------------------------------------------------
# This rule links in binary data with the .jpg extension
#---------------------------------------------------------------------------------
%.jpg.o	%_jpg.h :	%.jpg
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	$(bin2o)

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
endif