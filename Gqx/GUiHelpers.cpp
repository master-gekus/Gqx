#include <QWidget>
#include <QTreeView>
#include <QSplitter>
#include <QTableWidget>
#include <QHeaderView>
#include <QSettings>
#include <QDir>
#include <QIcon>

#include "GUiHelpers.h"

void
GUiHelpers::enableControls( QWidget *pWindow, bool bEnable, QWidget *pExclusion )
{
  QWidgetList pWidgets  = pWindow->findChildren<QWidget*>();

  foreach( QWidget *pWidget, pWidgets ) {
    if( pWidget == pExclusion ) continue;
    pWidget->setEnabled( bEnable );
  }
}

void
GUiHelpers::enableControls( QWidget *pWindow, bool bEnable, QWidgetList const& pExclusions )
{
  QWidgetList pWidgets  = pWindow->findChildren<QWidget*>();

  foreach( QWidget *pWidget, pWidgets ) {
    if( pExclusions.contains( pWidget ) ) continue;
    pWidget->setEnabled( bEnable );
  }
}

void
GUiHelpers::saveElementsState( QWidget *pParent, QSettings &pSettings )
{
  foreach( QTreeView *pTree, pParent->findChildren<QTreeView*>() )
    pSettings.setValue( pTree->objectName() + "-column-widths", pTree->header()->saveState() );

  foreach( QSplitter *pSplitter, pParent->findChildren<QSplitter*>() )
    pSettings.setValue( pSplitter->objectName() + "-splitter-state", pSplitter->saveState() );

  foreach( QTabWidget *pTab, pParent->findChildren<QTabWidget*>() ) {
    QWidget *pCur = pTab->currentWidget();
    pSettings.setValue( pTab->objectName() + "-current-tab", pCur ? pCur->objectName() : QString() );
  }
}

void
GUiHelpers::restoreElementsState( QWidget *pParent, QSettings &pSettings )
{
  foreach( QTreeView *pTree, pParent->findChildren<QTreeView*>() ) {
//		bool bStretch = pTree->header()->stretchLastSection();
    pTree->header()->restoreState(
      pSettings.value( pTree->objectName() + "-column-widths" ).toByteArray()
    );
//		pTree->header()->setStretchLastSection( bStretch );
  }

  foreach( QSplitter *pSplitter, pParent->findChildren<QSplitter*>() )
    pSplitter->restoreState(
      pSettings.value( pSplitter->objectName() + "-splitter-state" ).toByteArray()
    );

  foreach( QTabWidget *pTab, pParent->findChildren<QTabWidget*>() ) {
    QString strCurTab = pSettings.value( pTab->objectName() + "-current-tab" ).toString();
    if( strCurTab.isEmpty() ) continue;
    QWidget *pCurTab = pParent->findChild<QWidget*>( strCurTab );
    if( 0 != pCurTab )
      pTab->setCurrentWidget( pCurTab );
  }
}

void
GUiHelpers::loadIcon(QIcon& icon, const QString& path)
{
  if (!icon.isNull())
    return;

  QDir folder(path);
  foreach (const QString& file_name, folder.entryList())
    icon.addFile(folder.filePath(file_name));

  Q_ASSERT(!icon.isNull());
}
