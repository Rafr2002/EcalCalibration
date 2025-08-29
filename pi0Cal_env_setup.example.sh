# ------------------------ ECal Cal Script Environment Setup ------------------------

# Cloned Repo location 
export ECALCALIB="Path/to/EcalCalibration"

# Search path for all scripts in this repository macros + analysis scripts
export ECALCALIB_PATH=""
export ECALCALIB_PATH="$ECALCALIB/g4macros:$ECALCALIB/analyzer_scripts:$ECALCALIB_PATH"

# Add to include paths so ROOT and Geant4 see your code automatically
export ROOT_INCLUDE_PATH="$ECALCALIB/analyzer_scripts:$ROOT_INCLUDE_PATH"
export CPLUS_INCLUDE_PATH="$ECALCALIB/g4macros:$CPLUS_INCLUDE_PATH"

#path to g4sbs executable
export PATH="Path/To/g4sbs/Exec:$PATH"

# Runs when something call a script from ECALCALIB_PATH
_find_ecal_script() {
  local script="$1"
  # split ECALCALIB_PATH into array
  local dirs
  dirs=(${(s/:/)ECALCALIB_PATH})

  for d in $dirs; do
    if [[ -f "$d/$script" ]]; then
      echo "$d/$script"
      return 0
    fi
  done

  # if not found, return original name
  echo "$script"
  return 1
}

#  Allow macro to be called by g4sbs
g4sbs() {
  if [[ -n "$1" ]]; then
    local found=$(_find_ecal_script "$1")
    shift
    command g4sbs "$found" "$@"
  else
    command g4sbs "$@"
  fi
}

# Allows script to be called by analyzer
analyzer() {
  if [[ -n "$1" ]]; then
    local found=$(_find_ecal_script "$1")
    shift
    command analyzer "$found" "$@"
  else
    command analyzer "$@"
  fi
}
