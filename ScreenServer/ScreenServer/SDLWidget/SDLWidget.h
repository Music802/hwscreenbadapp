#pragma once

#include <QWidget>
#include "CommonUtils/datapacket.h"
#include "VideoEncoderDecoder/Decoder/H264Decoder.h"

#include <thread>
#include <mutex>
extern "C"
{
#include <SDL.h>
#undef main

}
class SDLWidget : public QWidget
{
	Q_OBJECT

public:
	static bool InitSDL2();
	SDLWidget(int x,int y,int w,int h,int picWidth,int picHeight, H264ImgType imageType= H264_IMG_YUV420,QWidget *parent = Q_NULLPTR);
	~SDLWidget();
	void SetWindowSize(int w, int h);
	void SetWindowLocation(int x, int y,int w,int h);
	void UpdateYUV(std::shared_ptr<H264_IMG_Data> img);
private:
	void RenderLoop();
	void ReleaseRender();
	int H264IMG2SDLIMG(H264ImgType imgType);
private:
	int m_x, m_y;
	int m_w, m_h;
	int m_picWidth, m_picHight;
	int m_imgType;

	SDL_Window	*m_pWindow;
	SDL_Renderer *m_pRenderer;
	SDL_Texture *m_pSdlTexture;
	QWidget m_childWidget;
	
	std::thread m_threadRender;
	std::mutex m_muxRender;
	volatile bool m_rendering;
};
