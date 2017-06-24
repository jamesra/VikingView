#ifndef CHOOSESTRUCTURESDIALOG_H
#define CHOOSESTRUCTURESDIALOG_H
 
#include <QDialog>
#include <QFileDialog>

namespace Ui {
class ChooseStructuresDialog;
}

class ChooseStructuresDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChooseStructuresDialog(QWidget *parent = 0);
    ~ChooseStructuresDialog();

	static QString getStructureIDs(QWidget *parent, const QString &title,
		const QString &text = QString(), bool *ok = Q_NULLPTR,
		Qt::WindowFlags flags = Qt::WindowFlags());

	QString StructureIDs;

public Q_SLOTS:
	
	void on_browse_button_pressed();
	void on_structure_text_changed();

private:
	/// designer form
    Ui::ChooseStructuresDialog *ui;
};

#endif // CHOOSESTRUCTURESDIALOG_H
