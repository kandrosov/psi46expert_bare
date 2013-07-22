/*!
 * \file RootPrintToPdf.h
 * \brief Print ROOT histograms to PDF.
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 */

#pragma once

#include <sstream>
#include <stdexcept>

#include <TROOT.h>
#include <TStyle.h>
#include <TFile.h>
#include <Rtypes.h>

#include <TH1.h>
#include <TH2.h>
#include <TProfile.h>

#include "RootPrintTools.h"

namespace root_ext {
struct PageSideLayout {
    Box main_pad;
    bool has_stat_pad;
    Box stat_pad;
    bool has_legend;
    Box legend_pad;
};

struct PageSide {
    std::string histogram_name;
    std::string histogram_title;
    std::string axis_titleX;
    std::string axis_titleY;
    std::string draw_options;
    bool use_log_scaleX;
    bool use_log_scaleY;
    bool fit_range;

    PageSideLayout layout;
};

struct PageLayout {
    bool has_title;
    Box title_box;
    Font_t title_font;
    std::string global_style;
    Int_t stat_options;
    Int_t fit_options;
};

struct Page {
    typedef std::vector<const PageSide*> RegionCollection;
    std::string title;
    PageLayout layout;

    virtual RegionCollection Regions() const = 0;
    virtual ~Page() {}
};

struct SingleSidedPage : public Page {
    PageSide side;

    explicit SingleSidedPage(bool has_title = true, bool has_stat_pad = true, bool has_legend = true)
    {
        layout.has_title = has_title;
        layout.title_box = Box(0.1, 0.94, 0.9, 0.98);
        layout.title_font = 52;
        layout.global_style = "Plain";
        if(has_stat_pad) {
            layout.stat_options = 1111;
            layout.fit_options = 111;
        } else {
            layout.stat_options = 0;
            layout.fit_options = 0;
        }

        side.layout.has_stat_pad = has_stat_pad;
        side.layout.has_legend = has_legend;
        side.use_log_scaleX = false;
        side.use_log_scaleY = false;
        side.fit_range = true;

        side.layout.main_pad = Box(0.01, 0.01, 0.85, 0.91);
        side.layout.stat_pad = Box(0.86, 0.01, 0.99, 0.91);
        side.layout.legend_pad = Box(0.5, 0.67, 0.88, 0.88);
    }

    virtual RegionCollection Regions() const
    {
        RegionCollection regions;
        regions.push_back(&side);
        return regions;
    }
};

struct DoubleSidedPage : public Page {
    PageSide left_side, right_side;

    explicit DoubleSidedPage(bool has_title = true, bool has_stat_pad = true, bool has_legend = true)
    {
        layout.has_title = has_title;
        layout.title_box = Box(0.1, 0.94, 0.9, 0.98);
        layout.title_font = 52;
        layout.global_style = "Plain";
        if(has_stat_pad) {
            layout.stat_options = 1111;
            layout.fit_options = 111;
        } else {
            layout.stat_options = 0;
            layout.fit_options = 0;
        }

        left_side.layout.has_stat_pad = has_stat_pad;
        left_side.layout.has_legend = has_legend;
        left_side.use_log_scaleX = false;
        left_side.use_log_scaleY = false;
        left_side.fit_range = true;
        left_side.layout.main_pad = Box(0.01, 0.01, 0.35, 0.91);
        left_side.layout.stat_pad = Box(0.36, 0.01, 0.49, 0.91);
        left_side.layout.legend_pad = Box(0.5, 0.67, 0.88, 0.88);

        right_side.layout.has_stat_pad = has_stat_pad;
        right_side.layout.has_legend = has_legend;
        right_side.use_log_scaleX = false;
        right_side.use_log_scaleY = false;
        right_side.fit_range = true;
        right_side.layout.main_pad = Box(0.51, 0.01, 0.85, 0.91);
        right_side.layout.stat_pad = Box(0.86, 0.01, 0.99, 0.91);
        right_side.layout.legend_pad = Box(0.5, 0.67, 0.88, 0.88);
    }

    virtual RegionCollection Regions() const
    {
        RegionCollection regions;
        regions.push_back(&left_side);
        regions.push_back(&right_side);
        return regions;
    }
};

template<typename HistogramType, typename _ValueType=Double_t, typename OriginalHistogramType=HistogramType>
class HistogramSource {
public:
    typedef HistogramType Histogram;
    typedef OriginalHistogramType OriginalHistogram;
    typedef _ValueType ValueType;
    typedef typename root_ext::HistogramPlotter<Histogram, ValueType>::Options PlotOptions;
    typedef typename root_ext::HistogramPlotter<Histogram, ValueType>::Entry Entry;

