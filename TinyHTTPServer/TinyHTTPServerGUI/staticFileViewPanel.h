#pragma once

#include "viewPanel.h"

class StaticFileViewPanel : public ViewPanel {
public:
    StaticFileViewPanel(wxWindow* parent);
    int getSupportedMethod() const override;
    ViewPtr getView() const override;
private:
    wxTextCtrl* pathText, * defExtText, * defFileText;

    void OnChooseDir(wxCommandEvent& event);
};

enum {
    ID_CHOOSE_DIR = 1900
};
