/*!
 * \file chatwidget.cpp
 * \brief Implementation der \ref ChatWidget Klasse.
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

#include <QAbstractSocket>
#include <QObject>
#include <QWidget>

#include "netbackgammon.h"

#include "ui_chatwidget.h"
#include "chatwidget.h"

/////////////////////////////////////////////////////////////////////////////
/// Konstruktor, mit \a connection muss ein Zeiger auf die \ref NetBackgammon
/// Klasse übergeben werden, welche die Netzwerkverbindung bereitstellt.
/// Die Bedienelemente werden in \a parent dargestellt.
/////////////////////////////////////////////////////////////////////////////
ChatWidget::ChatWidget( NetBackgammonConnection *connection,
                        bool passive_mode, QWidget *parent,
                        Qt::WindowFlags f )
    : QWidget( parent, f ), m_connection( connection ),
      m_passive_mode( passive_mode )
{
  setupUi( this );

  connect( m_connection,
           SIGNAL( stateChanged( QAbstractSocket::SocketState ) ),
           this, SLOT( output_net_connection_state(
                           QAbstractSocket::SocketState ) ) );
  connect( m_connection, SIGNAL( error( QAbstractSocket::SocketError ) ),
           this, SLOT( output_net_error( QAbstractSocket::SocketError ) ) );
  connect( m_connection, SIGNAL( received_msg( NetBackgammonMsg ) ),
           this, SLOT( process_srv_msg( NetBackgammonMsg ) ) );
  connect( m_connection,
           SIGNAL( stateChanged( QAbstractSocket::SocketState ) ),
           this, SLOT( update( void ) ) );

  update();
}

/////////////////////////////////////////////////////////////////////////////
/// Deaktiviert bzw. Aktiviert die Bedienelemente entsprechend dem aktuellen
/// Verbindungsstatus.
/////////////////////////////////////////////////////////////////////////////
void ChatWidget::update( void )
{
  if( m_connection->state() == QAbstractSocket::ConnectedState )
    {
      send_to->setEnabled( true );
      text->setEnabled( true );
      button_send->setEnabled( true );
    }
  else
    {
      send_to->setEnabled( false );
      text->setEnabled( false );
      button_send->setEnabled( false );
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Gibt \a msg als ausgehende Chat-Nachricht aus.
/////////////////////////////////////////////////////////////////////////////
void ChatWidget::output_chat_msg( QString msg )
{
  chat_log->append( "<span style=\"color: #000;\">"
                    + m_connection->get_username() + ": " + msg
                    + "</span>" );
}

/////////////////////////////////////////////////////////////////////////////
/// Gibt den in \a state übergebenen Socket-Status im Chat-Fenster aus.
/// Zudem werden die Bedienelemente entsprechend aktiviert oder deaktiviert.
/////////////////////////////////////////////////////////////////////////////
void ChatWidget::output_net_connection_state(
    QAbstractSocket::SocketState state )
{
  QString output = "<span style=\"color: #444;\">";

  switch( state )
    {
      case QAbstractSocket::HostLookupState:
        output += tr( "Suche " ) + m_connection->peerName() + "...";
        break;
      case QAbstractSocket::ConnectingState:
        output += tr( "Verbinde..." );
        break;
      case QAbstractSocket::ConnectedState:
        output += tr( "Verbunden." );
        break;
      case QAbstractSocket::ClosingState:
        output += tr( "Verbindung wird getrennt..." );
        break;
      case QAbstractSocket::UnconnectedState:
        output += tr( "Verbindung getrennt." );
        break;
      default:
        break;
    }

  output += "</span>";
  chat_log->append( output );
}

/////////////////////////////////////////////////////////////////////////////
/// Gibt für \a error eine Fehlermeldung aus.
/////////////////////////////////////////////////////////////////////////////
void ChatWidget::output_net_error( QAbstractSocket::SocketError error )
{
  QString output = tr( "<span style=\"color: #f00;\">Fehler: " );

  switch( error )
    {
      case QAbstractSocket::ConnectionRefusedError:
        output += tr( "Der Server hat die Verbindung verweigert." );
        break;
      case QAbstractSocket::RemoteHostClosedError:
        output += tr( "Der Server hat die Verbindung getrennt." );
        break;
      case QAbstractSocket::HostNotFoundError:
        output += tr( "Server konnte nicht gefunden werden." );
        break;
      case QAbstractSocket::SocketAccessError:
        output += tr( "Unzureichende Berechtigungen zum Aufbau der "
                      "Verbindung." );
        break;
      case QAbstractSocket::SocketResourceError:
        output += tr( "Nicht genügend Resourcen verfügbar. Eventuell sind "
                      "bereits zu viele Netzwerkverbindungen geöffnet." );
        break;
      case QAbstractSocket::SocketTimeoutError:
        output += tr( "Der Netzwerk-Timeout wurde überschritten." );
        break;
      case QAbstractSocket::DatagramTooLargeError:
        output += tr( "Größe des Datagrams hat das Limit des Betriebssystems"
                      " überschritten." );
        break;
      case QAbstractSocket::NetworkError:
        output += tr( "Netzwerkfehler." );
        break;
      case QAbstractSocket::UnsupportedSocketOperationError:
        output += tr( "Es wurde versucht eine nicht unterstützte "
                      "Netzwerkoperation auszuführen." );
        break;
      default:
        output += tr ( "Es ist ein unbekannter Fehler aufgetreten." );
    }

  output += "</span>";
  chat_log->append( output );
}

/////////////////////////////////////////////////////////////////////////////
/// Verarbeitet eine Server-Nachricht und produziert gegebenenfalls eine
/// entsprechende Ausgabe im Chat-Log. Ist \ref m_passive_mode false, werden
/// die am Server angemeldeten Benutzer beim Herstellen einer Verbindung
/// aktiv abgefragt.
/////////////////////////////////////////////////////////////////////////////
void ChatWidget::process_srv_msg( NetBackgammonMsg msg )
{
  bool output_msg = false; // Nachricht ausgeben?
  QString output = ""; // Ausgabe
  QString color = "00f"; // Darstellungsfarbe
  QStringList sub_params; // Unterparameter eines Parameters der Nachricht.
  unsigned int i;

  if( msg.get_type() == "HELLO" || msg.get_type() == "WELCOME" )
    {
      if( msg.get_params().size() > 0 )
        output += msg.get_params()[ 0 ];
      output_msg = true;
    }
  else if( msg.get_type() == "USERS" )
    {
      send_to->clear();
      for( i = 0; i < static_cast< unsigned int >( msg.get_params().size() );
           i++ )
        {
          if( msg.get_params()[ i ] != m_connection->get_username() )
            send_to->addItem( msg.get_params()[ i ] );
        }
    }
  else if( msg.get_type() == "CHAT" && msg.get_params().size() > 1 )
    {
      color = "000";
      output += msg.get_params()[ 0 ] + ": " + msg.get_params()[ 1 ];
      output_msg = true;
    }
  else if( msg.get_type() == "CONFIRM" )
    {
      if( msg.get_in_reply_to() == "LOGIN" )
        {
          output += tr( "Eingeloggt." );
          output_msg = true;

          if( !m_passive_mode )
            m_connection->request_user_list();
        }
      else if( msg.get_in_reply_to() == "LOGOUT" )
        {
          output += tr( "Ausgeloggt." );
          output_msg = true;
        }

      if( output_msg && msg.get_params().size() > 1
          && !msg.get_params()[ 1 ].isEmpty() )
        output += " (" + msg.get_params()[ 1 ] + ")";
    }
  else if( msg.get_type() == "ERROR" )
    {
      color = "f00";
      if( msg.get_in_reply_to() == "LOGIN" )
        {
          output += tr( "Login fehlgeschlagen: " );
          if( msg.get_params().size() > 1 )
            output += msg.get_params()[ 1 ];
          m_connection->close();
          output_msg = true;
        }
      else
        {
          output += tr( "Fehler: " );
          if( msg.get_params().size() > 1 )
            output += msg.get_params()[ 1 ];
          if( msg.get_params().size() > 0 )
            output += " (Befehl: " + msg.get_params()[ 0 ] + ")";
          output_msg = true;
        }
    }
  else if( msg.get_type() == "INFO" )
    {
      if( msg.get_in_reply_to() == "LOGIN" && msg.get_params().size() > 1
          && msg.get_params()[ 1 ] != m_connection->get_username() )
        {
          output += msg.get_params()[ 1 ];
          output += tr( " hat sich eingeloggt." );
          output_msg = true;

          send_to->addItem( msg.get_params()[ 1 ] );
        }
      else if( msg.get_in_reply_to() == "LOGOUT"
               && msg.get_params().size() > 1 )
        {
          output += msg.get_params()[ 1 ];
          output += tr( " hat sich ausgeloggt." );
          output_msg = true;

          send_to->removeItem( send_to->findText( msg.get_params()[ 1 ] ) );
        }
      else if( msg.get_in_reply_to() == "JOIN"
               && msg.get_params().size() > 1 )
        {
          sub_params = msg.get_params()[ 1 ].split( ' ' );
          if( sub_params.size() > 1 )
            {
               if( sub_params[ 0 ] == "-1" )
                {
                  if( sub_params[ 1 ] == m_connection->get_username() )
                    output += tr( "Sie haben das Spiel verlassen." );
                  else
                    output += sub_params[ 1 ]
                              + tr( " hat sein Spiel verlassen." );
                }
              else if( m_connection->get_joined_game()
                       && static_cast< unsigned int >(
                              sub_params[ 0 ].toInt() )
                          == m_connection->get_game_number() )
                {
                  if( sub_params[ 1 ] == m_connection->get_username() )
                    output += tr( "Sie sind dem Spiel " ) + sub_params[ 0 ]
                              + tr( " beigetreten." );
                  else
                    output += sub_params[ 1 ] + tr( " ist Ihrem Spiel (" )
                              + sub_params[ 0 ] + tr( ") beigetreten." );
                }
              else
                output += sub_params[ 1 ] + tr( " ist dem Spiel " )
                          + sub_params[ 0 ] + tr( " beigetreten." );
              output_msg = true;
            }
        }
      else if( msg.get_in_reply_to() == "ENDGAME"
               && msg.get_params().size() > 1 )
        {
          sub_params = msg.get_params()[ 1 ].split( ' ' );
          if( sub_params.size() > 4 )
            {
              output += sub_params[ 1 ] + tr( " gewinnt Spiel " )
                        + sub_params[ 0 ]
                        + tr( " mit " ) + sub_params[ 3 ] + " "
                        + (( sub_params[ 3 ] == "1" )
                           ? tr( "Punkt" ) : tr( "Punkten" ))
                        + tr( ". (Verlierer: " ) + sub_params[ 4 ] + " "
                        + (( sub_params[ 4 ] == "1" )
                           ? tr( "Punkt" ) : tr( "Punkte" ))
                        + tr( "; Verbliebene Runden: " ) + sub_params[ 2 ]
                        + ")";
              output_msg = true;
            }
        }
    }

  output = "<span style=\"color: #" + color + ";\">" + output + "</span>";
  if( output_msg )
    chat_log->append( output );
}

/////////////////////////////////////////////////////////////////////////////
/// Gibt \a data ohne weitere Verarbeitung oder Formatierung aus.
/////////////////////////////////////////////////////////////////////////////
void ChatWidget::output_raw_data( QString data )
{
  chat_log->append( data );
}

/////////////////////////////////////////////////////////////////////////////
/// Sendet eine Chat-Nachricht.
/////////////////////////////////////////////////////////////////////////////
void ChatWidget::on_button_send_clicked( void )
{
  m_connection->chat( send_to->currentText(), text->text() );
  output_chat_msg( text->text() );
  emit chat( text->text() );
  text->clear();
}
