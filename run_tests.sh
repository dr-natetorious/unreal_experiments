#!/usr/bin/env bash
set -euo pipefail

ENGINE="${UE_EDITOR_BIN:-/apps/git/UnrealEngine/Engine/Binaries/Linux/UnrealEditor}"
UPROJECT="$(cd "$(dirname "$0")" && pwd)/DudeWalks.uproject"
FILTER="${TEST_FILTER:-DudeWalks}"
LOG_DIR="$(dirname "$0")/Saved/Logs"
mkdir -p "${LOG_DIR}"
LOGFILE="${LOG_DIR}/run-$(date -u +%Y%m%dT%H%M%SZ).log"

if [[ ! -x "${ENGINE}" ]]; then
  echo "UnrealEditor not found: ${ENGINE}" | tee "${LOGFILE}"
  echo "Set UE_EDITOR_BIN= to override." | tee -a "${LOGFILE}"
  exit 2
fi

echo "Running filter: ${FILTER}" | tee "${LOGFILE}"

set +e
"${ENGINE}" "${UPROJECT}" \
  -ExecCmds="Automation RunTests ${FILTER};Quit" \
  -NullRHI -Unattended -NoSplash -NoSound -log \
  2>&1 | tee -a "${LOGFILE}"
EXIT=${PIPESTATUS[0]}
set -e

if grep -Eq "Automation Test Failed|Errors while running" "${LOGFILE}"; then
  echo "FAIL" | tee -a "${LOGFILE}"; exit 1
fi
if grep -Eq "Automation Test Succeeded|All tests passed|Test Completed.*Passed" "${LOGFILE}"; then
  echo "PASS" | tee -a "${LOGFILE}"; exit 0
fi
if [[ ${EXIT} -ne 0 ]]; then
  echo "FAIL (exit ${EXIT})" | tee -a "${LOGFILE}"; exit ${EXIT}
fi
echo "UNKNOWN — no pass/fail markers in log" | tee -a "${LOGFILE}"
exit 3
