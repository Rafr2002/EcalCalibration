#!/bin/bash
# merge_batch.sh

JOBDIR=$1   # e.g. 100K_job_54238854
cd "$JOBDIR" || exit 1

# Extract pieces from folder name
TOTAL_EVENTS=$(echo $JOBDIR | cut -d'_' -f1)   # "100K"
JOB_ID=$(echo $JOBDIR | cut -d'_' -f3)         # "54238854"

OUTFILE="${TOTAL_EVENTS}_Hadded_batch_pi0Ecal_${JOB_ID}.root"

echo "Merging into $OUTFILE ..."
hadd "$OUTFILE" *_batch_pi0Ecal_*.root
