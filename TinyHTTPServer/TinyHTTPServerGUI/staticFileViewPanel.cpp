#include "staticFileViewPanel.h"
#include "TinyHTTPServer/staticFileView.h"
#include "TinyHTTPServer/request.h"

StaticFileViewPanel::StaticFileViewPanel(wxWindow* parent) : 
    ViewPanel(parent, "��̬�ļ���ͼ") {
    viewSizer->Add(new wxStaticText(this, wxID_ANY, "����: <filepath>, �ļ����·��"), 0, wxALL, 5);
    viewSizer->Add(new wxStaticText(this, wxID_ANY, "URL����: /<path:filepath>"), 0, wxALL, 5);
    viewSizer->AddSpacer(10);

    viewSizer->Add(new wxStaticText(this, wxID_ANY, "�����ļ���·��:"), 0, wxALL, 5);
    viewSizer->Add(pathText = new wxTextCtrl(this, wxID_ANY, ""), 0, wxLEFT | wxRIGHT | wxEXPAND, 5);
    viewSizer->Add(new wxButton(this, ID_CHOOSE_DIR, "ѡ���ļ���..."), 0, wxALL | wxALIGN_RIGHT, 5);

    Bind(wxEVT_BUTTON, &StaticFileViewPanel::OnChooseDir, this, ID_CHOOSE_DIR);

    url = "/<path:filepath>";
}

ViewPtr StaticFileViewPanel::getView() const {
    return std::make_shared<StaticFileView>(pathText->GetValue().ToStdString());
}

int StaticFileViewPanel::getSupportedMethod() const {
    return Request::GET;
}

void StaticFileViewPanel::OnChooseDir(wxCommandEvent& event) {
    wxDirDialog dlg(NULL, "ѡ�񱾵��ļ���", "",
        wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    if (dlg.ShowModal() == wxID_OK) {
        pathText->SetValue(dlg.GetPath());
    }
}
