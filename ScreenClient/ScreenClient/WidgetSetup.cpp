#include "WidgetSetup.h"
#include <qmessagebox.h>

WidgetSetup::WidgetSetup(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	QObject::connect(ui.BTN_START,&QPushButton::clicked,
		this, &WidgetSetup::setupClicked);

	setInputNumber();
}

WidgetSetup::~WidgetSetup()
{
}

QRect WidgetSetup::GetDstRect()
{
	return m_dstRect;
}

void WidgetSetup::setInputNumber()
{
	QRegExp exp("[0-9]+$");

	QValidator *validator = new QRegExpValidator(exp, ui.lineEdit_SRC_TOP);
	ui.lineEdit_SRC_TOP->setValidator(validator);
	validator = new QRegExpValidator(exp, ui.lineEdit_SRC_LEFT);
	ui.lineEdit_SRC_LEFT->setValidator(validator);
	validator = new QRegExpValidator(exp, ui.lineEdit_SRC_WIDTH);
	ui.lineEdit_SRC_WIDTH->setValidator(validator);
	validator = new QRegExpValidator(exp, ui.lineEdit_SRC_HEIGHT);
	ui.lineEdit_SRC_HEIGHT->setValidator(validator);

	validator = new QRegExpValidator(exp, ui.lineEdit_DST_TOP);
	ui.lineEdit_DST_TOP->setValidator(validator);
	validator = new QRegExpValidator(exp, ui.lineEdit_DST_LEFT);
	ui.lineEdit_DST_LEFT->setValidator(validator);
	validator = new QRegExpValidator(exp, ui.lineEdit_DST_WIDTH);
	ui.lineEdit_DST_WIDTH->setValidator(validator);
	validator = new QRegExpValidator(exp, ui.lineEdit_DST_HEIGHT);
	ui.lineEdit_DST_HEIGHT->setValidator(validator);

	setTabOrder(ui.lineEdit_SRC_LEFT, ui.lineEdit_SRC_TOP);
	setTabOrder(ui.lineEdit_SRC_TOP, ui.lineEdit_SRC_WIDTH);
	setTabOrder(ui.lineEdit_SRC_WIDTH, ui.lineEdit_SRC_HEIGHT);
	setTabOrder(ui.lineEdit_SRC_HEIGHT, ui.lineEdit_DST_LEFT);
	setTabOrder(ui.lineEdit_DST_LEFT, ui.lineEdit_DST_TOP);
	setTabOrder(ui.lineEdit_DST_WIDTH, ui.lineEdit_DST_HEIGHT);

	//default vaules

	ui.lineEdit_SRC_TOP->setText("100");
	ui.lineEdit_SRC_LEFT->setText("200");
	ui.lineEdit_SRC_WIDTH->setText("800");
	ui.lineEdit_SRC_HEIGHT->setText("600");

	ui.lineEdit_DST_TOP->setText("100");
	ui.lineEdit_DST_LEFT->setText("200");
	ui.lineEdit_DST_WIDTH->setText("800");
	ui.lineEdit_DST_HEIGHT->setText("600");
}

void WidgetSetup::setupClicked()
{
	//check input
	bool paramValid = true;
	int x, y, w, h,
		dx, dy, dw, dh;
	do
	{
		x = ui.lineEdit_SRC_LEFT->text().toInt();
		y = ui.lineEdit_SRC_TOP->text().toInt();
		w = ui.lineEdit_SRC_WIDTH->text().toInt();
		h = ui.lineEdit_SRC_HEIGHT->text().toInt();
		
		dx = ui.lineEdit_DST_LEFT->text().toInt();
		dy = ui.lineEdit_DST_TOP->text().toInt();
		dw = ui.lineEdit_DST_WIDTH->text().toInt();
		dh = ui.lineEdit_DST_HEIGHT->text().toInt();

		int	minSize = 16;

		if (w<minSize ||h<minSize ||dw< minSize ||dh< minSize)
		{
			paramValid = false;
			break;
		}

	} while (0);
	if (paramValid==false)
	{
		QMessageBox::warning(nullptr, "warning", "invaid param", QMessageBox::Yes , QMessageBox::Yes);
		return;
	}
	m_dstRect.setX(dx);
	m_dstRect.setY(dy);
	m_dstRect.setWidth(dw);
	m_dstRect.setHeight(dh);
	Setuped(x, y, w, h, dx, dy, dw, dh);
}