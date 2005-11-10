##########################################################################
#
#    Mp4Edit Program
#
#    (c) 2001-2002 Gilles Boccon-Gibod
#
##########################################################################
all: mp4edit

##########################################################################
# includes
##########################################################################
include $(BUILD_ROOT)/Makefiles/Crypto.exp
include $(BUILD_ROOT)/Makefiles/Core.exp
include $(BUILD_ROOT)/Makefiles/System.exp

##########################################################################
# targets
##########################################################################
TARGET_SOURCES = Mp4Edit.cpp

##########################################################################
# make path
##########################################################################
VPATH += $(SOURCE_ROOT)/Apps/Mp4Edit

##########################################################################
# includes
##########################################################################
include $(BUILD_ROOT)/Makefiles/Rules.mak

##########################################################################
# rules
##########################################################################
mp4edit: $(TARGET_OBJECTS) $(TARGET_LIBRARY_FILES)
	$(LINK) $(TARGET_OBJECTS) $(LIBRARIES_CPP) -o $@ 


