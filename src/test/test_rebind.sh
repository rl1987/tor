#!/bin/sh

set -x

exitcode=0


if [ "$TESTING_TOR_BINARY" = "" ] ; then
  echo "TESTING_TOR_BINARY not set"
  exit 1
fi

"${PYTHON:-python}" "${abs_top_srcdir:-.}/src/test/test_rebind.py" "${TESTING_TOR_BINARY}" || exitcode=1

exit ${exitcode}
