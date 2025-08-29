# ------------------------ ECal Cal Script Environment Setup ------------------------

# Canonical re`po location (edit per machine if needed)
export ECALCALIB="$HOME/EcalCalibration"

# Chainable search path for macros + analysis scripts
export ECALCALIB_PATH="$ECALCALIB/macros:$ECALCALIB/analysis:$ECALCALIB_PATH"

# Extend include paths so ROOT and Geant4 see your code automatically
export ROOT_INCLUDE_PATH="$ECALCALIB/analysis:$ROOT_INCLUDE_PATH"
export CPLUS_INCLUDE_PATH="$ECALCALIB/include:$CPLUS_INCLUDE_PATH"

# Helper: resolve a script against ECALCALIB_PATH
_resolve_ecal_script() {
  local script="$1"
  IFS=':' read -ra dirs <<< "$ECALCALIB_PATH"
  for d in "${dirs[@]}"; do
    if [[ -f "$d/$script" ]]; then
      echo "$d/$script"
      return 0
    fi
  done
  # if not found, return as-is
  echo "$script"
  return 1
}

# Wrapper for g4sbs (so macros can be called by name only)
g4sbs() {
  if [[ -n "$1" ]]; then
    local resolved=$(_resolve_ecal_script "$1")
    shift
    command g4sbs "$resolved" "$@"
  else
    command g4sbs "$@"
  fi
}

# Wrapper for analyzer (so macros can be called by name only)
analyzer() {
  if [[ -n "$1" ]]; then
    local resolved=$(_resolve_ecal_script "$1")
    shift
    command analyzer "$resolved" "$@"
  else
    command analyzer "$@"
  fi
}
