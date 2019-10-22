#pragma once

#include "viewPanel.h"

class ErrorViewPanel : public ViewPanel {
public:
    ErrorViewPanel(wxWindow* parent);
    ViewPtr getView() const override;
    wxTextCtrl* statusCode;
private:
    wxRadioBox* radio;
    wxTextCtrl* redirect;
};