#ifndef GLOGGER_PRIVATE_H
#define GLOGGER_PRIVATE_H

#include <QByteArray>
#include "GSelfOwnedThread.h"
#include "GLogger.h"
#include "GLoggerEvent.h"


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GOutputLoggerThread - поток вывода лога
class GOutputLoggerThread : public GSelfOwnedThread
{
	Q_OBJECT

private:
	GOutputLoggerThread( const QByteArray &strFileName, GLogger::LogFlags nFlags );
	~GOutputLoggerThread();

private slots:
	void onNewLoggedEvent( GLoggerEvent const& pEvent );

protected:
	bool beforeExec();
	void afterExec();

private:
	QByteArray			m_strFileName;
	GLogger::LogFlags	m_nFlags;
	int					m_hFile;

private:
	bool _open_log();
	void _close_log();

	friend class GLogger;
};

#endif // GLOGGER_PRIVATE_H
