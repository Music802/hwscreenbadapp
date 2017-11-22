#pragma once

#include <QWidget>
#include "ui_WidgetSetup.h"

class WidgetSetup : public QWidget
{
	Q_OBJECT

public:
	WidgetSetup(QWidget *parent = Q_NULLPTR);
	~WidgetSetup();
	QRect GetDstRect();
signals:
	void Setuped(int x, int y, int w, int h, int dst_x, int dst_y, int dst_w, int dst_h);
	private slots:
	void setupClicked();
private:
	Ui::WidgetSetup ui;
	void setInputNumber();
	QRect m_dstRect;
	bool m_useTCP;
};
