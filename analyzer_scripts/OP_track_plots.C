//======================================================================
// **Usage:
//
// *Just Plot (default restriction=2, default file):
//   .L OP_track_plots.C+
//   OP_track_plots();
//
// *Change restriction but use default file:
//   OP_track_plots(3);
//
// *Change restriction + save histos (custom outfname):
//   OP_track_plots(1,"/Users/rr/jlabHallASoftware/EcalCalibration/analyzer_scripts/10K_pi0_Ecal_cal.root",
//                     "restrict1_out.root");
//
// *Compare restrict modes (overlay mass, angle, dE, or dR):
//   .L OP_track_plots.C+
//   compare_restrict_modes();            // default = "mass", default file
//   compare_restrict_modes("angle");     // override variable only
//   compare_restrict_modes("dE","/path/to/other.root"); // override both
//======================================================================

// Headers
#include <TFile.h>
#include <TTree.h>
#include <TH1I.h>
#include <TH2I.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TProfile.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <vector>
#include <map>
#include <iostream>
#include <TVector3.h>
#include <TLorentzVector.h>
#include <TLegend.h>
// End headers section

// --------------------------------------------------------------
// Global restriction setting
// 0 = any photon
// 1 = require MPID == 111 (parent is pi0)
// 2 = require MID == 1 (true daughters of primary pi0)
// 3 = require both MID==1 && MPID==111
int restrictMode = 3;

TString restrictLabel(int mode) {
  // if (restrictMode==0) return "Mode 0: All photons";
  // if (restrictMode==1) return "Mode 1: MPID==111 (pi0 parent)";
  // if (restrictMode==2) return "Mode 2: MID==1 (primary pi0 daughters)";
  // if (restrictMode==3) return "Mode 3: MID==1 && MPID==111";
  if (mode==0) return "Mode 0: All";
  if (mode==1) return "Mode 1: pi0 parent";
  if (mode==2) return "Mode 2: daughters";
  if (mode==3) return "Mode 3: dau+parent";
  return "Unknown";
  return "Unknown mode";
}


// Helper: apply restriction
bool passPhotonSelection(int pid, int mid, int mpid) {
  if (pid != 22) return false; // must be photon
  if (restrictMode == 0) return true;
  if (restrictMode == 1) return (mpid == 111);
  if (restrictMode == 2) return (mid == 1);
  if (restrictMode == 3) return (mid == 1 && mpid == 111);
  return true;
}

