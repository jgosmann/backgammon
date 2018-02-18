/*!
 * \file netbackgammon.h
 * \brief Deklaration der NetBackgammon-Klasse.
 * \date Mo Jan 29 2007
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

#ifndef NETBACKGAMMON_H
#define NETBACKGAMMON_H 1

#include <QString>
#include <QStringList>
#include <QTcpSocket>

#include "backgammon.h"

/*< \label{NetBackgammonMsg} >*/
/////////////////////////////////////////////////////////////////////////////
/// \brief Klasse für Server-Nachrichten.
///
/// Diese Klasse enthält die Daten die in einer Nachricht an oder von dem
/// Server übertragen werden.
/////////////////////////////////////////////////////////////////////////////
class NetBackgammonMsg
{
  public:
    NetBackgammonMsg( QString type = "", QString params = "",
                      QString in_reply_to = "" );
        ///< \brief Standard-Konstruktor
    NetBackgammonMsg( QString msg ); ///< \brief Konstruktor für vom Server
                                     ///< erhaltene Nachrichten.

    inline QString get_type( void ) const { return m_type; };
    inline QString get_in_reply_to( void ) const { return m_in_reply_to; };
    inline const QStringList & get_params( void ) const { return m_params; };

    inline void set_type( QString type ) { m_type = type; };
    inline void set_in_reply_to( QString in_reply_to )
        { m_in_reply_to = in_reply_to; };
    inline void set_params( QString params )
        { m_params = params.split( ';'); };
    void set_by_raw_data( QString msg ); ///< \brief Verwendet Roh-Daten zum
                                         ///< Setzen der Werte.

    QString get_raw_data( void ) const;
        ///< \brief Gibt die Rohdaten der Nachricht zurück, so wie sie an den
        ///< Server übertragen werden würden.

  private:
    QString m_type; ///< \brief Typ der Nachricht.
    QString m_in_reply_to; ///< \brief Antwort auf Nachricht von diesem Typ.
    QStringList m_params; ///< \brief Mit der Nachricht mitgesendete
                          ///< Parameter.
};

/*< \label{NetBackgammonConnection} >*/
/////////////////////////////////////////////////////////////////////////////
/// \brief Klasse zur Kommunikation mit einem Backgammon-Server.
///
/// Diese Klasse dient zum Verbinden mit einem Backgammon-Server und dem
/// Datenaustausch mit diesem.
/////////////////////////////////////////////////////////////////////////////
class NetBackgammonConnection : public QTcpSocket
{
  Q_OBJECT

  public:
    static const quint16 STD_PORT = 30167; ///< \brief Standard-Port

    NetBackgammonConnection( bool dbg = false, QObject *parent = 0 );
    ~NetBackgammonConnection( void );

    inline bool get_dbg( void ) const { return m_dbg; };
    inline bool get_logged_in( void ) const { return m_logged_in; };
    inline bool get_joined_game( void ) const { return m_joined_game; };
    inline bool get_game_running( void ) const { return m_game_running; };
    inline QString get_username( void ) const { return m_username; };
    inline unsigned int get_game_number( void ) const
        { return m_game_number; };

  public slots:
    void log_in( QString username, QString password ); ///< \brief Am Server
                                                       ///< einloggen.
    void log_out( void ); ///< \brief Am Server ausloggen.
    void request_game_list( void ); ///< \brief Spiele abfragen.
    void request_user_list( void ); ///< \brief Benutzer abfragen.
    void chat( QString send_to, QString msg ); ///< \brief Chat-Nachricht
                                               ///< versenden.
    void create_game( QString password = "", unsigned int timeout = 0,
                      unsigned int turns = 0 ); ///< \brief Erstellt ein
                                                ///< neues Spiel auf dem
                                                ///< Server.
    void join_game( unsigned int game_number, QString password = "" );
        ///< \brief Tritt einem Spiel auf dem Server bei.
    void leave_game( void ); ///< \brief Verlässt ein Spiel.
    void start( void ); ///< \brief Startet ein Spiel.
    void confirm( void ); ///< \brief Bestätigt den Empfang eines Befehls.
    void turn( const BG::BackgammonTurn &turn ); ///< \brief Überträgt einen
                                                 ///< Zug an den Server.

