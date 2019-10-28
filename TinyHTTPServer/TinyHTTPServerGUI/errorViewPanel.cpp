#include "errorViewPanel.h"

#include "TinyHTTPServer/funcView.h"
#include "TinyHTTPServer/requestExcept.h"

ErrorViewPanel::ErrorViewPanel(wxWindow *parent) : ViewPanel(parent, "��������ͼ")
{
    viewSizer->Add(new wxStaticText(this, wxID_ANY, "��������ʱ,����ͼ������Ӧ"), 0, wxALL, 5);
    viewSizer->Add(new wxStaticText(this, wxID_ANY, "URL��ҪΪ: [errorhandler]"), 0, wxALL, 5);

    wxBoxSizer *lineSizer;
    viewSizer->Add(lineSizer = new wxBoxSizer(wxHORIZONTAL));
    lineSizer->Add(new wxStaticText(this, wxID_ANY, "״̬��(Ϊ0��ʾ������״̬������):"),
                   0,
                   wxALL | wxALIGN_CENTER_VERTICAL,
                   5);
    lineSizer->Add(statusCode = new wxTextCtrl(this, wxID_ANY, "0"), 0, wxLEFT | wxRIGHT, 5);
    viewSizer->AddSpacer(10);

    wxString chioces[] = {"����ҳ��", "�ض���"};
    viewSizer->Add(radio = new wxRadioBox(this,
                                          wxID_ANY,
                                          "����ʽ",
                                          wxDefaultPosition,
                                          wxDefaultSize,
                                          2,
                                          chioces),
                   0,
                   wxALL,
                   5);

    viewSizer->Add(new wxStaticText(this, wxID_ANY, "�ض���URL:"), 0, wxALL, 5);
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