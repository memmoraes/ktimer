#ifndef KTIMERPREF_H_INCLUDED
#define KTIMERPREF_H_INCLUDED

#include <QDialog>
#include <QTreeWidgetItem>
#include <KStatusNotifierItem>
#include <KHelpClient>
#include <KStandardGuiItem>
#include <KStandardAction>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KLineEdit>
#include "ui_prefwidget.h"
#include "ktimerjob.h"

class QTreeWidgetItem;

class KTimerPref : public QDialog, public Ui::PrefWidget
{
    Q_OBJECT
 public:
    KTimerPref( QWidget *parent=0);
    virtual ~KTimerPref();

 public slots:
    void exit();
    void done(int result);

 protected slots:
    void add();
    void remove();
    void help();
    void currentChanged( QTreeWidgetItem * , QTreeWidgetItem *);

    void saveJobs( KConfig *cfg );
    void loadJobs( KConfig *cfg );
    void saveAllJobs();

 private slots:
    void jobChanged( KTimerJob *job );
    void jobFinished( KTimerJob *job, bool error );
    void delayChanged();

 private:
    struct KTimerPrefPrivate *d;
};

#endif
