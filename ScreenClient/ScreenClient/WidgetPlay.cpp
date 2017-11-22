#include "WidgetPlay.h"

WidgetPlay::WidgetPlay(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	QRegExp exp("[0-9]+$");

	QValidator *validator = new QRegExpValidator(exp, ui.lineEdit_LEFT);
	ui.lineEdit_LEFT->setValidator(validator);
	validator = new QRegExpValidator(exp, ui.lineEdit_TOP);
	ui.lineEdit_TOP->setValidator(validator);
	validator = new QRegExpValidator(exp, ui.lineHeight);
	ui.lineHeight->setValidator(validator);
	validator = new QRegExpValidator(exp, ui.lineWidth);
	ui.lineWidth->setValidator(validator);

	QObject::connect(ui.pushButton, &QPushButton::clicked, this, &WidgetPlay::moveClicked);
}

WidgetPlay::~WidgetPlay()
{
}


void WidgetPlay::moveClicked() {
	auto x = ui.lineEdit_LEFT->text().toInt();
	auto y = ui.lineEdit_TOP->text().toInt();
	auto w = ui.lineWidth->text().toInt();
	auto h = ui.lineHeight->text().toInt();
	if (w<100|h<100)
	{
		return;
	}
	MoveToPos(x, y, w, h);
}

void WidgetPlay::SetDefaultValues(int x, int y, int w, int h) {
	ui.lineEdit_LEFT->setText(QString::number(x));
	ui.lineEdit_TOP->setText(QString::number(y));
	ui.lineWidth->setText(QString::number(w));
	ui.lineHeight->setText(QString::number(h));
}

