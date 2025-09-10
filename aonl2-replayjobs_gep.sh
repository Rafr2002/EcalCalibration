#!/bin/bash
#set -u


###################################################################################################################################
#
#Rafael Ruiz edited replau_gep_mc_ECALONLY.C to make this script work for ECAL calibration data
# Original script produce by Anuruddha Rathnayake (anuruddha@uconn.edu, adr@jlab.org) by adapting a script written by Sean Jeffas. 
# Last Updated: April 1, 2025
#
# The purpose of this script is to split up analysis jobs across one or more aonl computers for faster completion of analysis.
# This scrip is not meant to be run by itself. Rather it will be called by higher level shell scripts like 'ShiftCrew-50k-GEp.sh'.
#
# The usage:
# multimachine-replayjobs_gep.sh <runnumber> <first_event> <nevents> <nevents_per_job> <CM_plots> <nseg_replay> <n_machines>
# <runnumber> = CODA run #
# <first_event> = first event to analyze
# <nevents> = number of events to analyze. Input -1 to trigger a full replay.
# <nevents_per_job> = number of events to analyze in a single analysis job. Input -1 to make 'hadd' work for full replay.
# <CM_plots> = Input 0 for single stream and 2 for 3-stream DAQ mode.
# <nseg_replay> = Number of CODA file segments to replay. If -1 is input. All the file segments available will be used.
# <n_machines> = Put 1 for just aonl1. 2 for aonl1 and aonl2. 3 for aonl, aonl2, and aonl3.
#
###################################################################################################################################


#This function is called when prompting the user
function yes_or_no(){
  while true; do
    read -p "$* [y/n]: " yn
    case $yn in
      [Yy]*) return 0 ;;
      [Nn]*) echo "No entered" ; exit 1;;
    esac
  done
}

runnumber=$1
first_event=$2
nevents=$3
nevents_per_job=$4
CM_plots=$5
dogems=$6
nseg_replay=$7
n_machines=$8
do_hadd_rootfiles=$9 

if [ -z "$nseg_replay" ]
then
    nseg_replay=1
fi

if [ -z "$do_hadd_rootfiles" ]
then
    do_hadd_rootfiles=1
fi

#These variables define how we want our jobs to be distributed
max_jobs=200 # What is the maximum allowed number of jobs that should be submitted per machine?
nseg=-1      # Set to -1 to start.
maxstream=0  # Set to 0 to start.

fname_prefix='gep5' # Set to 'gep5' to analyze main DAQ experiment data.

# Set where the data is stored. 
# export DATA_DIR=/adaqeb1/data1:/adaqeb2/data1:/adaqeb3/data1:/cache/halla/sbs/GEp/raw
# export DATA_DIR=/volatile/halla/sbs/rfruiz:/work/halla/sbs/rfruiz/EcalCalibration/analyzer_scripts
export DATA_DIR=/volatile/halla/sbs/rfruiz


