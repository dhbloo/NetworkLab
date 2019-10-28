#include "mainFrame.h"

#include "app.h"
#include "routerTree.h"

#include <TinyHTTPServer/httpServer.h>

MainFrame::MainFrame() : wxFrame(NULL, wxID_ANY, "Http ������", wxDefaultPosition, wxSize(870, 500))
{
    CenterOnScreen();

    wxPanel *panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizerAndFit(topSizer);

    wxStaticBoxSizer *overviewSizer = new wxStaticBoxSizer(wxHORIZONTAL, panel, "����������");
    topSizer->Add(overviewSizer, 0, wxALL, 10);

    overviewSizer->Add(new wxStaticText(panel, wxID_ANY, "������"),
                       0,
                       wxALL | wxALIGN_CENTRE_VERTICAL,
                       10);
    overviewSizer->Add(ipText = new wxTextCtrl(panel, wxID_ANY, "0.0.0.0"), 0, wxALL, 5);
    overviewSizer->Add(new wxStaticText(panel, wxID_ANY, "�˿ں�"),
                       0,
                       wxALL | wxALIGN_CENTRE_VERTICAL,
                       10);
    overviewSizer->Add(portText = new wxTextCtrl(panel, wxID_ANY, "80"), 0, wxALL, 5);
    overviewSizer->Add(startBtn = new wxButton(panel, ID_START_BTN, "����"), 0, wxALL, 5);

    wxNotebook *tabView = new wxNotebook(panel, wxID_ANY);
    topSizer->Add(tabView, 1, wxLEFT | wxRIGHT | wxEXPAND, 10);

    wxPanel *   page;
    wxBoxSizer *lineSizer;
    tabView->AddPage(page = new wxPanel(tabView, wxID_ANY), "��־");
    page->SetSizerAndFit(lineSizer = new wxBoxSizer(wxVERTICAL));
    lineSizer->Add(logText = new wxTextCtrl(page,
                                            ID_LOG,
                                            wxEmptyString,
                                            wxDefaultPosition,
                                            wxDefaultSize,
                                            wxTE_MULTILINE | wxTE_READONLY),
                   1,
                   wxALL | wxEXPAND,
                   8);
    lineSizer->Add(new wxButton(page, ID_CLEAR_BTN, "���"),
                   0,
                   wxLEFT | wxRIGHT | wxALIGN_RIGHT,
                   8);

    tabView->AddPage(page = new wxPanel(tabView, wxID_ANY), "��ǰ����");
    page->SetSizerAndFit(lineSizer = new wxBoxSizer(wxVERTICAL));
    lineSizer->Add(connList = new wxListView(page, wxID_ANY), 1, wxALL | wxEXPAND, 8);
    connList->AppendColumn("��ַ");
    connList->AppendColumn("�˿ں�", wxLIST_FORMAT_LEFT, 50);
    connList->AppendColumn("�����ֽ���");
    connList->AppendColumn("HTTP�汾");
    connList->AppendColumn("����");
    connList->AppendColumn("URL");
    connList->AppendColumn("״̬��");
    connList->AppendColumn("��Ӧ�ֽ���");
    connList->AppendColumn("�����Ӧʱ��", wxLIST_FORMAT_LEFT, 200);

    tabView->AddPage(page = new wxPanel(tabView, wxID_ANY), "·��");
    page->SetSizerAndFit(lineSizer = new wxBoxSizer(wxVERTICAL));
    routerTree = std::make_unique<RouterTree>(page);
    lineSizer->Add((wxPanel *)routerTree.get(), 1, wxALL | wxEXPAND, 8);

    redirect = std::make_unique<wxStreamToTextRedirector>(logText);
}

void MainFrame::OnClear(wxCommandEvent &event)
{
    logText->Clear();
}

void MainFrame::OnStart(wxCommandEvent &event)
{
    if (!App::GlobalHttpServer) {
        unsigned long port;
        if (!portText->GetValue().ToULong(&port) || port > 65535) {
            wxMessageBox("��Ч�˿ں�!", "����������ʧ��", wxOK | wxICON_ERROR);
            return;
        }

        try {
            App::GlobalRouter = routerTree->makeRouter();

            App::GlobalHttpServer = std::make_unique<HttpServer>(ipText->GetValue().c_str(),
                                                                 (uint16_t)port,
                                                                 *App::GlobalRouter,
                                                                 std::cout);

            App::GlobalHttpServer->start();

            routerTree->enable(false);
            startBtn->SetLabelText("ֹͣ");
            ipText->Enable(false);
            portText->Enable(false);
        }
        catch (std::exception e) {
            wxMessageBox("��Ч����!", "����������ʧ��", wxOK | wxICON_ERROR);
        }
    }
    else {
        std::thread([&] {
            App::GlobalHttpServer.reset();
            startBtn->Enable(true);
            ipText->Enable(true);
            portText->Enable(true);
            routerTree->enable(true);
        }).detach();

        startBtn->SetLabelText("����");
        startBtn->Enable(false);
    }
}

void MainFrame::OnRefreshList(wxCommandEvent &event)
{
    connList->DeleteAllItems();
    if (App::GlobalHttpServer) {
        auto clients = App::GlobalHttpServer->getCurrentClients();
        for (auto &client : clients) {
            using std::to_string;

            long itemIdx = connList->InsertItem(0, client->conn.ip_addr());
            connList->SetItem(itemIdx, 1, to_string(client->conn.port()));
            if (client->totalBytesReceived == 0)
                continue;

            connList->SetItem(itemIdx, 2, to_string(client->totalBytesReceived));
            connList->SetItem(itemIdx, 3, client->request.version);
            connList->SetItem(itemIdx, 4, client->request.methodStr);
            connList->SetItem(itemIdx, 5, client->request.url);
            if (client->response.statusCode)
                connList->SetItem(itemIdx,
                                  6,
                                  to_string(client->response.statusCode) + " "
                                      + client->response.statusInfo());
            connList->SetItem(itemIdx, 7, to_string(client->totalBytesSent));
            try {
                connList->SetItem(itemIdx, 8, client->response.headers.at("Date"));
            }
            catch (...) {
            }
        }
    }
}

void MainFrame::OnClose(wxCloseEvent &event)
{
    if (App::GlobalHttpServer) {
        if (wxMessageBox("��������������... ȷ�Ϲر�?", "ȷ�Ϲر�", wxICON_WARNING | wxYES_NO)
            == wxYES) {
            std::thread([&] {
                App::GlobalHttpServer.reset();
                Close(true);
            }).detach();
        }
        event.Veto();  // ȡ���˳�
    }
    else
        event.Skip();
}

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame) EVT_BUTTON(ID_START_BTN, MainFrame::OnStart)
    EVT_BUTTON(ID_CLEAR_BTN, MainFrame::OnClear) EVT_TEXT(ID_LOG, MainFrame::OnRefreshList)
        EVT_CLOSE(MainFrame::OnClose) wxEND_EVENT_TABLE()