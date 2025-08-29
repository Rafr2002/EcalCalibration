# EcalCalibration
ECal calibration scripts for the GEpV ECal

Analysis and calibration scripts for π⁰ → γγ reconstruction for the GEpV ECal.  
This repository contains macros and analysis tools developed to calibrate the ECal using simulated and experimental data.

## Contents
- `macros/` – G4SBS macros for π⁰ generation and ECal simulation
- `analysis/` – ROOT macros and C++ scripts for invariant mass reconstruction and calibration

## Dependencies
- [g4sbs](https://github.com/JeffersonLab/g4sbs)
- [SBS-offline](https://github.com/JeffersonLab/SBS-offline)
- [ROOT](https://root.cern/)

- `pi0_Ecal_Cal.mac` – Simulates π⁰ → γγ events in the calorimeter.  
- `analyze_pi0.C` – Reconstructs the invariant mass and related observables.

-------

## Environment Setup

Shell script: **`ecal_env_setup.zsh`**.
Defines a path (`ECALCALIB_PATH`) so that all macros and scripts from this repository
can be called directly from anywhere. Also updates `ROOT_INCLUDE_PATH` and `CPLUS_INCLUDE_PATH`
so ROOT and Geant4 can automatically find path scripts and macros.

## Usage
  # Run g4sbs simulation macro
  `g4sbs pi0_Ecal_Cal.mac`
  
  # Run analyzer calibration macro
  `analyzer ecal_pi0calib.C`
  
  # Call analyzer macro with/without arguments
  `analyzer 'ecal_pi0calib(1,2)'`


### Implementation
1) Clone repository

2) Open the ecal_env_setup.zsh file and replace /path/to/ with the actual clone location
  (ex. `/work/halla/sbs/User/EcalCalibration on ifarm`).

3) Add the following line to your ~/.zshrc or ~/.bashrc:
   `source /path/to/EcalCalibration/ecal_env_setup.zsh`
