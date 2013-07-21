/*!
 * \file RootPrintTools.h
 * \brief Contains useful functions to print histograms with ROOT.
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 */

#pragma once

#include <limits>
#include <vector>

#include <TPaveStats.h>
#include <TPaveLabel.h>
#include <TPad.h>
#include <TCanvas.h>
#include <TLegend.h>

namespace root_ext {
template<typename Histogram, typename ValueType=Double_t, typename Container=std::vector<Histogram*> >
class HistogramFitter {
public:
    static ValueType FindMinLimit(const Histogram* h)
    {
        if(!h)
            return std::numeric_limits<ValueType>::max();
        for(Int_t i = 0; i < h->GetNbinsX(); ++i) {
            if(h->GetBinContent(i))
                return h->GetBinLowEdge(i);
        }
        return h->GetMinimum();
    }

    static ValueType FindMaxLimit(const Histogram* h)
    {
        if(!h)
            return std::numeric_limits<ValueType>::min();
        for(Int_t i = h->GetNbinsX() - 1; i > 0; --i) {
            if(h->GetBinContent(i))
                return h->GetBinLowEdge(i) + h->GetBinWidth(i);
        }
        return h->GetMaximum();
    }

    static void FitRange(const Container& hists)
    {
        if(!hists.size())
            return;
        ValueType min = FindMinLimit(hists[0]);
        ValueType max = FindMaxLimit(hists[0]);
        for(unsigned n = 1; n < hists.size(); ++n) {
            min = std::min(min, FindMinLimit(hists[n]));
            max = std::max(max, FindMaxLimit(hists[n]));
        }
        for(unsigned n = 0; n < hists.size(); ++n) {
            if(hists[n])
                hists[n]->SetAxisRange(min, max, "X");
        }
    }

private:
    HistogramFitter() {}
};

struct Point {
    Double_t x, y;
    Point() : x(0), y(0) {}
    Point(Double_t _x, Double_t _y) : x(_x), y(_y) {}
};

struct Box {
    Point left_bottom, right_top;
    Box() {}
    Box(const Point& _left_bottom, const Point& _right_top) : left_bottom(_left_bottom), right_top(_right_top) {}
    Box(Double_t left_bottom_x, Double_t left_bottom_y, Double_t right_top_x, Double_t right_top_y)
        : left_bottom(left_bottom_x, left_bottom_y), right_top(right_top_x, right_top_y) {}
};

class Adapter {
public:
    static TPaveLabel* NewPaveLabel(const Box& box, const std::string& text)
    {
        return new TPaveLabel(box.left_bottom.x, box.left_bottom.y, box.right_top.x, box.right_top.y, text.c_str());
    }

    static TPad* NewPad(const Box& box)
    {
        static const char* pad_name = "pad";
        return new TPad(pad_name, pad_name, box.left_bottom.x, box.left_bottom.y, box.right_top.x, box.right_top.y);
    }

private:
    Adapter() {}
};

template<typename Histogram, typename ValueType=Double_t>
class HistogramPlotter {
public:
    struct Options {
        Color_t color;
        Width_t line_width;
        Box pave_stats_box;
        Double_t pave_stats_text_size;
        Color_t fit_color;
        Width_t fit_line_width;
        Options() : color(kBlack), line_width(1), pave_stats_text_size(0), fit_color(kBlack), fit_line_width(1) {}
        Options(Color_t _color, Width_t _line_width, const Box& _pave_stats_box, Double_t _pave_stats_text_size,
            Color_t _fit_color, Width_t _fit_line_width)
            : color(_color), line_width(_line_width), pave_stats_box(_pave_stats_box),
              pave_stats_text_size(_pave_stats_text_size), fit_color(_fit_color), fit_line_width(_fit_line_width) {}
    };

    struct Entry {
        Histogram* histogram;
        Options plot_options;
        Entry(Histogram* _histogram, const Options& _plot_options)
            : histogram(_histogram), plot_options(_plot_options) {}
    };

    typedef std::vector<Histogram*> HistogramContainer;

public:
    HistogramPlotter(const std::string& _title, const std::string& _axis_titleX, const std::string& _axis_titleY)
        : title(_title), axis_titleX(_axis_titleX),axis_titleY(_axis_titleY) {}

    void Add(Histogram* histogram, const Options& plot_options)
    {
        histograms.push_back(histogram);
        options.push_back(plot_options);
    }

    void Add(const Entry& entry)
    {
        histograms.push_back(entry.histogram);
        options.push_back(entry.plot_options);
    }

    void Superpose(TPad* main_pad, TPad* stat_pad, bool fit_range, bool draw_legend, const Box& legend_box,
                   const std::string& draw_options)
    {
        if(!histograms.size() || !main_pad)
            return;

        if(fit_range)
            HistogramFitter<Histogram, ValueType>::FitRange(histograms);

        histograms[0]->SetTitle(title.c_str());
        histograms[0]->GetXaxis()->SetTitle(axis_titleX.c_str());
        histograms[0]->GetYaxis()->SetTitle(axis_titleY.c_str());

        TLegend* legend = 0;
        if(draw_legend) {
            legend = new TLegend(legend_box.left_bottom.x, legend_box.left_bottom.y,
                                 legend_box.right_top.x, legend_box.right_top.y);
        }

        for(unsigned n = 0; n < histograms.size(); ++n) {
            main_pad->cd();
            const Options& o = options[n];
            Histogram* h = histograms[n];
            if(!h)
                continue;

            const char* opt = n ? "sames" : draw_options.c_str();
            h->Draw(opt);
            if(legend) {
                legend->AddEntry(h, h->GetName());
                legend->Draw();
            }

            main_pad->Update();
            if(!stat_pad)
                continue;
            stat_pad->cd();
            TPaveStats *pave_stats = (TPaveStats*)h->GetListOfFunctions()->FindObject("stats");

            TPaveStats *pave_stats_copy = (TPaveStats *) pave_stats->Clone();
            h->SetStats(0);

            pave_stats_copy->SetX1NDC(o.pave_stats_box.left_bottom.x);
            pave_stats_copy->SetX2NDC(o.pave_stats_box.right_top.x);
            pave_stats_copy->SetY1NDC(o.pave_stats_box.left_bottom.y);
            pave_stats_copy->SetY2NDC(o.pave_stats_box.right_top.y);
            pave_stats_copy->ResetAttText();
            pave_stats_copy->SetTextColor(o.color);
            pave_stats_copy->SetTextSize(o.pave_stats_text_size);
            pave_stats_copy->Draw();
            stat_pad->Update();
        }
    }

private:
    HistogramContainer histograms;
    std::vector<Options> options;
    std::string title;
    std::string axis_titleX, axis_titleY;
};

} // root_ext
