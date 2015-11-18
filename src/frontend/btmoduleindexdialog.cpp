/*********
*
* This file is part of BibleTime's source code, http://www.bibletime.info/.
*
* Copyright 1999-2015 by the BibleTime developers.
* The BibleTime source code is licensed under the GNU General Public License version 2.0.
*
**********/

#include "frontend/btmoduleindexdialog.h"

#include <QApplication>
#include <QMutexLocker>
#include "backend/managers/cswordbackend.h"
#include "frontend/messagedialog.h"


bool BtModuleIndexDialog::indexAllModules(const QList<CSwordModuleInfo *> &modules)
{
    static QMutex mutex;
    QMutexLocker lock(&mutex);

    if (modules.empty()) return true;

    BtModuleIndexDialog d(modules.size());
    d.show();
    d.raise();
    return d.indexAllModulesPrivate(modules);
}

BtModuleIndexDialog::BtModuleIndexDialog(int numModules)
    : QProgressDialog(tr("Preparing to index modules..."), tr("Cancel"), 0,
                      numModules * 100, nullptr),
      m_currentModuleIndex(0)
{
    setWindowTitle(tr("Creating indices"));
    setModal(true);
}

bool BtModuleIndexDialog::indexAllModulesPrivate(const QList<CSwordModuleInfo*> &modules)
{
    bool success = true;

    QList <CSwordModuleInfo*> indexedModules;
    Q_FOREACH(CSwordModuleInfo * const m, modules) {
        Q_ASSERT(!m->hasIndex());

        /*
          Keep track of created indices to delete them on failure or
          cancellation:
        */
        indexedModules.append(m);

        connect(this, SIGNAL(canceled()),
                m,    SLOT(cancelIndexing()));
        connect(m,    SIGNAL(indexingFinished()),
                this, SLOT(slotFinished()));
        connect(m,    SIGNAL(indexingProgress(int)),
                this, SLOT(slotModuleProgress(int)));

        // Single module indexing blocks until finished:
        setLabelText(tr("Creating index for work: %1").arg(m->name()));

        try {
            m->buildIndex();
        } catch (...) {
            message::showWarning(this,
                                 tr("Indexing aborted"),
                                 tr("An internal error occurred while building "
                                    "the index."));
            success = false;
        }

        m_currentModuleIndex++;

        disconnect(this, SIGNAL(canceled()),
                   m,    SLOT(cancelIndexing()));
        disconnect(m,    SIGNAL(indexingFinished()),
                   this, SLOT(slotFinished()));
        disconnect(m,    SIGNAL(indexingProgress(int)),
                   this, SLOT(slotModuleProgress(int)));

        if (wasCanceled()) success = false;

        if (!success) break;
    }

    if (!success)
        // Delete already created indices:
        Q_FOREACH(CSwordModuleInfo * const m, indexedModules)
            if (m->hasIndex())
                m->deleteIndex();
    return success;
}

void BtModuleIndexDialog::slotModuleProgress(int percentage) {
    setValue(m_currentModuleIndex * 100 + percentage);
    qApp->processEvents();
}

void BtModuleIndexDialog::slotFinished() {
    setValue(m_currentModuleIndex * 100 + 100);
    qApp->processEvents();
}
