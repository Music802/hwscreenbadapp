#pragma once
#include <qstring.h>

enum PLAY_CMD
{
	OPT_CONNECT,
	OPT_SET_VIDEO_INFO,
	OPT_SET_LOCATION,
	OPT_HEARTBEAT,
	OPT_CLOSE,
	OPT_CONNECT_RESULT,
	OPT_RESULT
};

struct SetVideoInfo
{
	int		width;
	int		height;
	QString sps64;
	QString pps64;
};

struct SetLocation
{
	int	x;
	int y;
	int w;
	int h;
};

struct ConnectResult
{
	int ssrc;
	int port;
};

struct Result
{
	bool result;
};