#include <QString>
#include <QProcess>
#include <KConfigGroup>
#include <KConfig>
#include <QTimer>
#include <QList>
#include "ktimerjob.h"
#include "ktimerpref.h"

struct KTimerJobPrivate {
    unsigned delay;
    QString command;
    bool loop;
    bool oneInstance;
    unsigned value;
    KTimerJob::States state;
    QList<QProcess *> processes;
    void *user;

    QTimer *timer;
};

KTimerJob::KTimerJob( QObject *parent)
    : QObject( parent )
{
    d = new KTimerJobPrivate;

    d->delay = 100;
    d->loop = false;
    d->oneInstance = true;
    d->value = 100;
    d->state = Stopped;
    d->user = 0;

    d->timer = new QTimer( this );
    connect(d->timer, &QTimer::timeout, this, &KTimerJob::timeout);
}

KTimerJob::~KTimerJob()
{
    delete d;
}

unsigned KTimerJob::delay() const
{
    return d->delay;
}

QString KTimerJob::command() const
{
    return d->command;
}

bool KTimerJob::loop() const
{
    return d->loop;
}

bool KTimerJob::oneInstance() const
{
    return d->oneInstance;
}

unsigned KTimerJob::value() const
{
    return d->value;
}

KTimerJob::States KTimerJob::state() const
{
    return d->state;
}

void *KTimerJob::user()
{
    return d->user;
}

void KTimerJob::setUser( void *user )
{
    d->user = user;
}

void KTimerJob::load( KConfig *cfg, const QString& grp )
{
	KConfigGroup groupcfg = cfg->group(grp);
    setDelay( groupcfg.readEntry( "Delay", 100 ) );
    setCommand( groupcfg.readPathEntry( "Command", QString() ) );
    setLoop( groupcfg.readEntry( "Loop", false ) );
    setOneInstance( groupcfg.readEntry( "OneInstance", d->oneInstance ) );
    setState( (States)groupcfg.readEntry( "State", (int)Stopped ) );
}

void KTimerJob::save( KConfig *cfg, const QString& grp )
{
	KConfigGroup groupcfg = cfg->group(grp);
    groupcfg.writeEntry( "Delay", d->delay );
    groupcfg.writePathEntry( "Command", d->command );
    groupcfg.writeEntry( "Loop", d->loop );
    groupcfg.writeEntry( "OneInstance", d->oneInstance );
    groupcfg.writeEntry( "State", (int)d->state );
}

// Format given seconds to hour:minute:seconds and return QString
QString KTimerJob::formatTime( int seconds ) const
{
    int h, m, s;
    secondsToHMS( seconds, &h, &m, &s );
    return QStringLiteral( "%1:%2:%3" ).arg( h ).arg( m, 2, 10, QLatin1Char( '0' ) ).arg( s,2, 10, QLatin1Char( '0' ) );
}

// calculate seconds from hour, minute and seconds, returns seconds
int KTimerJob::timeToSeconds( int hours, int minutes, int seconds ) const
{
    return hours * 3600 + minutes * 60 + seconds;
}

// calculates hours, minutes and seconds from given secs.
void KTimerJob::secondsToHMS( int secs, int *hours, int *minutes, int *seconds ) const
{
    int s = secs;
    (*hours) = s / 3600;
    s = s % 3600;
    (*minutes) = s / 60;
    (*seconds) = s % 60;
}

void KTimerJob::setDelay( unsigned sec )
{
    if( d->delay!=sec ) {
        d->delay = sec;

        if( d->state==Stopped )
            setValue( sec );

        emit delayChanged( this, sec );
        emit changed( this );
    }
}

void KTimerJob::setDelay( int sec )
{
    setDelay( (unsigned)sec );
}

void KTimerJob::setCommand( const QString &cmd )
{
    if( d->command!=cmd ) {
        d->command = cmd;
        emit commandChanged( this, cmd );
        emit changed( this );
    }
}

void KTimerJob::setLoop( bool loop )
{
    if( d->loop!=loop ) {
        d->loop = loop;
        emit loopChanged( this, loop );
        emit changed( this );
    }
}

void KTimerJob::setOneInstance( bool one )
{
    if( d->oneInstance!=one ) {
        d->oneInstance = one;
        emit oneInstanceChanged( this, one );
        emit changed( this );
    }
}

void KTimerJob::setValue( unsigned value )
{
    if( d->value!=value ) {
        d->value = value;
        emit valueChanged( this, value );
        emit changed( this );
    }
}

void KTimerJob::setValue( int value )
{
    setValue( (unsigned)value );
}

void KTimerJob::setState( KTimerJob::States state )
{
    if( d->state!=state ) {
        if( state==Started )
            d->timer->start( 1000 );
        else
            d->timer->stop();

        if( state==Stopped )
            setValue( d->delay );

        d->state = state;
        emit stateChanged( this, state );
        emit changed( this );
    }
}

void KTimerJob::pause()
{
    setState( Paused );
}

void KTimerJob::stop()
{
    setState( Stopped );
}

void KTimerJob::start()
{
    setState( Started );
}

void KTimerJob::fire()
{
    if( !d->oneInstance || d->processes.isEmpty() ) {
        QProcess *proc = new QProcess;
        d->processes.append( proc );
        connect(proc, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &KTimerJob::processExited);
        if (!d->command.simplified ().isEmpty()) {
	        proc->start(d->command);
	        emit fired( this );
        }
        if(proc->state() == QProcess::NotRunning) {
            const int i = d->processes.indexOf( proc);
            if (i != -1)
                delete d->processes.takeAt(i);
            emit error( this );
            emit finished( this, true );
        }
    }
}

void KTimerJob::timeout()
{
    if( d->state==Started && d->value!=0 ) {
        setValue( d->value-1 );
        if( d->value==0 ) {
            fire();
            if( d->loop )
                setValue( d->delay );
            else
                stop();
        }
    }
}

void KTimerJob::processExited(int, QProcess::ExitStatus status)
{
	QProcess * proc = static_cast<QProcess*>(sender());
    const bool ok = status==0;
    const int i = d->processes.indexOf( proc);
    if (i != -1)
        delete d->processes.takeAt(i);

    if( !ok ) emit error( this );
    emit finished( this, !ok );
}