// --------------------------------------------------------------
// Main plotting + optional ROOT output
// If outfname == "" → only plotting, no file output
void OP_track_plots(
  int restriction = 2,
  const char *fname="/Users/rr/jlabHallASoftware/EcalCalibration/analyzer_scripts/10K_pi0_Ecal_cal.root",
                    const char *outfname="") {

  restrictMode = restriction;

  // Open file and tree
  TFile *f = TFile::Open(fname);
  if (!f) { std::cerr << "Could not open file " << fname << std::endl; return; }
  TTree *T = (TTree*) f->Get("T");
  if (!T) { std::cerr << "Tree T not found!" << std::endl; return; }

  // Geometry
  double z_calo   = 6.0;      // m
  double z_target = -0.1003;  // m

  // Branches
  int ntracks;
  std::vector<int> *TID=0,*MID=0,*PID=0,*MPID=0;
  std::vector<double> *posx=0,*posy=0,*posz=0;
  std::vector<double> *momx=0,*momy=0,*momz=0;
  std::vector<double> *Etot=0;

  T->SetBranchAddress("OTrack.ntracks", &ntracks);
  T->SetBranchAddress("OTrack.TID", &TID);
  T->SetBranchAddress("OTrack.MID", &MID);
  T->SetBranchAddress("OTrack.PID", &PID);
  T->SetBranchAddress("OTrack.MPID", &MPID);
  T->SetBranchAddress("OTrack.posx", &posx);
  T->SetBranchAddress("OTrack.posy", &posy);
  T->SetBranchAddress("OTrack.posz", &posz);
  T->SetBranchAddress("OTrack.momx", &momx);
  T->SetBranchAddress("OTrack.momy", &momy);
  T->SetBranchAddress("OTrack.momz", &momz);
  T->SetBranchAddress("OTrack.Etot", &Etot);

  // Histograms
  TH1D *hPhE   = new TH1D("hPhotonE","Photon Energy [GeV];E [GeV];Counts",100,0,7);
  TH1D *hTheta = new TH1D("hTheta","Photon polar angle #theta [deg];#theta [deg];Counts",90,0,45);
  TH1D *hPhi   = new TH1D("hPhi","Photon azimuthal angle #phi [deg];#phi [deg];Counts",90,-180,180);
  TH2D *hXY    = new TH2D("hXY","Photon hit positions;x [m];y [m]",100,-4,4,100,-2,2);

  // TH1D *hPi0Mass    = new TH1D("hPi0Mass","#pi^{0} invariant mass;M_{#gamma#gamma} [GeV];Counts",100,0,0.15);
  TH1D *hPi0Mass    = new TH1D("hPi0Mass","#pi^{0} invariant mass;M_{#gamma#gamma} [GeV];Counts",30,0,0.15);
  TH1D *hOpeningAng = new TH1D("hOpenAngle","Opening angle between photons;#theta_{12} [#deg];Counts",50,0,40);
  TH1D *hDR         = new TH1D("hDR","Distance between photons dR [m];dR [m];Counts",100,0.0,3.0);
  TH1D *hDE         = new TH1D("hDE","Energy difference |E1-E2|;|E1-E2| [GeV];Counts",100,0,7);

  TH2D *hAngVsDE    = new TH2D("hAngVsDE","Opening angle vs #DeltaE;#theta_{12} [deg];|E1-E2| [GeV]",100,0,40,100,0,7);
  TH2D *hDrVsDE     = new TH2D("hDrVsDE","Photon separation vs #DeltaE;dR [m];|E1-E2| [GeV]",100,0,3,100,0,7);
  TH2D *hAngVsMass  = new TH2D("hAngVsMass","Opening angle vs M_{#gamma#gamma};#theta_{12} [deg];M_{#gamma#gamma} [GeV]",100,0,40,100,0,0.3);
  TH2D *hDrVsMass   = new TH2D("hDrVsMass","Photon separation vs M_{#gamma#gamma};dR [m];M_{#gamma#gamma} [GeV]",100,0,3,100,0,0.3);

  // Loop
  Long64_t nentries = T->GetEntries();
  for(Long64_t ie=0; ie<nentries; ie++){
    T->GetEntry(ie);

    // collect photons
    std::vector<std::pair<TVector3,TLorentzVector>> photons;
    for(int j=0;j<ntracks;j++){
      if(!passPhotonSelection(PID->at(j),MID->at(j),MPID->at(j))) continue;
      TVector3 mom(momx->at(j), momy->at(j), momz->at(j));
      TLorentzVector ph(mom, Etot->at(j));
      photons.push_back({TVector3(posx->at(j),posy->at(j),posz->at(j)), ph});

      // Fill single-photon hists
      hPhE->Fill(Etot->at(j));
      hTheta->Fill(mom.Theta()*180.0/TMath::Pi());
      hPhi->Fill(mom.Phi()*180.0/TMath::Pi());
      hXY->Fill(posx->at(j),posy->at(j));
    }

    // loop over pairs
    for(size_t a=0;a<photons.size();a++){
      for(size_t b=a+1;b<photons.size();b++){
        TLorentzVector ph1=photons[a].second;
        TLorentzVector ph2=photons[b].second;
        double mgg=(ph1+ph2).M();

        double ang = ph1.Vect().Angle(ph2.Vect())*180.0/TMath::Pi();
        double dE = fabs(ph1.E()-ph2.E());
        double dR = (photons[a].first - photons[b].first).Mag();

        hPi0Mass->Fill(mgg);
        hOpeningAng->Fill(ang);
        hDE->Fill(dE);
        hDR->Fill(dR);

        hAngVsDE->Fill(ang,dE);
        hDrVsDE->Fill(dR,dE);
        hAngVsMass->Fill(ang,mgg);
        hDrVsMass->Fill(dR,mgg);
      }
    }
  }

  // Write output file only if requested
  if (std::string(outfname).size() > 0) {
    TFile *fout = new TFile(outfname,"RECREATE");
    fout->mkdir("Singles"); fout->cd("Singles");
    hPhE->Write(); hTheta->Write(); hPhi->Write(); hXY->Write();

    fout->mkdir("Pairs"); fout->cd("Pairs");
    hPi0Mass->Write(); hOpeningAng->Write(); hDR->Write(); hDE->Write();
    hAngVsDE->Write(); hDrVsDE->Write(); hAngVsMass->Write(); hDrVsMass->Write();

    fout->Close();
    std::cout << "Histograms written to " << outfname << std::endl;
  }

  // Plots
  TCanvas *c1=new TCanvas("c1",
    Form("Photon kinematics [%s]",restrictLabel(restrictMode).Data()),
    1000,600); c1->Divide(2,2);
  c1->cd(1); hPhE->Draw();
  c1->cd(2); hTheta->Draw();
  c1->cd(3); hPhi->Draw();
  c1->cd(4); hXY->Draw("COLZ");

  TCanvas *c2=new TCanvas("c2",
    Form("Pairs [%s]",restrictLabel(restrictMode).Data()),
    1000,600); c2->Divide(2,2);
  c2->cd(1); hPi0Mass->Draw();
  c2->cd(2); hOpeningAng->Draw();
  c2->cd(3); hDR->Draw();
  c2->cd(4); hDE->Draw();

  TCanvas *c3=new TCanvas("c3",
    Form("Correlations [%s]",restrictLabel(restrictMode).Data()),
    1000,600); c3->Divide(2,2);
  c3->cd(1); hAngVsDE->Draw("COLZ");
  c3->cd(2); hDrVsDE->Draw("COLZ");
  c3->cd(3); hAngVsMass->Draw("COLZ");
  c3->cd(4); hDrVsMass->Draw("COLZ");
}

