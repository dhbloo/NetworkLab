#include "routerTree.h"

#include "TinyHTTPServer/router.h"
#include "errorViewPanel.h"
#include "routerViewPanel.h"
#include "staticFileViewPanel.h"

RouterTree::RouterTree(wxWindow *parent) : wxPanel(parent, wxID_ANY)
{
    // wxBoxSizer* hsizer;
    SetSizerAndFit(hsizer = new wxBoxSizer(wxHORIZONTAL));
    hsizer->Add(tree = new wxTreeCtrl(this, ID_RT_TREE), 1, wxEXPAND, 0);
    hsizer->Add(sizer = new wxBoxSizer(wxVERTICAL), 2, wxLEFT | wxEXPAND, 5);

    sizer->Add(hsizer = new wxBoxSizer(wxHORIZONTAL), 0, wxEXPAND | wxDOWN, 5);
    sizer->Add(view = new wxPanel(this,
                                  wxID_ANY,
                                  wxDefaultPosition,
                                  wxDefaultSize,
                                  wxTAB_TRAVERSAL | wxBORDER_SIMPLE),
               1,
               wxEXPAND);

    hsizer->Add(new wxStaticText(this, wxID_ANY, "局部URL: "),
                0,
                wxALIGN_CENTER_VERTICAL | wxDOWN | wxUP,
                5);
    hsizer->Add(url = new wxStaticText(this, wxID_ANY, "/"), 1, wxALIGN_CENTER_VERTICAL);
    hsizer->Add(urlbtn = new wxButton(this, ID_URL_BTN, "修改"));

    view->SetSizerAndFit(sizer = new wxBoxSizer(wxVERTICAL));

    Bind(wxEVT_TREE_ITEM_MENU, &RouterTree::OnItemMenu, this, ID_RT_TREE);
    Bind(wxEVT_TREE_SEL_CHANGED, &RouterTree::OnItemSelected, this, ID_RT_TREE);
    Bind(wxEVT_BUTTON, &RouterTree::OnURLButton, this, ID_URL_BTN);

    selected = tree->AddRoot("根视图");
    tree->SetItemData(selected, new RouterViewPanel(view, tree, selected));
    ViewPanel *rootvp = (ViewPanel *)tree->GetItemData(selected);
    sizer->Add(rootvp, 1, wxEXPAND);
    tree->SelectItem(selected);
}

void RouterTree::OnItemMenu(wxTreeEvent &event)
{
    wxTreeItemId focused = event.GetItem();
    ViewPanel *  vp      = (ViewPanel *)tree->GetItemData(focused);
    itemRCMenu           = std::make_unique<wxMenu>();
    if (vp->isRouter()) {
        RouterViewPanel *rvp = (RouterViewPanel *)vp;

        wxMenu *itemAddMenu = new wxMenu;

        itemAddMenu->Append(ID_STATICFILE, "静态文件视图");
        Bind(
            wxEVT_MENU,
            [=](auto &) {
                addRoute(focused, [=](auto id) { return new StaticFileViewPanel(view); });
            },
            ID_STATICFILE);

        itemAddMenu->Append(ID_ROUTER, "子路由视图");
        Bind(
            wxEVT_MENU,
            [=](auto &) {
                addRoute(focused, [=](auto id) {
                    return id.IsOk() ? new RouterViewPanel(view, tree, id) : nullptr;
                });
            },
            ID_ROUTER);

        itemAddMenu->AppendSeparator();

        itemAddMenu->Append(ID_ERR_VIEW, "错误视图");
        Bind(
            wxEVT_MENU,
            [=](auto &) {
                addRoute(
                    focused,
                    [=](auto id) { return new ErrorViewPanel(view); },
                    true);
            },
            ID_ERR_VIEW);

        itemRCMenu->AppendSubMenu(itemAddMenu, "增加子视图");
        itemRCMenu->AppendSeparator();
    }

    itemRCMenu->Append(wxID_DELETE, "删除视图")->Enable(focused != tree->GetRootItem());
    Bind(
        wxEVT_MENU,
        [=](wxCommandEvent &) {
            tree->SelectItem(tree->GetItemParent(focused));
            RouterViewPanel *rvp = (RouterViewPanel *)tree->GetItemData(selected);
            tree->Delete(focused);
        },
        wxID_DELETE);

    PopupMenu(itemRCMenu.get());
}

void RouterTree::OnItemSelected(wxCommandEvent &event)
{
    ViewPanel *vpOld = (ViewPanel *)tree->GetItemData(selected);
    vpOld->Hide();

    selected         = tree->GetSelection();
    ViewPanel *vpNew = (ViewPanel *)tree->GetItemData(selected);
    vpNew->Show();

    sizer->Replace(vpOld, vpNew);
    sizer->Layout();

    if (selected == tree->GetRootItem()) {
        hsizer->Hide(2ULL);
        url->SetLabelText("(根路由)");
    }
    else {
        hsizer->Show(2ULL);
        url->SetLabelText(vpNew->url);
    }
    hsizer->Layout();
}

void RouterTree::OnURLButton(wxCommandEvent &event)
{
    ViewPanel *vp = (ViewPanel *)tree->GetItemData(selected);
    askURL(vp->url);
    tree->SetItemText(selected, vp->name + " | " + vp->url);
    url->SetLabelText(vp->url);
}

void RouterTree::enable(bool enable)
{
    view->Enable(enable);
    urlbtn->Enable(enable);
    if (enable)
        Bind(wxEVT_TREE_ITEM_MENU, &RouterTree::OnItemMenu, this, ID_RT_TREE);
    else
        Unbind(wxEVT_TREE_ITEM_MENU, &RouterTree::OnItemMenu, this, ID_RT_TREE);
}

bool RouterTree::askURL(std::string &url)
{
    wxTextEntryDialog dlg(this, "输入视图URL", "URL", url);
    if (dlg.ShowModal() == wxID_OK) {
        if (dlg.GetValue().IsEmpty()) {
            wxMessageBox("URL不能为空!", "错误", wxICON_ERROR);
            return false;
        }
        url = dlg.GetValue();
        return true;
    }
    return false;
}

std::unique_ptr<Router> RouterTree::makeRouter() const
{
    RouterViewPanel *rootvp = (RouterViewPanel *)tree->GetItemData(tree->GetRootItem());
    return rootvp->makeRouter();
}
