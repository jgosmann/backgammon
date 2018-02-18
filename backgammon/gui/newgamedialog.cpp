/*!
 * \file newgamedialog.cpp
 * \brief Enthält die Implementation der Klasse NewGameDialog, welche das
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

#include <cassert>

#include <QAbstractSocket>
#include <QComboBox>
#include <QDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QProcess>
#include <QScrollBar>
#include <QSettings>
#include <QStandardItemModel>
#include <QString>
#include <QStringList>
#include <QVBoxLayout>

#include "aithread.h"
#include "backgammon.h"
#include "netbackgammon.h"

#include "aisettingsdialog.h"
#include "ui_chatwidget.h"
#include "ui_newnetgamedialog.h"

#include "ai_std_config.h"

#include "ui_newgamedialog.h"
#include "newgamedialog.h"

/////////////////////////////////////////////////////////////////////////////
/// Konstruktor
/////////////////////////////////////////////////////////////////////////////
NewGameDialog::NewGameDialog( NetBackgammonConnection *connection,
                              QString chat_content, QWidget *parent,
                              Qt::WindowFlags f )
    : QDialog( parent, f ), m_connection( connection ),
      m_joined_game( false ), m_net_player( BG::WHITE )
{
  QSettings settings;
  QString tmp;
  unsigned int i, j;

  setupUi( this );
  chat_widget = new ChatWidget( m_connection, true, this );
  QVBoxLayout *chat_layout = new QVBoxLayout;
  chat_layout->addWidget( chat_widget );
  group_chat->setLayout( chat_layout );
  chat_widget->chat_log->insertHtml( chat_content );

  m_game_list_data = new QStandardItemModel( 1, 6, game_list );
  m_game_list_data->setHeaderData( 0, Qt::Horizontal, QVariant( "#" ) );
  m_game_list_data->setHeaderData( 1, Qt::Horizontal,
                                   QVariant( tr("Spieler 1") ) );
  m_game_list_data->setHeaderData( 2, Qt::Horizontal,
                                   QVariant( tr("Spieler 2") ) );
  m_game_list_data->setHeaderData( 3, Qt::Horizontal,
                                   QVariant( tr("Passwortschutz") ) );
  m_game_list_data->setHeaderData( 4, Qt::Horizontal,
                                   QVariant( tr("Timeout") ) );
  m_game_list_data->setHeaderData( 5, Qt::Horizontal, QVariant( tr("Runden") ) );
  game_list->setModel( m_game_list_data );
  game_list->resizeColumnsToContents();
  game_list->resizeRowsToContents();

  connect( combobox_type_white, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( update( void ) ) );
  connect( combobox_type_black, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( update( void ) ) );

  connect( button_refresh_game_list, SIGNAL( clicked( void ) ),
           m_connection, SLOT( request_game_list( void ) ) );

  connect( m_connection,
           SIGNAL( stateChanged( QAbstractSocket::SocketState ) ),
           this, SLOT( update( void ) ) );
  connect( m_connection, SIGNAL( connected( void ) ),
           this, SLOT( log_in( void ) ) );
  connect( m_connection, SIGNAL( received_msg( NetBackgammonMsg ) ),
           this, SLOT( process_srv_msg( NetBackgammonMsg ) ) );
  connect( m_connection, SIGNAL( joined_game( void ) ),
           this, SLOT( update( void ) ) );
  connect( m_connection, SIGNAL( left_game( void ) ),
           this, SLOT( update( void ) ) );
  connect( m_connection, SIGNAL( logged_in( void ) ),
           m_connection, SLOT( request_game_list( void ) ) );

  restoreGeometry( settings.value( "NewGameDialog/geometry" )
                           .toByteArray() );
  if( settings.value( "NewGameDialog/type_white", 0 ).toInt()
      < combobox_type_white->count() )
    combobox_type_white->setCurrentIndex(
        settings.value( "NewGameDialog/type_white", 0 ).toInt() );
  if( settings.value( "NewGameDialog/type_black", 0 ).toInt()
      < combobox_type_black->count() )
    combobox_type_black->setCurrentIndex(
        settings.value( "NewGameDialog/type_black", 0 ).toInt() );
  checkbox_auto_dice_roll_white->setChecked(
      settings.value( "NewGameDialog/auto_dice_roll_white", false )
              .toBool() );
  checkbox_auto_dice_roll_black->setChecked(
      settings.value( "NewGameDialog/auto_dice_roll_black", false )
              .toBool() );
  checkbox_blocking_msgs_white->setChecked(
      settings.value( "NewGameDialog/blocking_msgs_white", true ).toBool() );
  checkbox_blocking_msgs_black->setChecked(
      settings.value( "NewGameDialog/blocking_msgs_black", true ).toBool() );

  for( i = 0; i < 2; i++ )
    {
      m_ai_timeout[ i ] = settings.value( "NewGameDialog/m_ai_timeout["
                                          + QString::number( i ) + "]",
                                          AI_STD_TIMEOUT).toInt();
      for( j = 0; j < AIThread::NUM_RATING_FACTORS; j++ )
        m_ai_factors[ i ][ j ]
            = settings.value( "NewGameDialog/m_ai_factors["
                              + QString::number( i ) + "]["
                              + QString::number( j ) + "]",
                              AI_STD_RATING_FACTORS[ j ] ).toDouble();
    }

  for( i = 0; i < 10; i++ )
    {
      tmp = "NewGameDialog/server[" + QString::number( i ) + "]";
      if( !settings.value( tmp, "" ).toString().isEmpty() )
        {
          if( server->findText( settings.value( tmp ).toString() ) != -1 )
            server->removeItem( server->findText( settings.value( tmp )
                                                          .toString() ) );
          server->addItem( settings.value( tmp ).toString() );
        }
    }

  for( i = 0; i < 10; i++ )
    {
      tmp = "NewGameDialog/username[" + QString::number( i ) + "]";
      if( !settings.value( tmp, "" ).toString().isEmpty() )
        {
          if( username->findText( settings.value( tmp ).toString() ) != -1 )
            username->removeItem( username->findText(
                                      settings.value( tmp ).toString() ) );
          username->addItem( settings.value( tmp ).toString() );
        }
    }

  if( m_connection->state() == QAbstractSocket::ConnectedState
      && m_connection->get_logged_in() )
    m_connection->request_game_list();
  update();
}

/////////////////////////////////////////////////////////////////////////////
/// Destruktor
/////////////////////////////////////////////////////////////////////////////
NewGameDialog::~NewGameDialog( void )
{
  QSettings settings;
  unsigned int i, j;

  disconnect( m_connection, SIGNAL( logged_in( void ) ),
              m_connection, SLOT( request_game_list( void ) ) );

  settings.setValue( "NewGameDialog/geometry", saveGeometry() );
  settings.setValue( "NewGameDialog/type_white",
                     combobox_type_white->currentIndex() );
  settings.setValue( "NewGameDialog/type_black",
                     combobox_type_black->currentIndex() );
  settings.setValue( "NewGameDialog/auto_dice_roll_white",
                     checkbox_auto_dice_roll_white->isChecked() );
  settings.setValue( "NewGameDialog/auto_dice_roll_black",
                     checkbox_auto_dice_roll_black->isChecked() );
  settings.setValue( "NewGameDialog/blocking_msgs_white",
                     checkbox_blocking_msgs_white->isChecked() );
  settings.setValue( "NewGameDialog/blocking_msgs_black",
                     checkbox_blocking_msgs_black->isChecked() );

  for( i = 0; i < 2; i++ )
    {
      settings.setValue( "NewGameDialog/m_ai_timeout[" + QString::number( i )
                         + "]", m_ai_timeout[ i ] );
      for( j = 0; j < AIThread::NUM_RATING_FACTORS; j++ )
        settings.setValue( "NewGameDialog/m_ai_factors["
                           + QString::number( i ) + "]["
                           + QString::number( j ) + "]",
                           m_ai_factors[ i ][ j ] );
    }

  for( i = 0; i < 10; i++ )
    {
      settings.setValue( "NewGameDialog/server[" + QString::number( i )
                         + "]",
                         server->itemText( i ) );
      settings.setValue( "NewGameDialog/username[" + QString::number( i )
                         + "]",
                         username->itemText( i ) );
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Aktualisiert die GUI-Elemente des Dialogs gemäß den aktuellen
/// Einstellungen.
/////////////////////////////////////////////////////////////////////////////
void NewGameDialog::update( void )
{
  /* Tab Spieler */
  switch( combobox_type_white->currentIndex() )
    {
      case 0:
        button_configure_white->setEnabled( false );
        break;
      case 1:
        button_configure_white->setEnabled( true );
        break;
    }
  switch( combobox_type_black->currentIndex() )
    {
      case 0:
        button_configure_black->setEnabled( false );
        break;
      case 1:
        button_configure_black->setEnabled( true );
        break;
    }

  /* Tab Netzwerk */
  if( m_connection->state() == QAbstractSocket::ConnectedState )
    {
      button_connect->setText( tr("&Trennen") );
      server->setEnabled( false );
      username->setEnabled( false );
      password->setEnabled( false );
      button_connect->setEnabled( true );
      group_game_list->setEnabled( true );

      if( m_connection->get_joined_game() )
        {
          button_refresh_game_list->setEnabled( false );
          button_create_game->setEnabled( false );
          button_join_game->setText( tr("Spiel &verlassen") );
          button_join_game->setIcon( QIcon( ":/toolbar/remove.png" ) );

          if( !m_joined_game )
            {
              combobox_type_white->addItem(
                  QIcon( ":/toolbar/connect_creating.png" ),
                         "Netzwerkspieler" );
              combobox_type_black->addItem(
                  QIcon( ":/toolbar/connect_creating.png" ),
                         "Netzwerkspieler" );
              m_net_player = BG::BLACK;
              m_connection->request_game_list();
            }
          m_joined_game = true;

          if( ( combobox_type_white->currentText() != tr("Netzwerkspieler")
                && combobox_type_black->currentText() != tr("Netzwerkspieler") )
              || ( combobox_type_white->currentText() == tr("Netzwerkspieler")
                   && combobox_type_black->currentText()
                       == tr("Netzwerkspieler") ) )
            {
              if( m_net_player == BG::WHITE )
                {
                  if( combobox_type_white->currentText()
                      == tr("Netzwerkspieler") )
                    combobox_type_white->setCurrentIndex( 0 );
                  combobox_type_black->setCurrentIndex(
                      combobox_type_black->findText( tr("Netzwerkspieler") ) );
                  checkbox_auto_dice_roll_white->setEnabled( true );
                  checkbox_auto_dice_roll_black->setEnabled( false );
                  m_net_player = BG::BLACK;
                }
              else
                {
                  combobox_type_white->setCurrentIndex(
                      combobox_type_white->findText( tr("Netzwerkspieler") ) );
                  if( combobox_type_black->currentText()
                      == tr("Netzwerkspieler") )
                    combobox_type_black->setCurrentIndex( 0 );
                  checkbox_auto_dice_roll_white->setEnabled( false );
                  checkbox_auto_dice_roll_black->setEnabled( true );
                  m_net_player = BG::WHITE;
                }
            }
        }
      else
        {
          button_refresh_game_list->setEnabled( true );
          button_create_game->setEnabled( true );
          button_join_game->setText( tr("Spiel &beitreten") );
          button_join_game->setIcon( QIcon( ":/toolbar/add.png" ) );

          if( m_joined_game )
            {
              if( combobox_type_white->currentText() == tr("Netzwerkspieler") )
                combobox_type_white->setCurrentIndex( 0 );
              if( combobox_type_black->currentText() == tr("Netzwerkspieler") )
                combobox_type_black->setCurrentIndex( 0 );

              combobox_type_white->removeItem(
                  combobox_type_white->findText( tr("Netzwerkspieler") ) );
              combobox_type_black->removeItem(
                  combobox_type_black->findText( tr("Netzwerkspieler") ) );

              checkbox_auto_dice_roll_white->setEnabled( true );
              checkbox_auto_dice_roll_black->setEnabled( true );

              m_connection->request_game_list();
            }
          m_joined_game = false;
        }
    }
  else if ( m_connection->state() == QAbstractSocket::UnconnectedState )
    {
      button_connect->setText( tr("&Verbinden") );
      server->setEnabled( true );
      username->setEnabled( true );
      password->setEnabled( true );
      button_connect->setEnabled( true );
      group_game_list->setEnabled( false );

      button_refresh_game_list->setEnabled( true );
      button_create_game->setEnabled( true );
      button_join_game->setText( tr("Spiel &beitreten") );
      button_join_game->setIcon( QIcon( ":/toolbar/add.png" ) );

      if( m_joined_game )
        {
          if( combobox_type_white->currentText() == tr("Netzwerkspieler") )
            combobox_type_white->setCurrentIndex( 0 );
          if( combobox_type_black->currentText() == tr("Netzwerkspieler") )
            combobox_type_black->setCurrentIndex( 0 );

          combobox_type_white->removeItem(
              combobox_type_white->findText( tr("Netzwerkspieler") ) );
          combobox_type_black->removeItem(
              combobox_type_black->findText( tr("Netzwerkspieler") ) );

          checkbox_auto_dice_roll_white->setEnabled( true );
          checkbox_auto_dice_roll_black->setEnabled( true );
        }
      m_joined_game = false;
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Zeigt das Einstellungsfenster für einen Spieler vom Typ \a type an.
/// \a player gibt die Farbe des Spielers an.
/////////////////////////////////////////////////////////////////////////////
void NewGameDialog::configure_player( int player, int type )
{
  assert( player == BG::WHITE || player == BG::BLACK );
  switch( type )
    {
      case 1:
        AISettingsDialog dlg( this );

        dlg.timeout->setValue( m_ai_timeout[ player ] );
        dlg.prob_checker_is_hit->setValue(
            m_ai_factors[ player ][ AIThread::PROB_CHECKER_IS_HIT ] );
        dlg.prob_cannot_move->setValue(
            m_ai_factors[ player ][ AIThread::PROB_CANNOT_MOVE ] );
        dlg.prob_op_cannot_move->setValue(
            m_ai_factors[ player ][ AIThread::PROB_OP_CANNOT_MOVE ] );
        dlg.prob_op_cannot_move_bar->setValue(
            m_ai_factors[ player ][ AIThread::PROB_OP_CANNOT_MOVE_BAR ] );
        dlg.n_points_with_checkers->setValue(
            m_ai_factors[ player ]
                        [ AIThread::N_POINTS_WITH_CHECKERS ] );
        dlg.beared_off->setValue(
            m_ai_factors[ player ][ AIThread::BEARED_OFF ] );
        dlg.distance_from_homeboard->setValue(
            m_ai_factors[ player ][ AIThread::DISTANCE_FROM_HOMEBOARD ] );
        dlg.checkers_in_op_homeboard->setValue(
            m_ai_factors[ player ][ AIThread::CHECKERS_IN_OP_HOMEBOARD ] );
        dlg.op_distance_from_off_game->setValue(
            m_ai_factors[ player ][ AIThread::OP_DISTANCE_FROM_OFF_GAME ] );
        if( dlg.exec() == QDialog::Accepted )
          {
            m_ai_timeout[ player ] = dlg.timeout->value();
            m_ai_factors[ player ][ AIThread::PROB_CHECKER_IS_HIT ]
                = dlg.prob_checker_is_hit->value();
            m_ai_factors[ player ][ AIThread::PROB_CANNOT_MOVE ]
                = dlg.prob_cannot_move->value();
            m_ai_factors[ player ][ AIThread::PROB_OP_CANNOT_MOVE ]
                = dlg.prob_op_cannot_move->value();
            m_ai_factors[ player ][ AIThread::PROB_OP_CANNOT_MOVE_BAR ]
                = dlg.prob_op_cannot_move_bar->value();
            m_ai_factors[ player ][ AIThread::N_POINTS_WITH_CHECKERS ]
                = dlg.n_points_with_checkers->value();
            m_ai_factors[ player ][ AIThread::BEARED_OFF ]
                = dlg.beared_off->value();
            m_ai_factors[ player ][ AIThread::DISTANCE_FROM_HOMEBOARD ]
                = dlg.distance_from_homeboard->value();
            m_ai_factors[ player ][ AIThread::CHECKERS_IN_OP_HOMEBOARD ]
                = dlg.checkers_in_op_homeboard->value();
            m_ai_factors[ player ][ AIThread::OP_DISTANCE_FROM_OFF_GAME ]
                = dlg.op_distance_from_off_game->value();
          }
        break;
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Netwerkverbindung herstellen
/////////////////////////////////////////////////////////////////////////////
void NewGameDialog::on_button_connect_clicked( void )
{
  const QString srv_string = server->currentText();
  const QString user_string = username->currentText();
  QStringList srv_data;
  QString srv_domain;
  quint16 srv_port;

  button_connect->setEnabled( false );

  if( m_connection->state() == QAbstractSocket::UnconnectedState )
    {
      while( server->findText( srv_string ) != -1 )
        server->removeItem( server->findText( srv_string ) );
      server->insertItem( 0, srv_string );
      server->setCurrentIndex( 0 );

      while( username->findText( user_string ) != -1 )
        username->removeItem( username->findText( user_string ) );
      username->insertItem( 0, user_string );
      username->setCurrentIndex( 0 );

      srv_data = srv_string.split( ":" );
      srv_domain = srv_data[ 0 ];
      srv_port = NetBackgammonConnection::STD_PORT;

      if( srv_data.size() > 1 && !srv_data[ 1 ].isEmpty() )
        srv_port = srv_data[ 1 ].toInt();

      m_connection->connectToHost( srv_domain, srv_port );
    }
  else if( m_connection->state() == QAbstractSocket::ConnectedState )
    {
      if( m_connection->get_logged_in() )
        m_connection->log_out();
      m_connection->close();
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Erstellt ein neues Spiel auf dem Backgammon-Server.
/////////////////////////////////////////////////////////////////////////////
void NewGameDialog::on_button_create_game_clicked( void )
{
  Ui::NewNetGameDialog dlg_contents;
  QDialog new_net_game_dlg( this );

  dlg_contents.setupUi( &new_net_game_dlg );

  do
    {
      if( dlg_contents.password->text() != dlg_contents.password2->text() )
        QMessageBox::information( &new_net_game_dlg,
                                  QString( tr("Passwortschutz") ),
                                  QString( tr("Die eingegebenen Passwörter "
                                              "stimmen nicht überein.") ) );
      if( new_net_game_dlg.exec() != QDialog::Accepted )
        return;
    }
  while( dlg_contents.password->text() != dlg_contents.password2->text()
         && dlg_contents.cb_use_pass->isChecked() );

  m_connection->create_game( ( dlg_contents.cb_use_pass->isChecked() )
                               ? dlg_contents.password->text() : "", 0,
                             dlg_contents.max_turns->value() );
}

/////////////////////////////////////////////////////////////////////////////
/// Tritt einem Spiel auf dem Server bei bzw. verlässt es.
/////////////////////////////////////////////////////////////////////////////
void NewGameDialog::on_button_join_game_clicked( void )
{
  QString password = ""; // Passwort, dass zum Beitreten benötigt wird.

  if( !m_connection->get_joined_game() )
    {
      if( game_list->selectionModel()->selectedRows( 0 ).size() == 1 )
        {
          if( m_game_list_data->itemFromIndex(
                  game_list->selectionModel()->selectedRows( 3 )[ 0 ] )
              ->text() == tr("Ja") )
            {
              password = QInputDialog::getText( this, tr("Passwort"),
                                                tr("Passwort:"),
                                                QLineEdit::Password );
              if( password.isEmpty() )
                return;
            }
          m_connection->join_game(
              m_game_list_data->itemFromIndex(
                   game_list->selectionModel()->selectedRows( 0 )[ 0 ] )
              ->text().toInt(),
              password );
        }
    }
  else
    m_connection->leave_game();
}

/////////////////////////////////////////////////////////////////////////////
/// Startet einen Backgammon-Server.
/////////////////////////////////////////////////////////////////////////////
void NewGameDialog::on_button_start_server_clicked( void )
{
    if( QProcess::startDetached( "bgsrvstart" ) )
        ;
    else if( QProcess::startDetached( "./bgsrvstart" ) )
        ;
#ifdef Q_WS_MAC
    else if( QProcess::startDetached( "open bgsrvstart.app" ) )
        ;
#endif
    else
        QMessageBox::critical( this, tr("Fehler"),
                               tr("Server-Assistent konnte nicht gestart "
                                  "werden.") );
}

/////////////////////////////////////////////////////////////////////////////
/// Loggt den Benutzer auf dem Server ein.
/////////////////////////////////////////////////////////////////////////////
void NewGameDialog::log_in( void )
{
  m_connection->log_in( username->currentText(), password->text() );
}

/////////////////////////////////////////////////////////////////////////////
/// Verarbeitet die Server-Nachricht \a msg.
/////////////////////////////////////////////////////////////////////////////
void NewGameDialog::process_srv_msg( NetBackgammonMsg msg )
{
  QStringList sub_params; // Unter-Parameter eines Nachrichten-Parameters
  unsigned int i, j;

  if( msg.get_type() == "LIST" )
    {
      m_game_list_data->removeRows( 0, m_game_list_data->rowCount() );
      m_game_list_data->setRowCount( msg.get_params().size() );
      for( i = 0; i < static_cast< unsigned int >( msg.get_params().size() );
           i++ )
        {
          sub_params = msg.get_params()[ i ].split( ' ' );
          if( sub_params.size() < 6 )
            continue;
          for( j = 0; j < 3; j++ )
            m_game_list_data->setData( m_game_list_data->index( i, j ),
                                       QVariant( sub_params[ j ] ) );
          m_game_list_data->setData( m_game_list_data->index( i, 3 ),
                                     QVariant( ( sub_params[ 3 ] == "p" )
                                                 ? tr("Ja") : tr("Nein") ) );
          for( j = 4; j < 6; j++ )
            m_game_list_data->setData( m_game_list_data->index( i, j ),
                                       QVariant( sub_params[ j ] ) );
        }
      game_list->resizeColumnsToContents();
      game_list->resizeRowsToContents();
    }
  else if( msg.get_type() == "INFO" && msg.get_params().size() >= 2
           && msg.get_params()[ 0 ] == "JOIN" )
    {
      sub_params = msg.get_params()[ 1 ].split( ' ' );
      if( sub_params[ 0 ].toInt() == -1 )
        {
          for( int i = 0; i < m_game_list_data->rowCount(); i++ )
            {
              for( int j = 1; j <= 2; j++ )
                {
                  if( m_game_list_data->data( m_game_list_data
                                              ->index( i, j ) )
                      == sub_params[ 1 ] )
                    m_game_list_data->setData( m_game_list_data
                                               ->index( i, j ),
                                               QVariant( "NOONE" ) );
                }
              if(    m_game_list_data->data(
                       m_game_list_data->index( i, 1 ) ) == "NOONE"
                  && m_game_list_data->data(
                       m_game_list_data->index( i, 2 ) ) == "NOONE" )
                m_game_list_data->removeRow( i );
            }
        }
      else
        {
          for( int i = 0; i <= m_game_list_data->rowCount(); i++ )
            {
              if( i == m_game_list_data->rowCount() )
                {
                  m_connection->request_game_list();
                  return;
                }

              if( m_game_list_data->data( m_game_list_data->index( i, 0 ) )
                  == sub_params[ 0 ].toInt() )
                {
                  for( int j = 1; j <= 2; j++ )
                    {
                      if( m_game_list_data->data( m_game_list_data
                                                  ->index( i, j ) )
                          == "NOONE" )
                        m_game_list_data->setData(
                          m_game_list_data->index( i, j ),
                          QVariant( sub_params[ 1 ] ) );
                    }
                }
            }
        }
    }
}
