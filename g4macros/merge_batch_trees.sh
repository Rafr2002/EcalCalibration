#!/bin/bash
# merge_batch_trees.sh
# Merge TTrees from batch ROOT files, ignoring histograms
# Counts files, reports missing indices, and names output with formatted total event count

# Usage: ./merge_batch_trees.sh <input_files>
# Example: ./merge_batch_trees.sh 1K_batch_pi0Ecal_*.root

FILES=$@
if [ -z "$FILES" ]; then
    echo "Usage: $0 <input_files...>"
    exit 1
fi

##*File Naming convention:

# Constants for suffix conversion
bill=1000000000
mill=1000000
thous=1000

# Count how many .root files match
NFOUND=$(ls $FILES 2>/dev/null | wc -l)
echo "Found $NFOUND ROOT files matching input pattern."

# Detect per-file event count from first matching filename
SAMPLE=$(ls $FILES 2>/dev/null | head -n1)

# Default to 1000 if not detected
PERFILE=1000

if [[ "$SAMPLE" =~ ^([0-9]+)K_ ]]; then
    PERFILE=$(( BASH_REMATCH[1] * 1000 ))
elif [[ "$SAMPLE" =~ ^([0-9]+)M_ ]]; then
    PERFILE=$(( BASH_REMATCH[1] * 1000000 ))
elif [[ "$SAMPLE" =~ ^([0-9]+)_ ]]; then
    PERFILE=${BASH_REMATCH[1]}
fi

echo "Detected $PERFILE events per file (from sample: $SAMPLE)"

# Extract numeric indices from filenames (assumes ..._<N>_*.root)
INDICES=$(ls $FILES 2>/dev/null | sed -E 's/.*_pi0Ecal_([0-9]+)_.*/\1/' | sort -n)
MAXIDX=$(echo "$INDICES" | tail -n 1)

echo "Detected max task number: $MAXIDX"

# Report missing task numbers
MISSING=()
for i in $(seq 1 $MAXIDX); do
    fname=$(echo "$SAMPLE" | sed -E "s/_[0-9]+_/_${i}_/")  # replace index with $i
    if [ ! -f "$fname" ]; then
        MISSING+=($i)
    fi
done

if [ ${#MISSING[@]} -gt 0 ]; then
    echo "Missing task numbers (1..$MAXIDX): ${MISSING[@]}"
else
    echo "No missing files in range 1..$MAXIDX."
fi

# Compute total events = (#found files × events per file)
NEVENTS=$((NFOUND * PERFILE))

# Format total events into human-readable suffix
numEventsname=$NEVENTS
if [ "$NEVENTS" -ge "$bill" ]; then
    numEventsname="$(( (NEVENTS + bill/2) / bill ))B"
elif [ "$NEVENTS" -ge "$mill" ]; then
    numEventsname="$(( (NEVENTS + mill/2) / mill ))M"
elif [ "$NEVENTS" -ge "$thous" ]; then
    numEventsname="$(( (NEVENTS + thous/2) / thous ))K"
else
    numEventsname="$NEVENTS"
fi

# Build output filename with formatted prefix
OUTFILE="${numEventsname}_Hadded_batch_pi0Ecal.root"

echo "Total events: $NEVENTS → formatted as $numEventsname"
echo "Final output file will be: $OUTFILE"
echo "Merging TTrees into $OUTFILE ..."
echo "Input files: $FILES"

root -l -b -q <<EOF
{
    TChain ch("T");  // Replace "T" with actual TTree name if needed
    int nfiles = ch.Add("$FILES");
    if (nfiles == 0) {
        std::cerr << "ERROR: No input files matched." << std::endl;
        gSystem->Exit(1);
    }
    std::cout << "Added " << nfiles << " files to chain." << std::endl;
    ch.Merge("$OUTFILE");
}
EOF
