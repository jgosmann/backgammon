/*!
 * \file mainwindow.h
 * \brief Deklaration der MainWindow-Klasse beerbt, von der im
 *        Qt-Designer erstellten MainWindow-Klasse.
 * \date Mi Nov 15 2006
 * \author Jan Gosmann (jan@hyper-world.de)
 *
 * \b Copyright: Copyright (C) 2006 - 2007 Jan Gosmann
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H 1

#include <QAbstractSocket>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QSettings>
#include <QStandardItemModel>

#include "ai.h"
#include "backgammon.h"
#include "backgammonwidget.h"
#include "netbackgammon.h"

#include "chatwidget.h"
#include "dicewidget.h"

#include "ui_mainwindow.h"

/////////////////////////////////////////////////////////////////////////////
/// \brief Klasse des Hauptfensters.
///
/// Diese Klasse implementiert das Hauptfenster des Programms.
/////////////////////////////////////////////////////////////////////////////
class MainWindow : public QMainWindow, private Ui::MainWindow
{
  Q_OBJECT

  public:
    /// \brief Enumeration zur Angabe verschiedener "Arten" von Spielern
    /// (menschlicher, PC, ...).
    ///
    /// Enumeration zur Angabe verschiedener "Arten" von Spielern
    /// (menschlicher, PC, ...).
    enum PlayerType
      {
        HUMAN_PLAYER = 0,  ///< Menschlicher Spieler.
        AI_PLAYER = 1,     ///< Künstliche Intelligenz.
        NETWORK_PLAYER = 2 ///< Netzwerk-Spieler.
      };

    MainWindow( QWidget *parent = 0, Qt::WindowFlags flags = 0 );
    ~MainWindow( void );

  public slots:
    void init_new_game( void ); ///< \brief Startet ein neues
                                ///< Backgammon-Spiel.
    void start_next_round( void ); ///< \brief Startet die nächste Runde.
    void update_stats( void ); ///< \brief Aktualisiert die Statistiken.

    void on_action_color_white_triggered( void );
        ///< \brief Setzt die Farbe des weißen Spielers.
    void on_action_color_black_triggered( void );
        ///< \brief Setzt die Farbe des schwarzen Spielers.
    void on_action_color_pen_triggered( void );
        ///< \brief Setzt die Umrandungsfarbe
    void on_action_color_bar_triggered( void );
        ///< \brief Setzt die Farbe der Bar.
    void on_action_color_bg_triggered( void );
        ///< \brief Setzt die Hintergrundsfarbe.

    void on_action_german_triggered( void )
      {
        QSettings settings;
        settings.setValue( "language", "de" );
        QMessageBox::information( this, "Restart needed",
                                  "Please restart the program." );
      }
    void on_action_english_triggered( void )
      {
        QSettings settings;
        settings.setValue( "language", "en" );
        QMessageBox::information( this, "Restart needed",
                                  "Please restart the program." );
      }

    void on_action_show_info_triggered( void ); ///< \brief Zeigt das
                                                ///< Info-Fenster an.
    void on_action_show_license_triggered( void ); ///< \brief Zeigt die
                                                   ///< Lizenz an.

  protected slots:
    void next_player( void ); ///< \brief Nächster Zug.
    void roll_dice( void ); ///< \brief Zeigt den neuen Würfelwurf mit
                            ///< \ref m_d1 und \ref m_d2 an.
    void refresh_turn_list( void ); ///< \brief Aktualisiert die Anzeige der
                                    ///< Zugliste.
    void show_illegal_move_in_statusbar( int reason );
      ///< \brief Zeigt einen ungültigen Zug in der Statusbar an.
    void show_winner( void ); ///< \brief Zeigt den Gewinner der Runde an.

    void refresh_dice( void ); ///< \brief Aktualisiert das Würfelergebnis,
                               ///< wenn bei einem Netzwerkspiel dieses vom
                               ///< Server gesendet wird.

    void net_msg_received( NetBackgammonMsg msg ); ///< \brief Verarbeitet
                                                   ///< Nachrichten des
                                                   ///< Backgammon-Servers

    void on_actionUndo_triggered( void );

  protected:
    QString conv_pos( short int pos, BG::Player player );
        ///< \brief Konvertiert eine Programm interne Position in die
        ///< tatsächliche.

  private:
    NetBackgammonConnection m_net_connection; ///< \brief Netzwerk-Verbindung
                                              ///< für \ref m_game.
    NetBackgammon m_game; ///< \brief Das Backgammon-Spiel, das vom Programm
                          ///< verwaltet wird.
    BackgammonWidget *m_game_disp; ///< \brief Visualisierung des
                                   ///< Spielstands.
    ChatWidget *m_chat_widget; ///< \brief Chat-Fenster
    bool m_dice_rolled; ///< \brief Wurde bereits gewürfelt?
    DiceWidget *m_d1; ///< \brief Zur Anzeige des ersten Würfels.
    DiceWidget *m_d2; ///< \brief Zur Anzeige des zweiten Würfels.
    QLabel *m_player_disp; ///< \brief Zeigt an, welcher Spieler am Zug ist.
    QStandardItemModel *m_turn_list_model; ///< \brief Zugliste.

    PlayerType m_player_type[ 2 ]; ///< \brief Gibt an, welcher Spieler von
                                   ///< einem Menschen, von der KI oder über
                                   ///< das Netzwerk gesteuert wird.
    AI *m_ai[ 2 ]; ///< \brief Pointer, die gegebenenfalls auf Instanzen der
                   ///< KI für den jeweiligen Spieler zeigen.
    bool m_auto_dice_roll[ 2 ]; ///< \brief Gibt an, ob für einen Spieler
                                ///< automatisch gewürfelt werden soll.
    bool m_show_blocking_msgs[ 2 ];
        ///< \brief Gibt an, für welchen Spieler blockierende Nachrichten
        ///< angezeigt werden soll. (Die einzige solche Nachricht tritt auf,
        ///< wenn ein Spieler zugunfähig ist.)

    unsigned int m_round; ///< \brief Gespielte Runden.
    unsigned int m_wins[ 2 ]; ///< \brief Punkte, die die Spieler erreicht
                              ///< haben.
};

#endif
