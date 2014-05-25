#ifndef GSELFOWNEDTHREAD_H
#define GSELFOWNEDTHREAD_H

#include <QThread>

class GSelfOwnedThreadPrivate;

class GSelfOwnedThread : public QThread
{
	Q_OBJECT

protected:
	/*! Поскольку в конструкторе нельзя вызывать виртуальные функции,
	то конструктор должен обязательно завершаться вызовом \ref releaseSemaphore(), т.е.
	конструктор должен выглядеть примерно так:
	\code
	InheritedThread::InheritedThread() :
		GSelfOwnedThread()
	{
		// Здесь могут присутствовать дополнительные "отвязки", если они необходимы.
		m_pObject->moveToThread( thread() );

		releaseSemaphore();
	}
	\endcode
	Поток автоматически запускается в конструкторе, и в методе \ref run() уже реализован вызов \ref exec();
	Добавлены два виртуальных метода для инициализации и завершения
	\ref beforeExec() и \ref afterExec()
	*/
	GSelfOwnedThread();

public:
	~GSelfOwnedThread();

protected:
	void run();

protected:
	void releaseSemaphore();
	virtual bool beforeExec();
	virtual void afterExec();

public:
	void stopAndWait( int nExitCode = 0, unsigned long nWaitTime = ULONG_MAX );

private slots:
	void _exit( int nExitCode );

private:
	GSelfOwnedThreadPrivate *d;
};


#endif // GSELFOWNEDTHREAD_H
