#include <TFile.h>
#include <TChain.h>
#include <TCanvas.h>
#include <TH1D.h>
#include <TStyle.h>
#include <TPad.h>

void plotter() {

    // ── style ────────────────────────────────────────────────────────────────
    gStyle->SetOptStat("emr");
    gStyle->SetOptTitle(0);
    gStyle->SetFrameLineWidth(2);
    gStyle->SetHistLineWidth(2);
    gStyle->SetHistLineColor(kAzure+1);
    gStyle->SetPadTickX(1);
    gStyle->SetPadTickY(1);

    // ── chain ────────────────────────────────────────────────────────────────
    const char* treeName = "Events";   // <-- adjust if different
    TChain* chain = new TChain(treeName);
    TString fileListStr = gSystem->GetFromPipe(
        "ls data/reco/run_*/spill_*/out/outfile.root 2>/dev/null");
 
    if (fileListStr.IsNull()) {
        Printf("ERROR: no files found — check the path.");
        return;
    }
 
    TObjArray* tokens = fileListStr.Tokenize("\n");
    Printf("Files found:");
    for (int i = 0; i < tokens->GetEntries(); ++i) {
        const char* fname = ((TObjString*)tokens->At(i))->GetString().Data();
        Printf("  [%d] %s", i, fname);
        chain->Add(fname);
    }
    delete tokens;
 
    Long64_t nEntries = chain->GetEntries();
    int nFiles = (int)chain->GetListOfFiles()->GetSize();
    Printf("\nLoaded %d files, %lld entries total", nFiles, nEntries);
 
    if (nEntries == 0) {
        Printf("WARNING: 0 entries — tree name \"%s\" may be wrong.", treeName);
        Printf("Keys in the first file:");
        TFile* probe = TFile::Open(chain->GetListOfFiles()->First()->GetTitle(), "READ");
        if (probe && !probe->IsZombie()) { probe->ls(); probe->Close(); }
        return;
    }

    // ── merge ────────────────────────────────────────────────────────────────
    const char* outFile = "dimuon_merged.root";
    TFile* out = TFile::Open(outFile, "RECREATE");
    TTree* merged = chain->CloneTree(-1, "fast");
    merged->Write();
    out->Close();
    Printf("Merged tree written to %s", outFile);

    // ── histograms ───────────────────────────────────────────────────────────
    TH1D* h_zvtx = new TH1D("h_zvtx",
        ";z_{vtx}  [cm];Counts",
        200, -500., 500.);

    TH1D* h_mass = new TH1D("h_mass",
        ";M_{#mu#mu}  [GeV/c^{2}];Counts",
        200, 0., 10.);

    chain->Draw("dimuon_z_vtx>>h_zvtx", "", "goff");
    chain->Draw("dimuon_mass>>h_mass",  "", "goff");

    // helper lambda: save one histogram to PNG
    auto savePlot = [&](TH1D* h, const char* fname) {
        TCanvas* c = new TCanvas(fname, fname, 800, 600);
        c->SetLeftMargin(0.13);
        c->SetBottomMargin(0.13);
        h->GetYaxis()->SetTitleOffset(1.5);
        h->DrawCopy("HIST");
        c->SaveAs(fname);
        Printf("Saved %s", fname);
        delete c;
    };

    savePlot(h_zvtx, "dimuon_z_vtx.png");
    savePlot(h_mass, "dimuon_mass.png");

    delete h_zvtx;
    delete h_mass;
    delete chain;
}