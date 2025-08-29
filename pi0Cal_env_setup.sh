# ------------------------ ECal Cal Script Environment Setup ------------------------

# Cloned Repo location 
export ECALCALIB="$HOME/EcalCalibration"

# Search path for all scripts in this repository macros + analysis scripts
export ECALCALIB_PATH="$ECALCALIB/macros:$ECALCALIB/analysis:$ECALCALIB_PATH"

# Add to include paths so ROOT and Geant4 see your code automatically
export ROOT_INCLUDE_PATH="$ECALCALIB/analysis:$ROOT_INCLUDE_PATH"
export CPLUS_INCLUDE_PATH="$ECALCALIB/include:$CPLUS_INCLUDE_PATH"

# Runs when something call a script from ECALCALIB_PATH
_find_ecal_script() {
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

#  Allow macro to be called by g4sbs
g4sbs() {
  if [[ -n "$1" ]]; then
    local resolved=$(_resolve_ecal_script "$1")
    shift
    command g4sbs "$resolved" "$@"
  else
    command g4sbs "$@"
  fi
}

# Allows script to be called by analyzer
analyzer() {
  if [[ -n "$1" ]]; then
    local resolved=$(_resolve_ecal_script "$1")
    shift
    command analyzer "$resolved" "$@"
  else
    command analyzer "$@"
  fi
}
