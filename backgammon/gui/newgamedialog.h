/*!
 * \file newgamedialog.h
 * \brief Enthält die Deklaration der Klasse NewGameDialog, welche das
 *        Interface zum Setzen der Einstellungen für ein neues Spiel
 *        bereitstellt.
 * \date Fr Jan 26 2007
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

#ifndef NEWGAMEDIALOG_H
#define NEWGAMEDIALOG_H 1

#include <QDialog>
#include <QStandardItemModel>

#include "aithread.h"
#include "backgammon.h"
#include "netbackgammon.h"

#include "chatwidget.h"

#include "ui_newgamedialog.h"

/////////////////////////////////////////////////////////////////////////////
/// \brief Der von dieser Klasse bereitgestellte Dialog wird bei Beginn
///        eines neuen Spieles angezeigt.
///
/// Diese Klasse stellt ein Dialog-Fenster bereit, welches bei Beginn eines
/// neuen Spieles angezeigt wird. In diesem können alle wichtigen
/// Einstellungen für das neue Spiel vorgenommen werden.
/////////////////////////////////////////////////////////////////////////////
class NewGameDialog : public QDialog, public Ui::NewGameDialog
{
  Q_OBJECT

  public:
    NewGameDialog( NetBackgammonConnection *connection,
                   QString chat_content = "", QWidget *parent = NULL,
                   Qt::WindowFlags f = NULL );
    ~NewGameDialog( void );

    inline const unsigned int get_ai_timeout( BG::Player player )
        { return m_ai_timeout[ player ]; };
    inline const double * get_ai_factors( BG::Player player )
        { return m_ai_factors[ player ]; };

    ChatWidget *chat_widget; ///< \brief Chat-Fenster

  public slots:
    void update( void ); ///< \brief Aktualisiert die GUI-Elemente gemäß den
                         ///< aktuellen Einstellungen.

    void on_button_configure_white_clicked( void )
        { configure_player( BG::WHITE,
                            combobox_type_white->currentIndex() ); };
    void on_button_configure_black_clicked( void )
        { configure_player( BG::BLACK,
                            combobox_type_black->currentIndex() ); };
    void configure_player( int player, int type );
        ///< \brief Spieler konfigurieren.

    void on_button_connect_clicked( void );
    void on_button_create_game_clicked( void );
    void on_button_join_game_clicked( void );
    void on_button_start_server_clicked( void );

    void log_in( void ); ///< \brief Loggt den User ein.

    void process_srv_msg( NetBackgammonMsg msg ); ///< \brief Verarbeitet
                                                  ///< eine Server-Nachricht.

  private:
    NetBackgammonConnection *m_connection; ///< \brief Netzwerkverbindung

    unsigned int m_ai_timeout[ 2 ]; ///< \brief Timeout-Einstellungen für die
                                    ///< KI.
    double m_ai_factors[ 2 ][ AIThread::NUM_RATING_FACTORS ];
        ///< \brief Einstellungen für die Bewertungsfaktoren der KI.

    QStandardItemModel *m_game_list_data; ///< \brief Spiel-Liste für den
                                          ///< Netzwerkmodus.
    bool m_joined_game; ///< \brief Gibt an, ob einem Netzwerkspiel
                        ///< beigetreten wurde. Wird in der update()-Funktion
                        ///< dazu benutzt, zu erkennen, ob diese Funtion
                        ///< bereits seit dem letzten Beitretn zu einem Spiel
                        ///< oder Verlassen aufgerufen wurde.
    BG::Player m_net_player; ///< \brief Welcher Spieler wird über das
                             ///< Netzwerk gesteuert?
};

#endif
