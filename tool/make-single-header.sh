#!/usr/bin/bash

ORIGIN_DIR="$(pwd)"
SHELL_DIR=$(cd "$(dirname "$0")";pwd)
CONFIG_DIR="${SHELL_DIR}/make-single-header"
TARGET="${SHELL_DIR}"/../single-include/decoterm.hpp

cd "${SHELL_DIR}"
"${CONFIG_DIR}"/amalgamate/amalgamate.py -s "${SHELL_DIR}"/../include -c "${CONFIG_DIR}"/config-amalgamate.json -p "${CONFIG_DIR}"/prologue.hpp
"${SHELL_DIR}"/process-target.py "${TARGET}" $@
cd "${ORIGIN_DIR}"