// --------------------------------------------------------------
// Compare restrictMode outputs for a chosen variable
// opt = "mass", "angle", "dE", "dR"
void compare_restrict_modes(
  const char *opt="mass",
  const char *fname="/Users/rr/jlabHallASoftware/EcalCalibration/analyzer_scripts/10K_pi0_Ecal_cal.root") {

  // ===== Default: only compare Mode 0 vs Mode 2 =====
  int compareModes[2] = {0, 2};

  // ===== Full loop (uncomment if you want all 4) =====
  // int compareModes[4] = {0, 1, 2, 3};

  // Generate output files for selected modes
  for (int i=0; i<sizeof(compareModes)/sizeof(int); i++) {
    int mode = compareModes[i];
    restrictMode = mode;
    TString outname = Form("restrictMode%d.root",mode);
    std::cout << "Running with restrictMode=" << mode
              << " → " << outname << std::endl;
    OP_track_plots(mode, fname, outname.Data());
  }

  // Canvas for overlay
  TCanvas *c = new TCanvas("c_compare","Compare restrictModes",800,600);
  gStyle->SetOptStat(0);

  int colors[4]  = {kBlack, kRed, kBlue, kGreen+2};
  int markers[4] = {20, 21, 22, 23};
  TLegend *leg = new TLegend(0.55,0.65,0.85,0.85);

  // Global text sizes
  gStyle->SetTitleSize(0.05, "XYZ");
  gStyle->SetLabelSize(0.045, "XYZ");

  leg->SetTextSize(0.04);
  leg->SetBorderSize(0);
  leg->SetFillStyle(0);

  // Decide which histo to compare
  TString histoname;
  if      (std::string(opt)=="mass")  histoname = "Pairs/hPi0Mass";
  else if (std::string(opt)=="angle") histoname = "Pairs/hOpenAngle";
  else if (std::string(opt)=="dE")    histoname = "Pairs/hDE";
  else if (std::string(opt)=="dR")    histoname = "Pairs/hDR";
  else {
    std::cerr << "Unknown option: " << opt
              << ". Use mass, angle, dE, or dR." << std::endl;
    return;
  }

  // for invariant mass draw "all photons" vs "true daughters"
if (std::string(opt) == "mass") {
  // Mode 0 (all photons)
  TFile *f0 = TFile::Open("restrictMode0.root","READ");
  TH1D *h0 = (TH1D*) f0->Get(histoname);

  // Mode 2 (true daughters)
  TFile *f2 = TFile::Open("restrictMode2.root","READ");
  TH1D *h2 = (TH1D*) f2->Get(histoname);

  if (h0 && h2) {
    h0->SetMarkerStyle(22);
    h0->SetMarkerColor(kBlue);
    h0->SetMarkerSize(2);
    
    h2->SetLineColor(kRed);
    h2->SetFillColorAlpha(kRed, 0.3);
    
    h2->Draw("HIST");
    h0->Draw("E1 P SAME");
    
    leg->AddEntry(h0, "All photons", "p");
    leg->AddEntry(h2, "True #pi^{0} daughters", "f");
  }
}
else {// Overlay histos with markers
  bool first = true;
  for (int mode=0; mode<=3; mode++) {
    TString fnameOut = Form("restrictMode%d.root",mode);
    TFile *fin = TFile::Open(fnameOut,"READ");
    if (!fin || fin->IsZombie()) continue;

    TH1D *h = (TH1D*) fin->Get(histoname);
    if (!h) continue;

    h->SetMarkerStyle(markers[mode]);
    h->SetMarkerColor(colors[mode]);
    h->SetMarkerSize(1.2);

    if (first) { h->Draw("E1 P"); first=false; }
    else       { h->Draw("E1 P SAME"); }

    leg->AddEntry(h, restrictLabel(mode), "p");
  }
}


  leg->Draw();

  TString outpng = Form("compare_%s_modes.png",opt);
  c->SaveAs(outpng);
  std::cout << "Saved overlay to " << outpng << std::endl;
}
