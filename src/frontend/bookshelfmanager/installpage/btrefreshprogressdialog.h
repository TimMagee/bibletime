/*********
*
* In the name of the Father, and of the Son, and of the Holy Spirit.
*
* This file is part of BibleTime's source code, http://www.bibletime.info/.
*
* Copyright 1999-2015 by the BibleTime developers.
* The BibleTime source code is licensed under the GNU General Public License version 2.0.
*
**********/

#ifndef BTREFRESHPROGRESSDIALOG_H
#define BTREFRESHPROGRESSDIALOG_H

#include <QProgressDialog>

#include "backend/btinstallmgr.h"


class BtRefreshProgressDialog: public QProgressDialog {
    Q_OBJECT

    public: /* Methods: */
        BtRefreshProgressDialog(sword::InstallSource &source,
                                QWidget *parent = nullptr, Qt::WindowFlags f = nullptr);

        bool runAndDelete();

    private slots:

        void slotPercentCompleted(int, int);
        void slotCanceled();

    private: /* Fields: */

        sword::InstallSource m_source;
        BtInstallMgr m_installMgr;
};

#endif // BTREFRESHPROGRESSDIALOG_H
