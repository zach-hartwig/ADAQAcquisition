#!/bin/bash
#
# name: setup.sh
# date: 01 Feb 15
# auth: Zach Hartwig
# desc: Bash script to configure the user's environment for running
#       ADAQAcquisition

# The HOSTTYPE variable is used to determine whether 32 or 64 bit for
# ADAQ and CAEN libraries should be used
export HOSTTYPE=$(uname -m)

if [ ! "$#" -eq 0 ]; then
    echo -e "\nADAQAcquisition : Error! This setup script accepts zero arguments!\n"
else
    
    SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )" 
    export PATH=${SCRIPTDIR///scripts/}/bin:$PATH
    export LD_LIBRARY_PATH=$ADAQHOME/lib/$HOSTTYPE:$LD_LIBRARY_PATH
    export PYTHONPATH=$ADAQHOME/lib/$HOSTTYPE:$PYTHONPATH
    
    echo -e "\nADAQAcquisition : The environment has been successfully configured!\n"
fi
