#pragma once

#include "viewPanel.h"
#include <map>

class Router;

class RouterViewPanel : public ViewPanel {
public:
    RouterViewPanel(wxWindow* parent, wxTreeCtrl* tree, wxTreeItemId id);
    ViewPtr getView() const override;
    int getSupportedMethod() const override;
    bool isRouter() const { return true; }

    std::unique_ptr<Router> makeRouter() const;
private:
    wxTreeCtrl* tree;
    wxTreeItemId id;
};
