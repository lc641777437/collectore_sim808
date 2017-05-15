##########################################################################
# File:             user.mak 
# Description:      User config file.
# Version:          0.1 
# Author:           Maobin
# DATE              NAME                        DESCRIPTION
# 2013-01-10        Maobin                      Initial Version 0.1
#########################################################################

#RVCT 3.1 Root Directory
DIR_ARM  = $(RVCT31)

BIN = app

#Core file
CORE_DIR = core

#project directory
SRC_DIR = user

#output directory
OUTPUT_BIN_DIR = output
OUTPUT_LIB_DIR = output/obj
OUTPUT_LOG_DIR = output/log

#project name in project directory
SRC_OPEN_MODE_NAME = main common 3rd

# Macro definition
# FEA_APP_EXAMPLE_OPT = -DAPP_EXAMPLE
# how-to use in src
# #ifdef APP_EXAMPLE
# do something...;
# #endif

ifneq ("$(findstring true,$(DEBUG))","")
FEA_APP_DEBUG = -DAPP_DEBUG
endif

####################### WMMP begin ###########################
#FEA_APP_WMMP_OPT = -D__SIMCOM_WMMP__
#eat wmmp interface
#FEA_APP_WMMP_INTERFACE_OPT = -D__SIMCOM_EAT_WMMP__
#wmmp lib feature
#FEA_APP_WMMP30_OPT = -D__MODEM_WMMP30_SIMCOM__
#FEA_APP_WMMP_TEST_PLATFORM_OPT = -D__SIMCOM_WMMP_TEST_PLATFORM__
####################### WMMP end ############################
#FEA_APP_MULTI_APP_OPT = -D__SIMCOM_EAT_MULTI_APP__
 #First app or second app
#FEA_APP_MULTI_APP_INDEX_APP_OPT = -D__EAT_SECOND_APP__ #__EAT_FIRST_APP__

FEA_APP_CFLAGS = $(FEA_APP_DEBUG) \
				 $(FEA_APP_WMMP_OPT) \
				 $(FEA_APP_WMMP_INTERFACE_OPT) \
				 $(FEA_APP_WMMP30_OPT) \
				 $(FEA_APP_WMMP_TEST_PLATFORM_OPT) \
				 $(FEA_APP_MULTI_APP_OPT) \
				 $(FEA_APP_MULTI_APP_INDEX_APP_OPT)