    static PlotOptions& GetDefaultPlotOptions(unsigned n)
    {
        static std::vector<PlotOptions> options;
        if(!options.size())
        {
            options.push_back( PlotOptions(kBlack, 1, root_ext::Box(0.01, 0.71, 0.99, 0.9), 0.1, kBlack, 2) );
            options.push_back( PlotOptions(kRed, 1, root_ext::Box(0.01, 0.51, 0.99, 0.7), 0.1, kRed, 2) );
            options.push_back( PlotOptions(kGreen, 1, root_ext::Box(0.01, 0.31, 0.99, 0.5), 0.1, kGreen, 2) );
            options.push_back( PlotOptions(kBlue, 1, root_ext::Box(0.01, 0.11, 0.99, 0.3), 0.1, kBlue, 2) );
        }
        return n < options.size() ? options[n] : options[options.size() - 1];
    }

public:
    void Add(const std::string& display_name, TFile* source_file, const PlotOptions* plot_options = 0)
    {
        if(!plot_options)
            plot_options = &GetDefaultPlotOptions(display_names.size());

        display_names.push_back(display_name);
        source_files.push_back(source_file);
        plot_options_vector.push_back(*plot_options);
    }

    size_t Size() const { return display_names.size(); }

    Entry Get(unsigned id, const std::string& name) const
    {
        if(!source_files[id])
            return Entry(0, plot_options_vector[id]);
        std::string realName = GenerateName(id, name);
        OriginalHistogram* original_histogram
                = static_cast<OriginalHistogram*>(source_files[id]->Get(realName.c_str()));
        if(!original_histogram)
            throw std::runtime_error("original histogram not found.");
        Histogram* histogram = Convert(original_histogram);
        if(!histogram)
            throw std::runtime_error("source histogram not found.");
        Prepare(histogram, display_names[id], plot_options_vector[id]);
        return Entry(histogram, plot_options_vector[id]);
    }

    virtual ~HistogramSource() {}

protected:
    virtual Histogram* Convert(OriginalHistogram* original_histogram) const = 0;

    virtual std::string GenerateName(unsigned /*id*/, const std::string& name) const
    {
        return name;
    }

    virtual void Prepare(Histogram* histogram, const std::string& display_name,
                         const PlotOptions& plot_options) const
    {
        histogram->SetName(display_name.c_str());
        histogram->SetLineColor(plot_options.color);
        histogram->SetLineWidth(plot_options.line_width);
    }

private:
    std::vector<TFile*> source_files;
    std::vector<std::string> display_names;
    std::vector<PlotOptions> plot_options_vector;
};

template<typename Histogram, typename ValueType=Double_t>
class SimpleHistogramSource : public HistogramSource<Histogram, ValueType, Histogram> {
protected:
    virtual Histogram* Convert(Histogram* original_histogram) const
    {
        return static_cast<Histogram*>(original_histogram->Clone());
    }
};


class PdfPrinter {
public:
    PdfPrinter(const std::string& _output_file_name)
        : output_file_name(_output_file_name)
    {
        canvas = new TCanvas();
        canvas->Print((output_file_name + "[").c_str());
    }

    template<typename Source>
    void Print(const Page& page, const Source& source)
    {
        gROOT->SetStyle(page.layout.global_style.c_str());
        gStyle->SetOptStat(page.layout.stat_options);
        gStyle->SetOptFit(page.layout.fit_options);
        canvas->cd();

        canvas->SetTitle(page.title.c_str());
        if(page.layout.has_title) {
            TPaveLabel *title = Adapter::NewPaveLabel(page.layout.title_box, page.title);
            title->SetTextFont(page.layout.title_font);
            title->Draw();
        }

        Page::RegionCollection page_regions = page.Regions();
        for(Page::RegionCollection::const_iterator iter = page_regions.begin(); iter != page_regions.end(); ++iter)
        {
            canvas->cd();
            DrawHistograms(*(*iter), source);
        }

        canvas->Draw();
        canvas->Print(output_file_name.c_str());
    }

    ~PdfPrinter()
    {
        canvas->Print((output_file_name+"]").c_str());
    }

private:
    template<typename Source>
    void DrawHistograms(const PageSide& page_side, const Source& source)
    {
        typedef root_ext::HistogramPlotter<typename Source::Histogram, typename Source::ValueType> Plotter;

        TPad* stat_pad = 0;
        if(page_side.layout.has_stat_pad) {
            stat_pad = Adapter::NewPad(page_side.layout.stat_pad);
            stat_pad->Draw();
        }

        TPad *pad = Adapter::NewPad(page_side.layout.main_pad);
        if(page_side.use_log_scaleX)
            pad->SetLogx();
        if(page_side.use_log_scaleY)
            pad->SetLogy();
        pad->Draw();
        pad->cd();

        Plotter plotter(page_side.histogram_title, page_side.axis_titleX, page_side.axis_titleY);
        for(unsigned n = 0; n < source.Size(); ++n)
        {
            const typename Plotter::Entry entry = source.Get(n, page_side.histogram_name);
            plotter.Add(entry);
        }
        plotter.Superpose(pad, stat_pad, page_side.fit_range, page_side.layout.has_legend, page_side.layout.legend_pad,
                          page_side.draw_options);
    }

private:
    TCanvas* canvas;
    std::string output_file_name;
};

} // root_ext
