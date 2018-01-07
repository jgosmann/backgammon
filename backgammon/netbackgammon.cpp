/*!
 * \file netbackgammon.cpp
 * \brief Implementation der NetBackgammon-Klasse.
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

#include <cassert>
#include <iostream>

#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QTcpSocket>

#include "backgammon.h"

#include "netbackgammon.h"

using namespace std;

//===========================================================================
// NetBackgammonMsg
//===========================================================================
/////////////////////////////////////////////////////////////////////////////
/// Standard-Konstruktor
/////////////////////////////////////////////////////////////////////////////
NetBackgammonMsg::NetBackgammonMsg( QString type, QString params,
                                    QString in_reply_to )
    : m_type( type ), m_in_reply_to( in_reply_to ), m_params( params )
{

}

/////////////////////////////////////////////////////////////////////////////
/// Konstruktor, der die Nachricht \a msg in der Form wie sie vom
/// Backgammon-Server gesendet wird in eine \ref NetBackgammonMsg Klasse
/// konvertiert.
/////////////////////////////////////////////////////////////////////////////
NetBackgammonMsg::NetBackgammonMsg( QString msg )
{
  set_by_raw_data( msg );
}

/////////////////////////////////////////////////////////////////////////////
/// Verwendet Roh-Daten vom Server zum Setzen der Member-Variablen.
/////////////////////////////////////////////////////////////////////////////
void NetBackgammonMsg::set_by_raw_data( QString msg )
{
  m_type = msg;
  m_in_reply_to = "";

  if( !msg.contains( ':' ) )
    {
      m_type = "";
      m_params.clear();
      return;
    }

  m_type.truncate( m_type.indexOf( ':' ) );

  msg.remove( 0, msg.indexOf( ':' ) + 1 );
  msg.remove( QRegExp( "[\r\n]" ) );
  m_params = msg.split( ';' );

  if( m_type == "CONFIRM" || m_type == "ERROR" || m_type == "INFO" )
    {
      if( m_params.size() > 0 )
        m_in_reply_to = m_params[ 0 ];
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Gibt die Daten so zurück, wie sie an den Server gesendet würden. Am Ende
/// der Rückgabe befindet sich allerdings keine Newline-Zeichen.
/////////////////////////////////////////////////////////////////////////////
QString NetBackgammonMsg::get_raw_data( void ) const
{
  QString ret_val = m_type + ":";
  unsigned int i;

  for( i = 0; i < static_cast< unsigned int >( m_params.size() ); i++ )
    {
      if( i > 0 )
        ret_val += ";";
      ret_val += m_params[ i ];
    }

  return ret_val;
}

//===========================================================================
// NetBackgammonConnection
//===========================================================================
/////////////////////////////////////////////////////////////////////////////
/// Konstruktor, \a dbg gibt an, ob Debugausgabe produziert werden soll.
/////////////////////////////////////////////////////////////////////////////
NetBackgammonConnection::NetBackgammonConnection( bool dbg, QObject *parent )
    : QTcpSocket( parent ), m_dbg( dbg ), m_logged_in( false ),
      m_joined_game( false ), m_game_running( false ),
      m_max_game_number( 0 ), m_game_number( 0 )
{
  connect( this, SIGNAL( readyRead( void ) ),
           this, SLOT( on_readyRead( void ) ) );
  connect( this, SIGNAL( disconnected( void ) ),
           this, SLOT( on_disconnected( void ) ) );
}

/////////////////////////////////////////////////////////////////////////////
/// Destruktor
/////////////////////////////////////////////////////////////////////////////
NetBackgammonConnection::~NetBackgammonConnection( void )
{
  if( m_logged_in )
    {
      log_out();
      waitForBytesWritten( 30000 );
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Versucht sich am Backgammon-Server mit dem Benutzername \a username
/// und dem Passwort \a password anzumelden.
/////////////////////////////////////////////////////////////////////////////
void NetBackgammonConnection::log_in( QString username, QString password )
{
  send( "LOGIN:" + username + ";" + password + ";\r\n" );
  m_username = username;
}

/////////////////////////////////////////////////////////////////////////////
/// Loggt den gerade eingeloggten Benutzer am Backgammon-Server aus.
/////////////////////////////////////////////////////////////////////////////
void NetBackgammonConnection::log_out( void )
{
  send( "LOGOUT\r\n" );
}

/////////////////////////////////////////////////////////////////////////////
/// Fragt die aktuell offenen Spiele vom Server ab.
/////////////////////////////////////////////////////////////////////////////
void NetBackgammonConnection::request_game_list( void )
{
  send( "LIST\r\n" );
}

/////////////////////////////////////////////////////////////////////////////
/// Fragt die am Server angemeldeten Benutzer ab.
/////////////////////////////////////////////////////////////////////////////
void NetBackgammonConnection::request_user_list( void )
{
  send( "USERS\r\n" );
}

/////////////////////////////////////////////////////////////////////////////
/// Sendet die Chat-Nachricht \a msg an den Benutzer \a send_to.
/////////////////////////////////////////////////////////////////////////////
void NetBackgammonConnection::chat( QString send_to, QString msg )
{
  send( "CHAT:" + send_to + ";" + msg + "\r\n" );
}

/////////////////////////////////////////////////////////////////////////////
/// Erstellt ein neues Spiel auf dem Server. Wenn \a password nicht leer
/// ist wird dies als Passwort verwendet. Wenn \a timeout oder \a turns
/// größer als 0 ist, wird dies als Timeout bzw. als Rundenbegrenzung
/// verwendet.
/////////////////////////////////////////////////////////////////////////////
void NetBackgammonConnection::create_game( QString password,
                                           unsigned int timeout,
                                           unsigned int turns )
{
  QString msg = "JOIN:" + QString::number( m_max_game_number + 1 ) + ";"
                + password + ";";

  if( timeout > 0 )
    msg += QString::number( timeout );
  msg += ";";

  if( turns > 0 )
    msg += QString::number( turns );

  msg += "\r\n";
  send( msg );
}

/////////////////////////////////////////////////////////////////////////////
/// Tritt dem Spiel \a game_number auf dem Server bei. Falls dies ein
/// passwortgeschütztes Spiel ist, muss in \a password das Passwort übergeben
/// werden.
/////////////////////////////////////////////////////////////////////////////
void NetBackgammonConnection::join_game( unsigned int game_number,
                                         QString password )
{
  send( "JOIN:" + QString::number( game_number ) + ";" + password + "\r\n" );
}

/////////////////////////////////////////////////////////////////////////////
/// Verlässt ein Spiel.
/////////////////////////////////////////////////////////////////////////////
void NetBackgammonConnection::leave_game( void )
{
  m_joined_game = false;
  send( "JOIN:-1\r\n" );
}

/////////////////////////////////////////////////////////////////////////////
/// Teilt dem Backgammon-Server mit, dass das Backgammon-Spiel gestartet
/// werden kann.
/////////////////////////////////////////////////////////////////////////////
void NetBackgammonConnection::start( void )
{
  m_game_running = true;
  send( "START\r\n" );
}

/////////////////////////////////////////////////////////////////////////////
/// Informiert den Server über den Erhalt eines Befehls.
/////////////////////////////////////////////////////////////////////////////
void NetBackgammonConnection::confirm( void )
{
  send( "CONFIRM\r\n" );
}

/*< \label{void:NetBackgammonConnection::turn(const:BG::BackgammonTurn&)} >*/
/////////////////////////////////////////////////////////////////////////////
/// Überträgt den Zug \a turn an den Backgammon-Server.
/////////////////////////////////////////////////////////////////////////////
void NetBackgammonConnection::turn( const BG::BackgammonTurn &turn )
{
  const short int PLAYER_IND = ( turn.get_player() == BG::WHITE ) ? 1 : -1;
      // Um Berechnungen einfacher zu gestalten.
  short int start_position, end_position; // Start- und Endpositione eines
                                          // Zuges für Berechnungszwecke.
  QString msg = "TURN:";
  unsigned int i, j;

  for( i = 0; i < 8; i++ )
    {
      for( j = 0; j < turn.get_move_list().size(); j++ )
        {
          /* Mit folgendem Code-Block wird Sichergestellt, dass die Züge so
           * sortiert sind, dass sie vom Server akzeptiert werden (Züge von
           * der Bar, normale Züge, Züge bei denen Spielsteine ausgewürfelt
           * werden). */
          start_position = turn.get_move_list()[ j ].get_from();
          if( start_position == BG::BAR )
            {
              if( turn.get_player() == BG::WHITE )
                start_position = -1;
              else
                start_position = 24;
            }
          end_position = start_position
                         + turn.get_move_list()[ j ].get_distance()
                           * PLAYER_IND;
          if( end_position < 0 || end_position > 23 )
            end_position = BG::OUT_OF_GAME;
          if( ( i == 0 && turn.get_move_list()[ j ].get_from() != BG::BAR )
             || ( i == 1 && ( turn.get_move_list()[ j ].get_from() == BG::BAR
                              || end_position == BG::OUT_OF_GAME ) )
             || ( i >= 2 && ( end_position != BG::OUT_OF_GAME
                              || turn.get_move_list()[ j ].get_distance()
                                 != static_cast< signed int >( 8 - i ) ) ) )
            continue;

          /* Jetzt wird der Zug in die Nachricht, die an den Server gesendet
           * wird übersetzt. */
          msg += QString::number( turn.get_move_list()[ j ].get_distance() );
          msg += " ";
          if( turn.get_player() == BG::WHITE )
            {
              if( turn.get_move_list()[ j ].get_from() != BG::BAR )
                msg += QString::number( 24 - start_position );
              else
                msg += "25";
              msg += " ";
              if( end_position != BG::OUT_OF_GAME )
                msg += QString::number( 24 - end_position );
              else
                msg += "0";
            }
          else
            {
              if( turn.get_move_list()[ j ].get_from() != BG::BAR )
                msg += QString::number( 1 + start_position );
              else
                msg += "25";
              msg += " ";
              if( end_position != BG::OUT_OF_GAME )
                msg += QString::number( 1 + end_position );
              else
                msg += "0";
            }
          msg += ";";
        }
    }

  if( turn.get_move_list().size() <= 0 )
    msg += "NOTURN";

  msg += "\r\n";
  send( msg );
}

