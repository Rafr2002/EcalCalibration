#!/bin/bash


#############################################################################
# 
# THIS SCRIPT IS FOR SHIFT CREW.  
#
# It just take one argument as input which is the CODA run number.
# Analyzes 50k events, make Panguin plots, HALOG the results.
#
# Last updated: 05/01/2025 by Anuruddha Rathnayake.
#############################################################################


# Just a single user input that takes the CODA run number.
runnum=$1

# Check if the  user is inputing the arguments correctly and if not provide help.
if  [ "$#" -ne 1 ] ||  ! [[ "$1" =~ ^[0-9]+$ ]] ; then
  echo "EROOR!: Invalid input."
  echo "Usage: ShiftCrew-50k-GEp.sh  <CODA run#>"
  exit 1
fi

first_event=0           #Begin analysis with the very first CODA event. 
nevents=50000           #Analyze 50K events.
nevents_per_job=5000    #Change as needed.
CM_plots=0              #Do we need CM plots enabled or disabled in 50K?
nseg_replay=5           #Let's assume there is only the first CODA file segment ready by the time shift crew run this script. And also it consists of more than 50K events.
n_machines=1            #Input 1 to just run on aonl1. Input 3 to run on ALL three aonl machines (aonl1, aon2, and aonl3).


### FIRST WE RUN THE ANALYSIS WITHOUT GEMS TO BE SURE HCAL-ECAL TIMING LOOKS GOOD. PROCEED WITH GEM ANALYSIS ONLY IF IT IS GOOD ###
# echo ""
# echo "50k analysis is starting WITHOUT GEMs..."
# echo ""

############################ REPLAY WITH NO GEMS ############################################################
# Call the script that splits the jobs and submit them.
#aonl2-replayjobs_gep.sh $runnum $first_event $nevents $nevents_per_job $CM_plots 0 $nseg_replay $n_machines

# Exit if the 'multimachine-replayjobs_gep.sh' exited with an error.
# if [ $? -ne 0 ]; then
#     exit 1
# fi

### Make Panguin plots and HALOG them ###
# Define variables needed for making the Panguin plots.
# ROOTFILE=~/sbs/Rootfiles/gep5_replayed_nogems_${runnum}_50k_events.root

# # Update the hadded ROOT file to include any additional histograms.
# update_hadded_rootfile.sh $ROOTFILE

# golden_runnum=1234 # SET GOLDEN RUN NUMBER HERE!
# GOLDENROOTFILE=~/sbs/Rootfiles/gep5_replayed_nogems_${golden_runnum}_50k_events.root
# PLOTS_DIR=/chafs2/work1/sbs/plots/

# panguin_plots_gep.sh $runnum "50k" $ROOTFILE $GOLDENROOTFILE $PLOTS_DIR 1 0
# #############################################################################################################

# #This function is called when prompting the user
# function yes_or_no(){
#   while true; do
#     read -p "$* [y/n]: " yn
#     case $yn in
#       [Yy]*) return 0 ;;
#       [Nn]*) echo "No entered. GEM analysis will not proceed." ; echo "ATTENTION: If you have concerns that ECal and HCal timing plots do not look good, please contact experts!" ; exit 1;;
#     esac
#   done
# }

# yes_or_no "Please confirm ECal and HCal timing plots looks good?"

echo ""
echo "50k analysis is starting WITH GEMs..."
echo ""

############################ REPLAY WITH GEMS ###############################################################
# Call the script that splits the jobs and submit them.
aonl2-replayjobs_gep.sh $runnum $first_event $nevents $nevents_per_job $CM_plots 1 $nseg_replay $n_machines

# Exit if the 'multimachine-replayjobs_gep.sh' exited with an error.
if [ $? -ne 0 ]; then
    exit 1
fi

### Make Panguin plots and HALOG them ###
# Define variables needed for making the Panguin plots.
ROOTFILE=~/sbs/Rootfiles/gep5_replayed_${runnum}_50k_events.root

golden_runnum=1234 # SET GOLDEN RUN NUMBER HERE!
GOLDENROOTFILE=~/sbs/Rootfiles/gep5_replayed_${golden_runnum}_50k_events.root
PLOTS_DIR=/chafs2/work1/sbs/plots/

panguin_plots_gep.sh $runnum "50k" $ROOTFILE $GOLDENROOTFILE $PLOTS_DIR 1 1
############################################################################################################

# Let's move the GEM align info files to a separate directory remove the clutter in the top level directory.
gemalign_localdir=gemalign

if [ -f GEM_alignment_info_sbs_gemFT_run${runnum}.txt ]; then
  mv GEM_alignment_info_sbs_gemFT_run${runnum}.txt $gemalign_localdir
  # echo "GEM alignment info file for FT can be found at the folder: "${gemalign_localdir}
  # echo ""
fi

if [ -f GEM_alignment_info_sbs_gemFPP_run${runnum}.txt ]; then
  mv GEM_alignment_info_sbs_gemFPP_run${runnum}.txt $gemalign_localdir
  # echo "GEM alignment info file for FPP can be found at the folder: "${gemalign_localdir}
  # echo ""
fi