    void set_dbg( bool val ); ///< \brief Setzt \ref m_dbg.

  signals:
    /*< \label{NetBackgammonConnection::received_msg(NetBackgammonMsg)} >*/
    void received_msg( NetBackgammonMsg msg );
        ///< \brief Wird gesendet, wenn der Server eine Nachricht sendet.
    void logged_in( void ); ///< \brief Wird gesendet, wenn sich der Benutzer
                            ///< eingeloggt hat.
    void logged_out( void ); ///< \brief Wird gesendet, wenn sich der
                             ///< Benutzer ausgeloggt hat.
    void joined_game( void ); ///< \brief Wird gesendet, wenn einem Spiel
                              ///< beigetreten wird.
    void left_game( void ); ///< \brief Wird gesendet, wenn ein Spiel
                            ///< verlassen wird.

  protected slots:
    void send( QString msg ); ///< \brief Sendet einen Befehl an den Server.

    void on_readyRead( void );
    void on_disconnected( void );

  private:
    bool m_dbg; ///< \brief Debug-Modus aktiviert?
    bool m_logged_in; ///< \brief Hat sich der Benutzer bereits auf dem
                      ///< Server eingeloggt?
    bool m_joined_game; ///< \brief Ist der Benutzer einem Spiel beigetreten?
    bool m_game_running; ///< \brief Läuft gerade ein Backgammon-Spiel?
    QString m_username; ///< \brief Benutzername unter dem der User
                        ///< eingeloggt ist.
    unsigned int m_max_game_number; ///< \brief Höchste an ein Netzwerkspiel
                                    ///< zugewiesene Nummer.
    unsigned int m_game_number; ///< \brief Nummer des Spiels, dem der
                                ///< Spieler beigetreten ist.
};

/*< \label{NetBackgammon} >*/
/////////////////////////////////////////////////////////////////////////////
/// \brief Klasse zur Verwaltung eines Backgammon-Spiels, welche auch Spiele
/// über einen Netzwerk-Server zulässt.
///
/// Diese Klasse dient zur Verwaltung eines Backgammon-Spiels, bei dem ein
/// Spieler über einen Backgammon-Server gesteuert wird. Es ist allerdings
/// auch weiterhin ein Spiel möglich, bei dem beide Spieler lokal gesteuert
/// werden.
/////////////////////////////////////////////////////////////////////////////
class NetBackgammon : public BG::Backgammon
{
  Q_OBJECT

  public:
    NetBackgammon( NetBackgammonConnection *connection,
                   BG::Player net_player = BG::WHITE,
                   bool is_auto_dice_roll_enabled = true,
                   QObject *parent = 0 );

    inline bool get_dice_received( void ) { return m_dice_received; };

  public slots:
    void start( void ); ///< \brief Startet das Backgammon-Spiel.

    void set_net_player( short int player ); ///< \brief Setzt
                                             ///< \ref m_net_player.

  protected slots:
    void transmit_last_turn( void ); ///< \brief Überträgt den letzten Zug.
    void process_srv_msg( NetBackgammonMsg msg );
        ///< \brief Server-Nachricht verarbeiten.

  private:
    NetBackgammonConnection *m_connection; ///< \brief Verbindung zum
                                           ///< Backgammon-Server
    /*< \label{NetBackgammon::m_net_player} >*/
    BG::Player m_net_player; ///< \brief Der über das Netzwerk gesteuerte
                             ///< Spieler.
    bool m_dice_received; ///< \brief Wurde der Würfelwurf vom Server bereits
                          ///< empfangen?
};

#endif
