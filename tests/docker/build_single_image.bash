#----------------------------------------------------------------------------------------------
#  Copyright (c) 2020 - present Alexander Voitenko
#  Licensed under the MIT License. See License.txt in the project root for license information.
#----------------------------------------------------------------------------------------------

#!/bin/bash

trap exit ERR

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
SCRIPT_NAME=$(basename "${BASH_SOURCE[0]}")

set -u

DOCKERFILE_PATH=${1}
source "${SCRIPT_DIR}/env.bash"

pushd ${SCRIPT_DIR}
docker build -t ${DOCKER_REGISTRY_PREFIX}:$(basename ${DOCKERFILE_PATH}) -f ${DOCKERFILE_PATH} .
popd
