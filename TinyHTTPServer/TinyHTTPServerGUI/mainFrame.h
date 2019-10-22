#pragma once

#include "wxcommon.h"
#include <memory>

class RouterTree;

class MainFrame : public wxFrame {
public:
    MainFrame();
private:
    wxTextCtrl* ipText, * portText, *logText;
    wxButton* startBtn;
    wxListView* connList;
    std::unique_ptr<RouterTree> routerTree;
    std::unique_ptr<wxStreamToTextRedirector> redirect;

    void OnStart(wxCommandEvent& event);
    void OnClear(wxCommandEvent& event);
    void OnRefreshList(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);

    wxDECLARE_EVENT_TABLE();
};

enum {
    ID_START_BTN = 101,
    ID_CLEAR_BTN,

    ID_LOG = 201
};
