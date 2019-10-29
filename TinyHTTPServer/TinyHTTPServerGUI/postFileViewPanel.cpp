#include "postFileViewPanel.h"

#include "TinyHTTPServer/request.h"
#include "TinyHTTPServer/postFileView.h"

PostFileViewPanel::PostFileViewPanel(wxWindow *parent) : ViewPanel(parent, "提交文件视图")
{
    viewSizer->Add(new wxStaticText(this, wxID_ANY, "支持方法: POST, DELETE"), 0, wxALL, 5);
    viewSizer->Add(new wxStaticText(this, wxID_ANY, "参数: <filepath>, 文件相对路径"), 0, wxALL, 5);
    viewSizer->Add(new wxStaticText(this, wxID_ANY, "URL例如: /<path:filepath>"), 0, wxALL, 5);
    viewSizer->AddSpacer(10);

    viewSizer->Add(new wxStaticText(this, wxID_ANY, "本地文件夹路径:"), 0, wxALL, 5);
    viewSizer->Add(pathText = new wxTextCtrl(this, wxID_ANY, ""),
                   0,
                   wxLEFT | wxRIGHT | wxEXPAND,
                   5);
    viewSizer->Add(new wxButton(this, ID_POST_CHOOSE_DIR, "选择文件夹..."),
                   0,
                   wxALL | wxALIGN_RIGHT,
                   5);

    viewSizer->Add(allowDelete = new wxCheckBox(this, wxID_ANY, "允许删除"), 0, wxALL, 5);

    Bind(wxEVT_BUTTON, &PostFileViewPanel::OnChooseDir, this, ID_POST_CHOOSE_DIR);

    url = "/<path:filepath>";
}

ViewPtr PostFileViewPanel::getView() const
{
    return std::make_shared<PostFileView>(pathText->GetValue().ToStdString());
}

int PostFileViewPanel::getSupportedMethod() const
{
    return allowDelete->GetValue() ? Request::POST | Request::DEL : Request::POST;
}

void PostFileViewPanel::OnChooseDir(wxCommandEvent &event)
{
    wxDirDialog dlg(NULL, "选择本地文件夹", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    if (dlg.ShowModal() == wxID_OK) {
        pathText->SetValue(dlg.GetPath());
    }
}