# Loop over EVIO files to see how many segments there are
iseg=0
nseg=-1
while :; do
    found=false
        
    for dir in ${DATA_DIR//:/ }; do
        STREAM1_FILE="${dir}/${fname_prefix}_${runnumber}.evio.0.${iseg}"
        
        if [[ -f "$STREAM1_FILE" ]]; then
            found=true
            break
        fi
    done

    if [[ "$found" == false ]]; then
        break
    fi
       
    nseg=$iseg
    ((iseg++))
done

# Check if stream 1 exists
for dir in ${DATA_DIR//:/ }; do
    STREAM2_FILE="${dir}/${fname_prefix}_${runnumber}.evio.1.0"
    if [[ -f "$STREAM2_FILE" ]]; then
        maxstream=1
        break
    fi
done

# Check if stream 2 exists
for dir in ${DATA_DIR//:/ }; do
    STREAM3_FILE="${dir}/${fname_prefix}_${runnumber}.evio.2.0"
    if [[ -f "$STREAM3_FILE" ]]; then
        maxstream=2
        break
    fi
done

echo "CODA stream = $((maxstream+1))"

# Exit if there are no EVIO files
if (($nseg == -1)); then
   echo "No CODA EVIO files found for run $runnumber"
   exit
fi

# Make $nseg_replay equal to the number of segments if it is input to be -1.
if (($nseg_replay == -1)); then
    nseg_replay=$nseg
fi

njobs=$(($nevents  / $nevents_per_job))
#Get number of segments for each machine to process
job_per_machine=$((($njobs) / $n_machines));
#job_remainder=$((($njobs) % $n_machines));
job_remainder=0
job_per_machine=$njobs

# Only replay segments that actually exist
if (($nseg < $nseg_replay)); then
    nseg_replay=$nseg
fi

# if nevents = -1 then set things up to do a full replay
if (($nevents == -1)); then
    njobs=$(($nseg_replay + 1))   # Add 1 because segments start at 0
    #job_per_machine=$((($njobs) / $n_machines));
    #job_remainder=$((($njobs) % $n_machines));
    job_per_machine=$njobs
fi

#This is a special case of running < $n_machines segments
#if (($job_per_machine == 0 && $job_remainder != 0)); then
#    n_machines=$job_remainder
#fi

#Limit the number of segments to replay to the maximum
if (($job_per_machine > $max_jobs)); then
    njobs=$(($max_jobs * $n_machines))
    job_per_machine=$max_jobs
    nevents=$(($njobs * $nevents_per_job))
    if (($nevents == -1)); then
	echo "User entered more segments than possible. Will instead replay "$nseg_replay" segments."
    else
	echo "User entered more events than possible. Will instead replay "$nevents" events."
    fi
fi

if (($nevents == 50000)); then
    yes_or_no "This will replay 50k events of run "$runnumber" with "$njobs" jobs. Are you sure you want to continue?"
elif (($nevents == 1000000)); then
    yes_or_no "This will replay 1M events of run "$runnumber" with "$njobs" jobs. This run number MUST HAVE 1M EVENTS for this to work. Are you sure you want to continue?"
elif (($nevents == -1)); then
    yes_or_no "This will launch a FULL replay of run "$runnumber" with "$njobs" jobs. Please run a full replay only once a shift. Are you sure you want to continue?"
else
    yes_or_no "This will replay "$nevents" events of run "$runnumber" with "$njobs" jobs. Are you sure you want to continue?"
fi

ievents=$first_event
start_seg=0  #count segments if we are doing full replays

time_replay_1=`date +%s`

# Loop over machines and start running the jobs
for ((imachine=1; imachine <= n_machines; imachine++))
do

    # If we are running less jobs than machines then just run all the jobs on the last machine
    if (($job_per_machine < n_machines));then
	    imachine=$n_machines
	fi

    start_event=$ievents
    end_event=$((start_event+job_per_machine*nevents_per_job)) 

    end_seg=$((start_seg+job_per_machine-1)) #subtract 1 because segments start from 0.

    print_n_jobs=$job_per_machine #This variable is just for printing info to the terminal

    
    # If we are on the last machine then add the remainder of jobs here
    if (($imachine == $n_machines));then
        end_event=$((end_event+job_remainder*nevents_per_job)) 
        end_seg=$((start_seg+job_per_machine+job_remainder-1)) #subtract 1 because segments start from 0.            
        print_n_jobs=$((print_n_jobs+job_remainder))
    fi
    
    echo "Submitted "$print_n_jobs" jobs on aonl2"
    
    n_jobs=$(((end_event-start_event)/nevents_per_job))  #calculate the number of jobs
    n_segs_thisjob=$((end_seg-start_seg+1))
    
    if (($nevents == -1)); then
        # Open an xterm for this machine and start the replays 
        if (($imachine == $n_machines));then
            xterm -e "ssh a-onl@aonl2 'source ~/.bashrc && cd ~/sbs_tools && submit-replayjobs_gep.sh "$runnumber" "$first_event" "$nevents" "$n_segs_thisjob" "$start_seg" "$end_seg" "$maxstream" "$CM_plots" "$dogems"'"
        else
            xterm -e "ssh a-onl@aonl2 'source ~/.bashrc && cd ~/sbs_tools && submit-replayjobs_gep.sh "$runnumber" "$first_event" "$nevents" "$n_segs_thisjob" "$start_seg" "$end_seg" "$maxstream" "$CM_plots" "$dogems"'" &
        fi
    else
        # Open an xterm for this machine and start the replays 
        if (($imachine == $n_machines));then
            xterm -e "ssh a-onl@aonl2 'source ~/.bashrc && cd ~/sbs_tools && submit-replayjobs_gep.sh "$runnumber" "$start_event" "$nevents_per_job" "$n_jobs" 0 "$nseg_replay" "$maxstream" "$CM_plots" "$dogems"'"
        else
            xterm -e "ssh a-onl@aonl2 'source ~/.bashrc && cd ~/sbs_tools && submit-replayjobs_gep.sh "$runnumber" "$start_event" "$nevents_per_job" "$n_jobs" 0 "$nseg_replay" "$maxstream" "$CM_plots" "$dogems"'" &
        fi
    fi

    #set starting segment for the next machine
    ievents=$(($end_event))
    start_seg=$(($end_seg + 1))

done

wait

echo "Finished all replays"

time_replay_2=`date +%s`

if (($do_hadd_rootfiles == 1)); then
   #Now we will add all the replays to one file for panguin plots
    combine_files_gep.sh $runnumber $first_event $(($first_event + $nevents)) $nevents_per_job $nseg_replay $maxstream $dogems

    time_replay_3=`date +%s`

    echo "Replay time = "$(($time_replay_2 - $time_replay_1))" s"
    echo "Hadd time = "$(($time_replay_3 - $time_replay_2))" s"

else

    echo "Replay time = "$(($time_replay_2 - $time_replay_1))" s"

fi