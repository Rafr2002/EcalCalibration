# EcalCalibration
ECal calibration scripts for the GEpV ECal

Analysis and calibration scripts for Pi0 → γγ reconstruction for the GEpV ECal.  
This repository contains macros and analysis tools developed to calibrate the ECal using simulated and experimental data.

## Contents
- `macros/` – G4SBS macros for π⁰ generation and ECal simulation
- `analysis/` – ROOT macros and C++ scripts for invariant mass reconstruction and calibration
- `docs/` – Notes and references related to π⁰ calibration and GEp filtering

## Dependencies
- [g4sbs](https://github.com/JeffersonLab/g4sbs)
- [SBS-offline](https://github.com/JeffersonLab/SBS-offline)
- [ROOT](https://root.cern/)

## Usage
1. Run the provided G4SBS macros to simulate Pi0 → γγ events in the calorimeter.
2. Use the analysis scripts to reconstruct invariant mass spectra and perform calibration.
3. Apply calibration constants to filter out non-electron events in GEp data.
