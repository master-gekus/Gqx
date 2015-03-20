#ifndef CONSOLEWIDGET_H
#define CONSOLEWIDGET_H

#include <QTextEdit>
#include <QFont>
#include <QColor>
#include <QKeySequence>

//////////////////////////////////////////////////////////////////////////////////////////////////
/*! \brief Поддержка окна консольного вывода

	На текущий момент окно выполняет следующие функции:
		- "Расцвеченный" по строкам вывод
		- Добавление строк в конец с "умным" скроллиногом
		- Всплывающее окно поиска
*/
class GConsoleWidget : public QTextEdit
{
	Q_OBJECT

public:
	explicit GConsoleWidget( QWidget *pParent = 0 );

private:
	class SearchBoxLayout;	// Он нужен только внутри, сигналов-слотов нет, так что пусть будет внутренний!

signals:

public slots:
	void find();
	void findNext();
	void findPrevious();
	void findClose();

private slots:
	void onEditFinished();

// Управление
public:
	void addLine( QString const& strLine, QFont::Weight nWeight, QColor pColor );

	bool searchBoxVisible() const;
	void setSearchBoxVisible( bool bVisible );

private:
	class SearchBox;
	SearchBox *m_pForm;

// Хелперы
private:
	void connect_sequence( QKeySequence pKeys, const char *strSlot );
	inline void connect_sequence( QKeySequence::StandardKey pKeys, const char *strSlot )
		{ connect_sequence( QKeySequence( pKeys ), strSlot ); }
};
//////////////////////////////////////////////////////////////////////////////////////////////////


#endif // CONSOLEWIDGET_H
