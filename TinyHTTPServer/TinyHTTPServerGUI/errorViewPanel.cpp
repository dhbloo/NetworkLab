#include "errorViewPanel.h"

#include "TinyHTTPServer/funcView.h"
#include "TinyHTTPServer/requestExcept.h"

ErrorViewPanel::ErrorViewPanel(wxWindow *parent) : ViewPanel(parent, "错误处理视图")
{
    viewSizer->Add(new wxStaticText(this, wxID_ANY, "发生错误时,该视图处理响应"), 0, wxALL, 5);
    viewSizer->Add(new wxStaticText(this, wxID_ANY, "URL需要为: [errorhandler]"), 0, wxALL, 5);

    wxBoxSizer *lineSizer;
    viewSizer->Add(lineSizer = new wxBoxSizer(wxHORIZONTAL));
    lineSizer->Add(new wxStaticText(this, wxID_ANY, "状态码(为0表示对所有状态码适用):"),
                   0,
                   wxALL | wxALIGN_CENTER_VERTICAL,
                   5);
    lineSizer->Add(statusCode = new wxTextCtrl(this, wxID_ANY, "0"), 0, wxLEFT | wxRIGHT, 5);
    viewSizer->AddSpacer(10);

    wxString chioces[] = {"错误页面", "重定向"};
    viewSizer->Add(radio = new wxRadioBox(this,
                                          wxID_ANY,
                                          "处理方式",
                                          wxDefaultPosition,
                                          wxDefaultSize,
                                          2,
                                          chioces),
                   0,
                   wxALL,
                   5);

    viewSizer->Add(new wxStaticText(this, wxID_ANY, "重定向URL:"), 0, wxALL, 5);
    viewSizer->Add(redirect = new wxTextCtrl(this, wxID_ANY, ""),
                   0,
                   wxLEFT | wxRIGHT | wxEXPAND,
                   5);

    url = "[errorhandler]";
}

ViewPtr ErrorViewPanel::getView() const
{
    switch (radio->GetSelection()) {
    case 0:
        return MakeFuncView([](auto &req, auto &res) {
            res.body = R"(<h1 style="text-align:center;">)" + std::to_string(res.statusCode) + " "
                       + res.statusInfo() + R"(!</h1><HR><p style="text-align:center;">)"
                       + res.headers["Server"] + "</p>";
            res.headers["Content-Type"] = "text/html;";
        });

    case 1:
        return MakeFuncView(
            [=](auto &req, auto &res) { throw Redirect(redirect->GetValue().ToStdString(), ""); });
    }
}