#!/bin/bash

echo "Setting GENIE environment variables......"
export GENIEBASE=/opt
export GENIE=$GENIEBASE/GENIE
export PYTHIA6=$GENIEBASE/pythia/v6_428/lib
export ROOTSYS=$GENIEBASE/root
export LHAPATH=$GENIEBASE/lhapdf_install
export LHAPDF_INC=$GENIEBASE/lhapdf_install/include
export LHAPDF_LIB=$GENIEBASE/lhapdf_install/lib
export XSECSPLINEDIR=$GENIEBASE/data
export LD_LIBRARY_PATH=$LHAPDF_LIB:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$PYTHIA6:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$GENIE:$LD_LIBRARY_PATH
export PATH=$GENIE/bin:$ROOTSYS/bin:$PATH
unset GENIEBASE
