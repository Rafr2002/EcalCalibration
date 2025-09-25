void filter_ECal_histos(const char* infile = "replayed_Digd_10K_pi0_Ecal_cal.root",
                        const char* outfile = "ECal_only_histos.root")
{
    TFile *fin  = TFile::Open(infile, "READ");
    if(!fin || fin->IsZombie()) {
        Error("filter_ECal_histos","Cannot open %s", infile);
        return;
    }

    TFile *fout = new TFile(outfile, "RECREATE");

    // Loop over all keys in the input file
    TIter next(fin->GetListOfKeys());
    TKey *key;
    while ((key = (TKey*)next())) {
        TObject *obj = key->ReadObj();

        // Keep only histograms
        if (obj->InheritsFrom("TH1") || obj->InheritsFrom("TH2")) {
            TH1 *h = (TH1*)obj;

            // Require non-empty AND name/title containing "ECal"
            if (h->GetEntries() > 0 &&
                (TString(h->GetName()).Contains("ECal") || TString(h->GetTitle()).Contains("ECal")))
            {
                fout->cd();
                h->Write();
                cout << "Saved: " << h->GetName() << endl;
            }
        }
    }

    fout->Close();
    fin->Close();

    cout << "Filtered histograms written to " << outfile << endl;
}