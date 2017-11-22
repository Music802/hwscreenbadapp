#include "ScreenServer.h"
#include "Config.h"
#include <QtWidgets/QApplication>
#include "VideoEncoderDecoder/ffmpeg_header.h"
#include "SDLWidget/SDLWidget.h"

int main(int argc, char *argv[])
{
	FFMPEG_Init();
	SDLWidget::InitSDL2();
	QApplication a(argc, argv);
	QString configName = "svrConfig.json";
	if (argc>1)
	{
		configName = argv[1];
	}
	Config::GetInstance().SetFileUrl(configName);
	ScreenServer w;
	//SDLWidget sdlwidght(0, 0, 200, 200, 200, 200);
	w.show();
	return a.exec();
}
