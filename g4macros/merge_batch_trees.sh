#!/bin/bash
# merge_batch_trees.sh
# Merge TTrees from batch ROOT files, ignoring histograms

# Usage: ./merge_batch_trees.sh <output_file.root> <input_files>
# Example: ./merge_batch_trees.sh _Hadded_batch_pi0Ecal_.root 1K_batch_pi0Ecal_*.root

OUTFILE=$1
shift
FILES=$@

if [ -z "$OUTFILE" ] || [ -z "$FILES" ]; then
    echo "Usage: $0 <output.root> <input_files...>"
    exit 1
fi

echo "Merging TTrees into $OUTFILE ..."
echo "Input files: $FILES"

root -l -b -q <<EOF
{
    // Replace "T" with the actual TTree name if needed
    TChain ch("T");
    int nfiles = ch.Add("$FILES");
    if (nfiles == 0) {
        std::cerr << "ERROR: No input files matched." << std::endl;
        gSystem->Exit(1);
    }
    std::cout << "Added " << nfiles << " files to chain." << std::endl;
    ch.Merge("$OUTFILE");
}
EOF
