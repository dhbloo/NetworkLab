#include "staticFileViewPanel.h"

#include "TinyHTTPServer/request.h"
#include "TinyHTTPServer/staticFileView.h"

StaticFileViewPanel::StaticFileViewPanel(wxWindow *parent) : ViewPanel(parent, "静态文件视图")
{
    viewSizer->Add(new wxStaticText(this, wxID_ANY, "参数: <filepath>, 文件相对路径"), 0, wxALL, 5);
    viewSizer->Add(new wxStaticText(this, wxID_ANY, "URL例如: /<path:filepath>"), 0, wxALL, 5);
    viewSizer->AddSpacer(10);

    viewSizer->Add(new wxStaticText(this, wxID_ANY, "本地文件夹路径:"), 0, wxALL, 5);
    viewSizer->Add(pathText = new wxTextCtrl(this, wxID_ANY, ""),
                   0,
                   wxLEFT | wxRIGHT | wxEXPAND,
                   5);
    viewSizer->Add(new wxButton(this, ID_CHOOSE_DIR, "选择文件夹..."), 0, wxALL | wxALIGN_RIGHT, 5);

    wxBoxSizer *lineSizer;
    viewSizer->Add(lineSizer = new wxBoxSizer(wxHORIZONTAL));
    lineSizer->Add(new wxStaticText(this, wxID_ANY, "访问目录时的默认文件名:"),
                   0,
                   wxALL | wxALIGN_CENTER_VERTICAL,
                   5);
    lineSizer->Add(defFileText = new wxTextCtrl(this, wxID_ANY, "index.html"), 0, wxALL, 5);

    viewSizer->Add(lineSizer = new wxBoxSizer(wxHORIZONTAL));
    lineSizer->Add(new wxStaticText(this, wxID_ANY, "默认扩展名:"),
                   0,
                   wxALL | wxALIGN_CENTER_VERTICAL,
                   5);
    lineSizer->Add(defExtText = new wxTextCtrl(this, wxID_ANY, ".html"), 0, wxALL, 5);

    Bind(wxEVT_BUTTON, &StaticFileViewPanel::OnChooseDir, this, ID_CHOOSE_DIR);

    url = "/<path:filepath>";
}

ViewPtr StaticFileViewPanel::getView() const
{
    return std::make_shared<StaticFileView>(pathText->GetValue().ToStdString(),
                                            defExtText->GetValue().ToStdString(),
                                            defFileText->GetValue().ToStdString());
}

int StaticFileViewPanel::getSupportedMethod() const
{
    return Request::GET;
}

void StaticFileViewPanel::OnChooseDir(wxCommandEvent &event)
{
    wxDirDialog dlg(NULL, "选择本地文件夹", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    if (dlg.ShowModal() == wxID_OK) {
        pathText->SetValue(dlg.GetPath());
    }
}
