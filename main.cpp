#include "LoginWindow.h"
#include "Sc2AiManager.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
    Sc2AiManager MainSc2App(&a);
    MainSc2App.RunApp();
}
