/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBTAS_ENCODEWINDOW_H_INCLUDED
#define LIBTAS_ENCODEWINDOW_H_INCLUDED

#include <QtWidgets/QDialog>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QSpinBox>

/* Forward declaration */
struct Context;

class EncodeWindow : public QDialog {
    Q_OBJECT

public:
    EncodeWindow(Context *c, QWidget *parent = Q_NULLPTR);

    /* Update UI elements when the config has changed */
    void update_config();

private:
    Context *context;

    QLineEdit *encodePath;
    QPushButton *browseEncodePath;
    QComboBox *videoChoice;
    QSpinBox *videoBitrate;
    QComboBox *audioChoice;
    QSpinBox *audioBitrate;
    QLineEdit *ffmpegOptions;
    QSpinBox *videoFramerate;

private slots:
    void slotBrowseEncodePath();
    void slotUpdate();
    void slotOk();
    void slotDefault();
};

#endif
