#!/bin/bash
#SBATCH --job-name=pi0Ecal
##SBATCH --output=../g4Outs/batches/pi0Ecal_%A_%a.out
##SBATCH --error=../g4Outs/batches/pi0Ecal_%A_%a.err
# Put logs into the same job_<arrayID> folder
#SBATCH --output=/volatile/halla/sbs/rfruiz/g4Outs/batches/logs/job_%x_%A/pi0Ecal_%A_%a.out
#SBATCH --error=/volatile/halla/sbs/rfruiz/g4Outs/batches/logs/job_%x_%A/pi0Ecal_%A_%a.err
##SBATCH --array=1-20
#SBATCH --array=1-100
#SBATCH --time=12:00:00
#SBATCH --partition=ifarm
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1

# Make sure modules are available
source /etc/profile.d/modules.sh

# Load environment
module use /group/halla/modulefiles
module load analyzer geant4 cmake
source /work/halla/sbs/rfruiz/SBS-offline/install/bin/sbsenv.sh
source /work/halla/sbs/rfruiz/g4sbs/install/bin/g4sbs.sh

# Number of events
# NEVENTS=5000
NEVENTS=1000
# NEVENTS=10

# Total events for the array job
TOTAL_EVENTS=$(( NEVENTS * SLURM_ARRAY_TASK_COUNT ))

bill=1000000000
mill=1000000
thous=1000

totalEventsName=$TOTAL_EVENTS

if [ "$TOTAL_EVENTS" -ge "$bill" ]; then
    totalEventsName="$(( (TOTAL_EVENTS + bill/2) / bill ))B"
elif [ "$TOTAL_EVENTS" -ge "$mill" ]; then
    totalEventsName="$(( (TOTAL_EVENTS + mill/2) / mill ))M"
elif [ "$TOTAL_EVENTS" -ge "$thous" ]; then
    totalEventsName="$(( (TOTAL_EVENTS + thous/2) / thous ))K"
fi

# Output directory (use volatile scratch space)
BASEOUT=/volatile/halla/sbs/rfruiz/g4Outs/batches


# Make a unique folder for this job submission
# OUTDIR="$BASEOUT/job_${SLURM_ARRAY_JOB_ID}"
OUTDIR="$BASEOUT/${totalEventsName}_job_${SLURM_ARRAY_JOB_ID}"
mkdir -p $OUTDIR
# mkdir -p $OUTDIR/logs

numEventsname=$NEVENTS

# Add rounding when dividing
if [ "$NEVENTS" -ge "$bill" ]; then
    numEventsname="$(( (NEVENTS + bill/2) / bill ))B"
elif [ "$NEVENTS" -ge "$mill" ]; then
    numEventsname="$(( (NEVENTS + mill/2) / mill ))M"
elif [ "$NEVENTS" -ge "$thous" ]; then
    numEventsname="$(( (NEVENTS + thous/2) / thous ))K"
fi

# Unique output file per job
# OUTFILE="$OUTDIR/${NEVENTS}_batch_pi0Ecal_${SLURM_ARRAY_TASK_ID}.root"
# OUTFILE="$OUTDIR/${numEventsname}_batch_pi0Ecal_${SLURM_ARRAY_TASK_ID}.root"
# OUTFILE="$OUTDIR/${numEventsname}_batch_pi0Ecal_${SLURM_ARRAY_TASK_ID}_${SLURM_JOB_ID}.root"
# OUTFILE="$OUTDIR/${numEventsname}_batch_pi0Ecal_${SLURM_ARRAY_JOB_ID}_${SLURM_ARRAY_TASK_ID}.root"
OUTFILE="$OUTDIR/${numEventsname}_batch_pi0Ecal_${SLURM_ARRAY_TASK_ID}_${SLURM_ARRAY_JOB_ID}.root"
# OUTFILE="$OUTDIR/${numEventsname}_batch_pi0Ecal_${SLURM_ARRAY_JOB_ID}_${SLURM_ARRAY_TASK_ID}.root"

# echo "[$(date)] Starting job on host $(hostname)"
# echo "[$(date)] Writing ${NEVENTS} events to: $OUTFILE"
# echo ""

# Run generator
./Nevent_pi0_Ecal_Cal.sh $NEVENTS $OUTFILE