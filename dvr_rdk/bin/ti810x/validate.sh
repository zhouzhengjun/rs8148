#!/bin/sh
#Script validates kernel bootargs.
#Incase of failure returns 0 else 1
if test "`grep -i mem=${RDK_LINUX_MEM} /proc/cmdline`" = ""
then
  echo "*** This RDK version requires mem=${RDK_LINUX_MEM}M in bootargs ***"
  exit 0
else
  echo "*** Bootargs Validated for mem param ***"
fi

if test "`grep -i notifyk.vpssm3_sva=${NOTIFYK_VPSSM3_SVA_ADDR} /proc/cmdline`" = ""
then
  echo "*** This RDK version requires notifyk.vpssm3_sva=${NOTIFYK_VPSSM3_SVA_ADDR} in bootargs ***"
  exit 0
else
  echo "*** Bootargs Validated for notifyk.vpssm3 params ***"
fi

exit 1