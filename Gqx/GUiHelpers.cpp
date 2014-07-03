#include <QWidget>
#include <QSettings>

#include "GUiHelpers.h"

void GUiHelpers::enableControls( QWidget *pWindow, bool bEnable, QWidget *pExclusion )
{
	QWidgetList pWidgets  = pWindow->findChildren<QWidget*>();

	foreach( QWidget *pWidget, pWidgets ) {
		if( pWidget == pExclusion ) continue;
		pWidget->setEnabled( bEnable );
	}
}

void GUiHelpers::enableControls( QWidget *pWindow, bool bEnable, QWidgetList const& pExclusions )
{
	QWidgetList pWidgets  = pWindow->findChildren<QWidget*>();

	foreach( QWidget *pWidget, pWidgets ) {
		if( pExclusions.contains( pWidget ) ) continue;
		pWidget->setEnabled( bEnable );
	}
}
