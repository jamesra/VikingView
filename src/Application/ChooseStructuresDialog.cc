#include "choosestructuresdialog.h"
#include "ui_choosestructuresdialog.h"

ChooseStructuresDialog::ChooseStructuresDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChooseStructuresDialog)
{
    ui->setupUi(this);

	connect(this->ui->btnBrowse, SIGNAL(pressed()), this, SLOT(on_browse_button_pressed()));
	connect(this->ui->textStructures, SIGNAL(textChanged()), this, SLOT(on_structure_text_changed()));
}

ChooseStructuresDialog::~ChooseStructuresDialog()
{
    delete ui;
}

QString ChooseStructuresDialog::getStructureIDs(QWidget * parent, const QString & title, const QString & text, bool * ok, Qt::WindowFlags flags)
{
	ChooseStructuresDialog dlg(parent);
	*ok = false;
	if (dlg.exec())
	{
		*ok = true;
		return dlg.StructureIDs;
	}

	return QString();
}

void ChooseStructuresDialog::on_structure_text_changed()
{
	StructureIDs = this->ui->textStructures->toPlainText();
}

void ChooseStructuresDialog::on_browse_button_pressed()
{
	QFileDialog dlg(this->parentWidget(), Qt::Dialog);
	
	dlg.setFileMode(QFileDialog::ExistingFile);

	if (dlg.exec())
	{
		QStringList files = dlg.selectedFiles();
		QString new_value;
		foreach(QString f, files)
		{
			if (new_value.length() > 0)
				new_value += " ";

			new_value += f;
		}

		StructureIDs = new_value;

		this->ui->textStructures->setText(new_value);
	}
}
