/*!
 * \file aisettingsdialog.h
 * \brief Deklaration einer Klasse für einen Dialog, um KI-Einstellungen
 *        vorzunehmen.
 * \date Su Jan 28 2007
 * \author Jan Gosmann (jan@hyper-world.de)
 *
 * \b Copyright: Copyright (C) 2007 Jan Gosmann
 */

/***************************************************************************
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                   *
 *                                                                         *
 *  See the GNU General Public License for more details.                   *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program; if not, write to the Free Software            *
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.              *
 ***************************************************************************/

#ifndef AISETTINGSDIALOG_H
#define AISETTINGSDIALOG_H 1

#include <QDialog>

#include "ui_aisettingsdialog.h"

/////////////////////////////////////////////////////////////////////////////
/// \brief Dialog um die Backgammon-KI zu konfigurieren.
///
/// Diese Klasse stellt ein Dialog-Fenster zum Konfigurieren der KI bereit.
/////////////////////////////////////////////////////////////////////////////
class AISettingsDialog : public QDialog, public Ui::AISettingsDialog
{
  Q_OBJECT

  public:
    AISettingsDialog( QWidget *parent = 0, Qt::WindowFlags f = 0 );

  public slots:
    void restore_defaults( void ); ///< \brief Stellt die
                                   ///< Standardeinstellungen wieder her.
};

#endif