/////////////////////////////////////////////////////////////////////////////
/// Setzt \ref m_dbg auf \a val.
/////////////////////////////////////////////////////////////////////////////
void NetBackgammonConnection::set_dbg( bool val )
{
  m_dbg = val;
}

/////////////////////////////////////////////////////////////////////////////
/// Sendet \a msg an den Server. Wenn \ref NetBackgammonConnection::m_dbg
/// true ist, wird auf cerr eine Debug-Ausgabe geschrieben.
/////////////////////////////////////////////////////////////////////////////
void NetBackgammonConnection::send( QString msg )
{
  if( m_dbg )
    cerr << "< " << msg.toStdString() << std::flush;
  write( msg.toAscii() );
}

/////////////////////////////////////////////////////////////////////////////
/// Dieser Slot wird aufgerufen, wenn Daten zum Lesen am Socket bereit
/// stehen. Diese werden dann verarbeitet, so fern sie das aktuelle Spiel
/// betreffen und anschließend werden alle eingehende Daten mit dem Signal
/// received_msg() weitergeleitet.
/////////////////////////////////////////////////////////////////////////////
void NetBackgammonConnection::on_readyRead( void )
{
  QStringList sub_params; // Unter-Parameter eines Parameters einer
                          // Server-Nachricht.
  NetBackgammonMsg msg;
  unsigned int i;

  while( canReadLine() )
    {
      msg.set_by_raw_data( readLine() );

      if( m_dbg )
        cerr << "> " << msg.get_raw_data().toStdString() << endl;

      if( msg.get_type() == "CONFIRM" )
        {
          if( msg.get_in_reply_to() == "LOGIN" )
            {
              m_logged_in = true;
              emit logged_in();
            }
          else if( msg.get_in_reply_to() == "LOGOUT" )
            {
              m_logged_in = false;
              m_joined_game = false;
              m_game_running = false;
              emit logged_out();
            }
          else if( msg.get_in_reply_to() == "JOIN" )
            {
              if( msg.get_params().size() > 1
                  && msg.get_params()[ 1 ] == "-1" )
                {
                  m_joined_game = false;
                  m_game_running = false;
                  emit left_game();
                }
              else
                {
                  m_joined_game = true;
                  if( msg.get_params().size() > 1 )
                    m_game_number = msg.get_params()[ 1 ].toInt();
                  emit joined_game();
                }
            }
        }
      else if( msg.get_type() == "LIST" )
        {
          m_max_game_number = 0;
          for( i = 0;
              i < static_cast< unsigned int >( msg.get_params().size() );
              i++ )
            {
              sub_params = msg.get_params()[ i ].split( ' ' );
              if( static_cast< unsigned int >( sub_params[ 0 ].toInt() )
                  > m_max_game_number )
                m_max_game_number = sub_params[ 0 ].toInt();
            }
        }
      else if( msg.get_type() == "INFO" && msg.get_in_reply_to() == "JOIN" )
        {
          if( msg.get_params().size() > 1
              && msg.get_params()[ 1 ] == "-1 " + m_username )
            {
              m_joined_game = false;
              m_game_running = false;
              emit left_game();
            }
        }
      else if( msg.get_type() == "ENDGAME" )
        m_game_running = false;

      emit received_msg( msg );
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Setzt die Membervariablen der Klasse nach dem Trennen einer Verbindung
/// wieder zurück.
/////////////////////////////////////////////////////////////////////////////
void NetBackgammonConnection::on_disconnected( void )
{
  m_logged_in = false;
  m_joined_game = false;
  m_game_running = false;
}

//===========================================================================
// NetBackgammon
//===========================================================================
/////////////////////////////////////////////////////////////////////////////
/// Konstruktor. Mit \a connection muss ein Zeiger auf die Verbindung zum
/// Backgammon-Server übergeben werden. \a net_player ist der über das
/// Netzwerkgesteuerte Spieler, sofern eine Netzwerkverbindung aufgebaut
/// wird/wurde. Die restlichen Argumente entsprechen denen des Konstruktors
/// von \ref BG::Backgammon.
/////////////////////////////////////////////////////////////////////////////
NetBackgammon::NetBackgammon( NetBackgammonConnection *connection,
                              BG::Player net_player,
                              bool is_auto_dice_roll_enabled,
                              QObject *parent )
    : BG::Backgammon( is_auto_dice_roll_enabled, parent ),
      m_connection( connection ), m_net_player( net_player ),
      m_dice_received( false )
{
  connect( m_connection, SIGNAL( received_msg( NetBackgammonMsg ) ),
           this, SLOT( process_srv_msg( NetBackgammonMsg ) ) );
  connect( this, SIGNAL( next_player( short int ) ),
           this, SLOT( transmit_last_turn( void ) ) );
  connect( this, SIGNAL( game_ended( void ) ),
           this, SLOT( transmit_last_turn( void ) ) );
      // Auch der letzte Zug muss noch übermittelt werden.
}

/////////////////////////////////////////////////////////////////////////////
/// Startet das Backgammon-Spiel. Bei einem Netzwerkspiel wird dem Server
/// mitgeteilt, dass das Spiel beginnen kann.
/////////////////////////////////////////////////////////////////////////////
void NetBackgammon::start( void )
{
  if( m_connection->get_joined_game() )
    m_connection->start();
}

/////////////////////////////////////////////////////////////////////////////
/// Setzt \ref m_net_player auf \a player.
/////////////////////////////////////////////////////////////////////////////
void NetBackgammon::set_net_player( short int player )
{
  assert( player == BG::WHITE || player == BG::BLACK );
  m_net_player = BG::Player( player );
}

/*< \label{void:NetBackgammon::transmit_last_turn(void)} >*/
/////////////////////////////////////////////////////////////////////////////
/// Überträgt den gesamten letzten Zug an den Backgammon-Server, sofern
/// dieser lokal ausgeführt worden ist.
/////////////////////////////////////////////////////////////////////////////
void NetBackgammon::transmit_last_turn( void )
{
  if( m_connection->get_game_running() )
    {
      if( get_game_status() == BG::GAME_FINISHED )
        m_connection->turn( get_turn_list()[ get_turn_list().size() - 1 ] );
      else if( get_act_player() == m_net_player && m_dice_received )
        m_connection->turn( get_turn_list()[ get_turn_list().size() - 2 ] );
    }
  m_dice_received = false;
}

/*< \label{void:NetBackgammon::process_srv_msg(NetBackgammonMsg)} >*/
/////////////////////////////////////////////////////////////////////////////
/// Verarbeitet eine vom Backgammon-Server empfangene Nachricht, sofern
/// sie für diese Klasse von Bedeutung ist (BOARD, DICE, INFO:DICE,
/// INFO:TURN).
/////////////////////////////////////////////////////////////////////////////
void NetBackgammon::process_srv_msg( NetBackgammonMsg msg )
{
  const short int LOCAL_PLAYER = ( m_net_player == BG::WHITE ) ? BG::BLACK
                                                               : BG::WHITE;
      // Der lokal gesteuerte Spieler.
  QStringList sub_params; // Speichert die Subparameter eines Parameters.
  short int start_position, distance; // Startposition und Zugweite eines
                                      // empfangenen Zuges.
  unsigned int i;

  if( msg.get_type() == "JOIN" )
    {
      m_dice_received = false;
    }
  else if( msg.get_type() == "BOARD" )
    {
      if( msg.get_params().size() < 1 )
        return;
      sub_params = msg.get_params()[ 0 ].split( ' ' );
      if( sub_params.size() < 28 )
        return;
      lock_arrays();
      set_beared_off( LOCAL_PLAYER, sub_params[ 0 ].toInt() );
      for( i = 1; i < 25; i++ )
        {
          if( LOCAL_PLAYER == BG::WHITE )
            set_point( 24 - i, sub_params[ i ].toInt() );
          else
            set_point( i - 1, -1 * sub_params[ i ].toInt() );
        }
      set_on_bar( LOCAL_PLAYER, sub_params[ 25 ].toInt() );
      set_beared_off( m_net_player, -1 * sub_params[ 26 ].toInt() );
      set_on_bar( m_net_player, -1 * sub_params[ 27 ].toInt() );
      unlock_arrays();
      m_connection->confirm();
    }
  else if( msg.get_type() == "DICE" )
    {
      if( msg.get_params().size() < 1 )
        return;
      sub_params = msg.get_params()[ 0 ].split( ' ' );
      if( sub_params.size() >= 2 )
        {
          clear_dice_silent();
          m_dice_received = true;
          if( get_game_status() == BG::FIRST_ROUND
              && ( ( LOCAL_PLAYER == BG::WHITE
                     && sub_params[ 1 ].toInt() > sub_params[ 0 ].toInt() )
                   || ( LOCAL_PLAYER == BG::BLACK
                        && sub_params[ 1 ].toInt()
                           < sub_params[ 0 ].toInt() ) ) )
            set_dice( sub_params[ 1 ].toInt(), sub_params[ 0 ].toInt() );
          else
            set_dice( sub_params[ 0 ].toInt(), sub_params[ 1 ].toInt() );
        }
    }
  else if( msg.get_type() == "INFO" && msg.get_in_reply_to() == "DICE" )
    {
      if( msg.get_params().size() < 2 )
        return;
      sub_params = msg.get_params()[ 1 ].split( ' ' );
      if( sub_params.size() >= 2 )
        {
          clear_dice_silent();
          m_dice_received = true;
          if( get_game_status() == BG::FIRST_ROUND
              && ( ( LOCAL_PLAYER == BG::WHITE
                     && sub_params[ 0 ].toInt() > sub_params[ 1 ].toInt() )
                   || ( LOCAL_PLAYER == BG::BLACK
                        && sub_params[ 0 ].toInt()
                           < sub_params[ 1 ].toInt() ) ) )
            set_dice( sub_params[ 1 ].toInt(), sub_params[ 0 ].toInt() );
          else
            set_dice( sub_params[ 0 ].toInt(), sub_params[ 1 ].toInt() );
        }
    }
  else if( msg.get_type() == "INFO" && msg.get_in_reply_to() == "TURN" )
    {
      m_dice_received = false;
      for( i = 1; i < static_cast< unsigned int >( msg.get_params().size() );
           i++ )
        {
          sub_params = msg.get_params()[ i ].split( ' ' );
          if( sub_params.size() < 3
              || sub_params[ 0 ].toInt() + sub_params[ 1 ].toInt()
                 != sub_params[ 2 ].toInt() )
            continue;

          start_position = sub_params[ 1 ].toInt();
          distance = sub_params[ 0 ].toInt();

          if( start_position < 0 || ( start_position > 24
                                      && start_position != 27 ) )
            continue;

          if( start_position == 27 )
            start_position = BG::BAR;
          else if( m_net_player == BG::WHITE )
            start_position = start_position - 1;
          else
            start_position = 24 - start_position;

          add_move_to_turn_list( BG::BackgammonMove( start_position,
                                                     distance ) );
        }

      if( get_turn_list().back().get_move_list().size() > 0 )
        clear_dice_silent();

      refresh();
      end_turn();
    }
  else if( msg.get_type() == "ENDGAME" )
    {
      if( get_game_status() != BG::GAME_FINISHED )
        {
          set_game_status( BG::GAME_FINISHED );
          set_winner( BG::NOT_DEFINED );
          set_win_height( BG::NOWIN );

          if( msg.get_params().size() > 0 )
            {
              if( msg.get_params()[ 0 ] == "WIN" )
                set_winner( ( m_net_player == BG::WHITE ) ? BG::BLACK
                                                          : BG::WHITE );
              else
                set_winner( m_net_player );
            }
          if( msg.get_params().size() > 2 )
            {
              sub_params = msg.get_params()[ 2 ].split( ' ' );
              if( sub_params.size() > 1 )
                {
                  if( sub_params[ 0 ].toInt() > sub_params[ 1 ].toInt() )
                    set_win_height( sub_params[ 0 ].toInt() );
                  else if( sub_params[ 0 ].toInt()
                           < sub_params[ 1 ].toInt() )
                    set_win_height( sub_params[ 1 ].toInt() );
                  else
                    set_winner( BG::NOT_DEFINED );
                }
            }
          emit game_ended();
        }
    }
}
