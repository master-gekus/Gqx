#ifndef GUIHELPERS_H
#define GUIHELPERS_H

class QWidget;
class QSettings;

#include <QWidgetList>

namespace GUiHelpers {
	void saveElementsState( QWidget *pParent, QSettings &pSettings );
	void restoreElementsState( QWidget *pParent, QSettings &pSettings );
	void enableControls( QWidget *pWindow, bool bEnable, const QWidgetList &pExclusions );
	void enableControls( QWidget *pWindow, bool bEnable, QWidget *pExclusion = 0 );
}

#endif // GUIHELPERS_H
