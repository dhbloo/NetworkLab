#pragma once

#include "wxcommon.h"

#include <memory>

class Router;
class View;
using ViewPtr = std::shared_ptr<View>;

class ViewPanel
    : public wxTreeItemData
    , public wxPanel
{
public:
    ViewPanel(wxWindow *parent, const char *titleText) : wxPanel(parent, wxID_ANY), name(titleText)
    {
        SetSizerAndFit(viewSizer = new wxBoxSizer(wxVERTICAL));
        wxStaticText *title = new wxStaticText(this, wxID_ANY, name);
        wxFont        font  = title->GetFont();
        font.MakeBold().SetPointSize(14);
        title->SetFont(font);
        viewSizer->Add(title, 0, wxALL, 5);
        viewSizer->Add(new wxStaticLine(this), 0, wxEXPAND);
        Hide();
    }
    virtual ViewPtr getView() const = 0;
    virtual int     getSupportedMethod() const { return 0; }
    virtual bool    isRouter() const { return false; }

    const wxString name;
    std::string    url;

protected:
    wxBoxSizer *viewSizer;
};
