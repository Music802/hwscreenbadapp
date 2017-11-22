#pragma once
#include "ffmpeg_header.h"

void FFMPEG_Init() {
	static bool inited = false;
	if (!inited)
	{
		av_register_all();
		avdevice_register_all();
		inited = true;
	}
}
