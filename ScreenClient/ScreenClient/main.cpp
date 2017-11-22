#include "ScreenClient.h"
#include <QtWidgets/QApplication>
#include "VideoEncoderDecoder/ffmpeg_header.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	FFMPEG_Init();

	ScreenClient w;
	w.show();
	return a.exec();
}
