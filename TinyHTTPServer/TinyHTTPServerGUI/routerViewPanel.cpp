#include "routerViewPanel.h"

#include "TinyHTTPServer/request.h"
#include "TinyHTTPServer/router.h"
#include "TinyHTTPServer/routerView.h"
#include "errorViewPanel.h"

RouterViewPanel::RouterViewPanel(wxWindow *parent, wxTreeCtrl *tree, wxTreeItemId id)
    : ViewPanel(parent, "路由视图")
    , tree(tree)
    , id(id)
{
    viewSizer->Add(new wxStaticText(this, wxID_ANY, "参数: <router>, 子视图URL"), 0, wxALL, 5);
    viewSizer->Add(new wxStaticText(this, wxID_ANY, "URL例如: /<path:router>"), 0, wxALL, 5);
}

ViewPtr RouterViewPanel::getView() const
{
    return std::make_shared<RouterView>(*makeRouter());
}

int RouterViewPanel::getSupportedMethod() const
{
    return Request::GET;
}

std::unique_ptr<Router> RouterViewPanel::makeRouter() const
{
    auto router = std::make_unique<Router>();

    wxTreeItemIdValue cookie;
    for (wxTreeItemId p = tree->GetFirstChild(id, cookie); p.IsOk();
         p              = tree->GetNextChild(id, cookie)) {
        ViewPanel *vp = (ViewPanel *)tree->GetItemData(p);
        if (vp->url.substr(0, 14) == "[errorhandler]") {
            ErrorViewPanel *evp = (ErrorViewPanel *)vp;
            unsigned long   statucCode;
            if (evp->statusCode->GetValue().ToULong(&statucCode))
                router->setErrorHandler(statucCode, evp->getView());
        }
        else {
            router->setRoute(vp->url, vp->getSupportedMethod(), vp->getView());
        }
    }

    return std::move(router);
}