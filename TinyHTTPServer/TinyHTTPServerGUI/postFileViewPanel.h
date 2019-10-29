#pragma once

#include "viewPanel.h"

class PostFileViewPanel : public ViewPanel
{
public:
    PostFileViewPanel(wxWindow *parent);
    int     getSupportedMethod() const override;
    ViewPtr getView() const override;

private:
    wxTextCtrl *pathText;
    wxCheckBox *allowDelete;

    void OnChooseDir(wxCommandEvent &event);
};

enum { ID_POST_CHOOSE_DIR = 1902 };
