#include "SDLWidget.h"
#include <QVBoxLayout>

bool SDLWidget::InitSDL2() {
	//if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)!=0)
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		return false;
	}

	return true;
}

SDLWidget::SDLWidget(int x, int y, int w, int h,int picWidth, int picHeight, H264ImgType imageType,QWidget *parent)
	: QWidget(parent),
	m_x(x),
	m_y(y),
	m_w(w),
	m_h(h),
	m_pWindow(nullptr),
	m_pRenderer(nullptr),
	m_pSdlTexture(nullptr),
	m_rendering(false),
	m_picWidth(picWidth),
	m_picHight(picHeight)
{
	resize(m_w, m_h);
	move(m_x, m_y);
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(&m_childWidget);
	setLayout(mainLayout);
	this->show();
	m_imgType = H264IMG2SDLIMG(imageType);

	//auto id = this->winId();
	m_pWindow = SDL_CreateWindowFrom((void*)m_childWidget.winId());
	if (nullptr==m_pWindow)
	{
		return;
	}
	std::thread thread(std::mem_fun(&SDLWidget::RenderLoop), this);
	m_threadRender = std::move(thread);
}

SDLWidget::~SDLWidget()
{
	ReleaseRender();
}

void SDLWidget::SetWindowSize(int w, int h)
{
	m_muxRender.lock();
	m_w = w;
	m_h = h;
	resize(m_w, m_h);
	m_muxRender.unlock();
}

void SDLWidget::SetWindowLocation(int x, int y,int w,int h)
{
	m_muxRender.lock();
	m_x = x;
	m_y = y;
	move(m_x, m_y); 
	m_w = w;
	m_h = h;
	resize(m_w, m_h);
	m_muxRender.unlock();
}

void SDLWidget::UpdateYUV(std::shared_ptr<H264_IMG_Data> img)
{
	SDL_Rect rect;
	rect.x = rect.y = 0;
	rect.w = m_picWidth;
	rect.h = m_picHight;
	m_muxRender.lock();
	img->data[0][1];
	if (nullptr != m_pSdlTexture)
	{
		//SDL_UpdateTexture(m_pSdlTexture, nullptr, img.data, img.lineSize);
		SDL_UpdateYUVTexture(m_pSdlTexture,&rect,
			img->data[0],img->lineSize[0],
			img->data[1],img->lineSize[1],
			img->data[2], img->lineSize[2]);
	}
	m_muxRender.unlock();
}

void SDLWidget::RenderLoop()
{
	m_rendering = true;
	m_pRenderer = SDL_CreateRenderer(m_pWindow, -1, SDL_RENDERER_ACCELERATED);
	if (m_pRenderer==nullptr)
	{
		m_rendering = false;
		return;
	}
	//format wrong
	m_pSdlTexture = SDL_CreateTexture(m_pRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,
		m_picWidth, m_picHight);
	if (nullptr==m_pSdlTexture)
	{
		m_rendering = false;
		return;
	}
	while (m_rendering)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(30));
		m_muxRender.lock();
		SDL_RenderClear(m_pRenderer);
		SDL_RenderCopy(m_pRenderer, m_pSdlTexture, nullptr,nullptr);
		SDL_RenderPresent(m_pRenderer);
		m_muxRender.unlock();
	}
}

void SDLWidget::ReleaseRender()
{
	m_rendering = false;
	if (m_threadRender.joinable())
	{
		m_threadRender.join();
	}
	if (m_pRenderer)
	{
		SDL_DestroyRenderer(m_pRenderer);
		m_pRenderer = nullptr;
	}
	if (m_pWindow)
	{
		SDL_DestroyWindow(m_pWindow);
		m_pWindow = nullptr;
	}
}

int SDLWidget::H264IMG2SDLIMG(H264ImgType imgType)
{
	int result = 0;
	switch (imgType)
	{
	case H264_IMG_YUV420:
		result = SDL_PIXELFORMAT_IYUV;
		break;
	case H264_IMG_NV12:
		result = SDL_PIXELFORMAT_NV12;
		break;
	default:
		break;
	}
	return result;
}

