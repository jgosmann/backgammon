/*!
 * \file aisettingsdialog.cpp
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

#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>

#include "ai_std_config.h"

#include "ui_aisettingsdialog.h"
#include "aisettingsdialog.h"

/////////////////////////////////////////////////////////////////////////////
/// Konstruktor
/////////////////////////////////////////////////////////////////////////////
AISettingsDialog::AISettingsDialog( QWidget *parent, Qt::WindowFlags f )
    : QDialog( parent, f )
{
  setupUi( this );
  restore_defaults();
  connect( buttonBox->button( QDialogButtonBox::RestoreDefaults ),
           SIGNAL( clicked( void ) ),
           this, SLOT( restore_defaults() ) );
}

/////////////////////////////////////////////////////////////////////////////
/// Stellt die Standardeinstellungen wieder her.
/////////////////////////////////////////////////////////////////////////////
void AISettingsDialog::restore_defaults( void )
{
  timeout->setValue( AI_STD_TIMEOUT );
  prob_checker_is_hit->setValue(
      AI_STD_RATING_FACTORS[ AIThread::PROB_CHECKER_IS_HIT ] );
  prob_cannot_move->setValue(
      AI_STD_RATING_FACTORS[ AIThread::PROB_CANNOT_MOVE ] );
  prob_op_cannot_move->setValue(
      AI_STD_RATING_FACTORS[ AIThread::PROB_OP_CANNOT_MOVE ] );
  prob_op_cannot_move_bar->setValue(
      AI_STD_RATING_FACTORS[ AIThread::PROB_OP_CANNOT_MOVE_BAR ] );
  n_points_with_checkers->setValue(
      AI_STD_RATING_FACTORS[ AIThread::N_POINTS_WITH_CHECKERS ] );
  beared_off->setValue( AI_STD_RATING_FACTORS[ AIThread::BEARED_OFF ] );
  distance_from_homeboard->setValue(
      AI_STD_RATING_FACTORS[ AIThread::DISTANCE_FROM_HOMEBOARD ] );
  checkers_in_op_homeboard->setValue(
      AI_STD_RATING_FACTORS[ AIThread::CHECKERS_IN_OP_HOMEBOARD ]);
  op_distance_from_off_game->setValue(
      AI_STD_RATING_FACTORS[ AIThread::OP_DISTANCE_FROM_OFF_GAME ] );
}
