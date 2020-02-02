#include "LoginWindow.h"
#include "Sc2AiManager.h"
#include <windows.h>
#include <ConsoleApi.h>
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    if (AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole())
    {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }
	QApplication a(argc, argv);
    Sc2AiManager MainSc2App(&a);
    MainSc2App.RunApp();
}
