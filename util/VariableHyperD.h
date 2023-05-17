#ifndef VARIABLEHYPERD_H
#define VARIABLEHYPERD_H

#include "PlotUtils/VariableHyperDBase.h"
#include "util/Categorized.h"
#include "util/SafeROOTName.h"

class VariableHyperD : public PlotUtils::VariableHyperDBase<CVUniverse> {
   private:
    typedef PlotUtils::HistHyperDWrapper<CVUniverse> Hist;

   public:
    // template <class... ARGS>
    // VariableHyperD(ARGS... args): PlotUtils::VariableHyperDBase<CVUniverse>(args...)
    // {
    // }

    VariableHyperD(std::vector<Variable*>& vars_vec, EAnalysisType type) : PlotUtils::VariableHyperDBase<CVUniverse>(type) {
        for (int i = 0; i < vars.size(); i++)
            AddVariable(*vars[i]);
    }

    VariableHyperD(std::string name, std::vector<Variable*>& vars_vec, EAnalysisType type) : PlotUtils::VariableHyperDBase<CVUniverse>(name, type) {
        for (int i = 0; i < vars.size(); i++)
            AddVariable(*vars[i]);
    }

    // TODO: It's really silly to have to make 2 sets of error bands just because they point to different trees.
    //       I'd rather the physics of the error bands remain the same and just change which tree they point to.
    void InitializeMCHists(std::map<std::string, std::vector<CVUniverse*>>& mc_error_bands,
                           std::map<std::string, std::vector<CVUniverse*>>& truth_error_bands) {
        std::map<int, std::string> BKGLabels = {{0, "NC_Bkg"},
                                                {1, "Bkg_Wrong_Sign"}};
        std::vector<double> lin_x_bins = GetBinVec();
        std::vector<double> y_bins = GetBinVec(1);

        m_backgroundHists = new util::Categorized<Hist, int>((GetName() + "_by_BKG_Label").c_str(),
                                                             GetName().c_str(), BKGLabels,
                                                             lin_x_bins, y_bins, mc_error_bands, m_analysis_type);

        efficiencyNumerator = new Hist((GetName() + "_efficiency_numerator").c_str(), GetName().c_str(), lin_x_bins, y_bins, mc_error_bands, m_analysis_type);
        efficiencyDenominator = new Hist((GetName() + "_efficiency_denominator").c_str(), GetName().c_str(), lin_x_bins, y_bins, truth_error_bands, m_analysis_type);
    }

    // Histograms to be filled
    util::Categorized<Hist, int>* m_backgroundHists;
    Hist* dataHist;
    Hist* efficiencyNumerator;
    Hist* efficiencyDenominator;

    void InitializeDATAHists(std::vector<CVUniverse*>& data_error_bands) {
        const char* name = GetName().c_str();
        dataHist = new Hist(Form("_data_%s", name), name, GetBinVec(), GetBinVec(1), data_error_bands, m_analysis_type);
    }

    void Write(TFile& file) {
        SyncCVHistos();
        file.cd();

        m_backgroundHists->visit([&file](Hist& categ) {
            categ->Write();  // TODO: Or let the TFile destructor do this the "normal" way?
        });

        if (dataHist->hist) {
            dataHist->Write();
        }
        if (efficiencyNumerator) {
            efficiencyNumerator->Write();
        }
        if (efficiencyDenominator) {
            efficiencyDenominator->Write();
        }
    }

    // Only call this manually if you Draw(), Add(), or Divide() plots in this
    // program.
    // Makes sure that all error bands know about the CV.  In the Old Systematics
    // Framework, this was implicitly done by the event loop.
    void SyncCVHistos() {
        m_backgroundHists->visit([](Hist& categ) { categ.SyncCVHistos(); });
        if (dataHist) dataHist->SyncCVHistos();
        if (efficiencyNumerator) efficiencyNumerator->SyncCVHistos();
        if (efficiencyDenominator) efficiencyDenominator->SyncCVHistos();
    }
};

#endif  // VARIABLEHYPERD_H
