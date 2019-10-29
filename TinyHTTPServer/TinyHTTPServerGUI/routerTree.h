#pragma once

#include "viewPanel.h"
#include "wxcommon.h"

#include <memory>

class Router;

class RouterTree : public wxPanel
{
public:
    RouterTree(wxWindow *parent);

    std::unique_ptr<Router> makeRouter() const;
    void                    enable(bool enable);

    template<typename Pred>
    wxTreeItemId addRoute(wxTreeItemId parent, Pred create, bool error = false);

private:
    wxBoxSizer *            sizer, *hsizer;
    wxTreeCtrl *            tree;
    wxStaticText *          url;
    wxButton *              urlbtn;
    wxPanel *               view;
    wxTreeItemId            selected;
    std::unique_ptr<wxMenu> itemRCMenu;

    void OnItemMenu(wxTreeEvent &event);
    void OnItemSelected(wxCommandEvent &event);
    void OnURLButton(wxCommandEvent &event);

    bool askURL(std::string &url);
};

enum {
    ID_RT_TREE = 1800,
    ID_URL_BTN,
    ID_STATICFILE,
    ID_POSTFILE,
    ID_ROUTER,
    ID_ERR_VIEW,
    ID_REDIRECT
};

template<typename Pred>
inline wxTreeItemId RouterTree::addRoute(wxTreeItemId parent, Pred create, bool error)
{
    ViewPanel * vpNew = create(wxTreeItemId());
    std::string url   = vpNew ? vpNew->url : "";
    if (!error && !askURL(url)) {
        if (vpNew)
            delete vpNew;
        return wxTreeItemId();
    }

    wxTreeItemId newItem = error ? tree->AppendItem(parent, "") : tree->PrependItem(parent, "");
    if (!vpNew)
        vpNew = create(newItem);
    tree->SetItemText(newItem, vpNew->name + " | " + (vpNew->url = url));
    tree->SetItemData(newItem, vpNew);
    tree->Expand(parent);
    return vpNew;
}
