#pragma once

#include <QWidget>
#include "ui_WidgetPlay.h"

class WidgetPlay : public QWidget
{
	Q_OBJECT

public:
	WidgetPlay(QWidget *parent = Q_NULLPTR);
	~WidgetPlay();
signals:
	void MoveToPos(int x, int y,int w,int h);
	public slots:
	void SetDefaultValues(int x, int y, int w, int h);
	private slots:
	void moveClicked();
private:
	Ui::WidgetPlay ui;
};
