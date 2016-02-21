#ifndef GUIHELPERS_H
#define GUIHELPERS_H

class QWidget;
class QSettings;
class QIcon;

#include <QWidgetList>

namespace GUiHelpers {
  void saveElementsState( QWidget *pParent, QSettings &pSettings );
  void restoreElementsState( QWidget *pParent, QSettings &pSettings );
  void enableControls( QWidget *pWindow, bool bEnable, const QWidgetList &pExclusions );
  void enableControls( QWidget *pWindow, bool bEnable, QWidget *pExclusion = 0 );
  void loadIcon(QIcon& icon, const QString& path);
}

#endif // GUIHELPERS_H
