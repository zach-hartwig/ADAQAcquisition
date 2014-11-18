#!/bin/bash
#
# name: license.sh
# date: 17 Nov 14
# auth: Zach Hartwig
# mail: hartwig@psfc.mit.edu
# desc: Bash script to add license header to all files

HeaderFiles="$(find ../include -name '*.hh')"
SourceFiles="$(find ../src -name '*.cc')"

for File in $HeaderFiles
do
    echo -e $File
done


cat > /tmp/tmp.txt << EOL
/////////////////////////////////////////////////////////////////////////
//                                                                     //
//   Copyright (C) 2014 ; Zachary Seth Hartwig ; All rights reserved   //
//                                                                     //
// The ADAQAcquisition source code is licensed under the GNU GPL v3.0  //
// You have the right to modify and/or redistribute under the terms of //
// this license, which may be found at $ADAQACQUISITION/License.txt or //
// online at http://www.gnu.org/licenses.                              //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

EOL


