#include <nana/gui.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/button.hpp>

using namespace nana;

int WinMain() {
    form form;
    form.caption("Tiny HTTP Server");
    form.events().click([] {
            
        });
    label label(form, rectangle{ 0, 0, 100, 20 });
    label.caption("hello!!");
    button btn(form, rectangle{ 25, 30, 100, 30 });
    btn.caption("Quit");
    btn.events().click(API::exit);

    form.show();
    exec();
    return 0;
}