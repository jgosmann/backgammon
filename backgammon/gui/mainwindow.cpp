/*!
 * \file mainwindow.cpp
 * \brief Finale Implementation der MainWindow-Klasse beerbt, von der im
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

#include "config.h"

#include <cstdlib>
#include <cmath>
#include <ctime>

#include <QAbstractSocket>
#include <QColorDialog>
#include <QCoreApplication>
#include <QHostAddress>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QScrollBar>
#include <QSettings>
#include <QStandardItemModel>
#include <QStringList>
#include <QTimer>
#include <QVBoxLayout>
#include "flowlayout.h"

#include "ai.h"
#include "backgammon.h"
#include "backgammonwidget.h"
#include "netbackgammon.h"

#include "chatwidget.h"
#include "dicewidget.h"
#include "newgamedialog.h"
#include "ui_infodialog.h"
#include "ui_licensedialog.h"

#include "ui_mainwindow.h"
#include "mainwindow.h"

/////////////////////////////////////////////////////////////////////////////
/// Konstruktor. Für die Argumente siehe die Qt4-Dokumentation (QMainWindow).
/////////////////////////////////////////////////////////////////////////////
MainWindow::MainWindow( QWidget *parent, Qt::WFlags flags )
    : QMainWindow( parent, flags ), m_game( &m_net_connection, BG::WHITE,
                                            false ),
      m_dice_rolled( true ), m_round( 0 )
{
  QSettings settings;

  if( QCoreApplication::arguments().size() > 1 )
    {
      if( QCoreApplication::arguments()[ 1 ] == "--netdbg" )
        m_net_connection.set_dbg( true );
    }

  setupUi( this );

  m_ai[ 0 ] = m_ai[ 1 ] = NULL;
  m_wins[ 0 ] = m_wins[ 1 ] = 0;

  m_game_disp = new BackgammonWidget( &m_game, this );
  setCentralWidget( m_game_disp );

  FlowLayout *dice_layout = new FlowLayout( dock_dice_widget );

  m_d1 = new DiceWidget();
  m_d2 = new DiceWidget();
  m_d1->setMinimumSize( 75, 75 );
  m_d2->setMinimumSize( 75, 75 );
  dice_layout->addWidget( m_d1 );
  dice_layout->addWidget( m_d2 );
  dock_dice_widget->setLayout( dice_layout );

  action_autostart_rounds->setChecked(
      settings.value( "mainwindow/autostart_rounds", 0 ).toBool() );

  menu_Ansicht->addSeparator();

  m_turn_list_model = new QStandardItemModel( 1, 2, turn_list_view );
  m_turn_list_model->setHeaderData( 0, Qt::Horizontal, QVariant( "Weiß" ) );
  m_turn_list_model->setHeaderData( 1, Qt::Horizontal,
                                    QVariant( "Schwarz" ) );
  turn_list_view->setModel( m_turn_list_model );
  QAction *show_turn_list = dock_turn_list->toggleViewAction();
  show_turn_list->setIcon( QIcon( ":/toolbar/view_remove.png" ) );
  show_turn_list->setShortcut( QKeySequence( "Ctrl+L" ) );
  toolBar->addAction( show_turn_list );
  menu_Ansicht->addAction( show_turn_list );

  QAction *show_stats = dock_stats->toggleViewAction();
  show_stats->setIcon( QIcon( ":/toolbar/kpercentage.png" ) );
  show_stats->setShortcut( QKeySequence( "Ctrl+S" ) );
  toolBar->addAction( show_stats );
  menu_Ansicht->addAction( show_stats );

  QAction *show_chat = dock_chat->toggleViewAction();
  show_chat->setIcon( QIcon( ":/toolbar/edu_languages.png" ) );
  show_chat->setShortcut( QKeySequence( "Ctrl+C" ) );
  toolBar->addAction( show_chat );
  menu_Ansicht->addAction( show_chat );

  m_chat_widget = new ChatWidget( &m_net_connection, false, this );
  QVBoxLayout *dock_chat_layout = new QVBoxLayout;
  dock_chat_layout->addWidget( m_chat_widget );
  dock_chat_contents->setLayout( dock_chat_layout );

  m_player_disp = new QLabel( statusbar );
  statusbar->addPermanentWidget( m_player_disp );

  connect( action_rotate, SIGNAL( toggled( bool) ),
           m_game_disp, SLOT( set_is_rotated( bool ) ) );

  connect( action_new_game, SIGNAL( triggered( bool ) ),
           this, SLOT( init_new_game( void ) ) );
  connect( action_start_next_round, SIGNAL( triggered( bool ) ),
           this, SLOT( start_next_round( void ) ) );

  connect( m_d1, SIGNAL( clicked( void ) ),
           this, SLOT( roll_dice( void ) ) );
  connect( m_d2, SIGNAL( clicked( void ) ),
           this, SLOT( roll_dice( void ) ) );
  connect( action_roll_dice, SIGNAL( triggered( void ) ),
           this, SLOT( roll_dice( void ) ) );

  connect( m_game_disp, SIGNAL( illegal_move( int ) ),
           this, SLOT( show_illegal_move_in_statusbar( int ) ) );
  connect( &m_game, SIGNAL( next_player( short int ) ),
           this, SLOT( next_player ( void ) ) );
  connect( &m_game, SIGNAL( game_ended( void ) ),
           this, SLOT( show_winner( void ) ) );
  connect( &m_game, SIGNAL( turn_list_changed ( void ) ),
           this, SLOT( refresh_turn_list( void ) ) );
  connect( &m_game, SIGNAL( dice_changed( short int, short int ) ),
           this, SLOT( refresh_dice( void ) ) );

  connect( &m_net_connection, SIGNAL( received_msg( NetBackgammonMsg ) ),
           this, SLOT( net_msg_received( NetBackgammonMsg ) ) );

  srand( time( NULL ) );

  restoreGeometry( settings.value( "mainwindow/geometry" ).toByteArray() );
  restoreState( settings.value( "mainwindow/state" ).toByteArray() );

  dock_dice->show();
}

/////////////////////////////////////////////////////////////////////////////
/// Destruktor
/////////////////////////////////////////////////////////////////////////////
MainWindow::~MainWindow( void )
{
  QSettings settings;

  settings.setValue( "mainwindow/geometry", saveGeometry() );
  settings.setValue( "mainwindow/state", saveState() );

  settings.setValue( "mainwindow/autostart_rounds",
                     action_autostart_rounds->isChecked() );
}

/*< \label{void:MainWindow::init_new_game(void)} >*/
/////////////////////////////////////////////////////////////////////////////
/// Startet ein neues Backgammon-Spiel.
/////////////////////////////////////////////////////////////////////////////
void MainWindow::init_new_game( void )
{
  unsigned int i, j;
  NewGameDialog new_game_dlg( &m_net_connection,
                              m_chat_widget->chat_log->toHtml() );
  connect( new_game_dlg.chat_widget, SIGNAL( chat( QString ) ),
           m_chat_widget, SLOT( output_chat_msg( QString ) ) );

  if( m_game.get_game_status() != BG::GAME_FINISHED
      && m_net_connection.get_joined_game() )
    {
      if( QMessageBox::question( this, tr("Neues Spiel..."),
                                 tr("Sind Sie sicher, dass Sie das laufende "
                                    "Netzwerk-Spiel verlassen wollen?"),
                                 QMessageBox::Yes | QMessageBox::No,
                                 QMessageBox::No )
          != QMessageBox::Yes )
        return;
      m_net_connection.leave_game();
    }

  if( new_game_dlg.exec() != QDialog::Accepted )
    {
      if( m_net_connection.get_joined_game() )
        m_net_connection.leave_game();
      return;
    }

  m_player_type[ BG::WHITE ]
      = PlayerType( new_game_dlg.combobox_type_white->currentIndex() );
  m_player_type[ BG::BLACK ]
      = PlayerType( new_game_dlg.combobox_type_black->currentIndex() );
  for( i = 0; i < 2; i++ )
    {
      switch( m_player_type[ i ] )
        {
          case HUMAN_PLAYER:
            if( m_ai[ i] )
              {
                delete m_ai[ i ];
                m_ai[ i ] = NULL;
              }
            break;
          case AI_PLAYER:
            if( !m_ai[ i ] )
              m_ai[ i ] = new AI( &m_game );
            m_ai[ i ]->set_timeout(
                new_game_dlg.get_ai_timeout( BG::Player( i ) ) );
            for( j = 0; j < AIThread::NUM_RATING_FACTORS; j++ )
              m_ai[ i ]->set_rating_factor(
                  AIThread::RatingFactors( j ),
                  new_game_dlg.get_ai_factors( BG::Player( i ) )[ j ] );
            break;
          case NETWORK_PLAYER:
            if( m_ai[ i] )
              {
                delete m_ai[ i ];
                m_ai[ i ] = NULL;
              }
            m_game.set_net_player( i );
            break;
        }
    }

  m_auto_dice_roll[ BG::WHITE ]
      = new_game_dlg.checkbox_auto_dice_roll_white->isChecked();
  m_auto_dice_roll[ BG::BLACK ]
      = new_game_dlg.checkbox_auto_dice_roll_black->isChecked();
  m_show_blocking_msgs[ BG::WHITE ]
      = new_game_dlg.checkbox_blocking_msgs_white->isChecked();
  m_show_blocking_msgs[ BG::BLACK ]
      = new_game_dlg.checkbox_blocking_msgs_black->isChecked();

  m_round = 0;
  m_wins[ 0 ] = m_wins[ 1 ] = 0;

  actionUndo->setEnabled( !m_net_connection.get_joined_game() );

  start_next_round();
}

/*< \label{void:MainWindow::start_next_round(void)} >*/
/////////////////////////////////////////////////////////////////////////////
/// Startet die nächste Runde (ein neues Backgammon-Spiel mit gleichen
/// Einstellungen).
/////////////////////////////////////////////////////////////////////////////
void MainWindow::start_next_round( void )
{
  action_start_next_round->setEnabled( false );

  m_game.reset();

  m_d1->set_bg_color( m_game_disp->get_col_white() );
  m_d1->set_fg_color( m_game_disp->get_col_black() );
  m_d2->set_bg_color( m_game_disp->get_col_black() );
  m_d2->set_fg_color( m_game_disp->get_col_white() );

  m_d1->set_value( 0 );
  m_d2->set_value( 0 );

  m_dice_rolled = false;

  m_round++;
  update_stats();

  m_turn_list_model->removeRows( 0, m_turn_list_model->rowCount() );
  m_turn_list_model->insertRow( 0 );

  m_game_disp->setEnabled( false );

  statusbar->clearMessage();
  m_player_disp->setText( tr(" Zum Beginnen auf die Würfel klicken. ") );

  m_game.start();
}

/////////////////////////////////////////////////////////////////////////////
/// Aktualisiert die Statistiken.
/////////////////////////////////////////////////////////////////////////////
void MainWindow::update_stats( void )
{
  n_rounds->setText( tr("Runde: ") + QString::number( m_round ) );
  n_wins_white->setText( tr("Weiß: ") + QString::number( m_wins[ BG::WHITE ] ) );
  n_wins_black->setText( tr("Schwarz: ")
                         + QString::number( m_wins[ BG::BLACK ] ) );
}

/////////////////////////////////////////////////////////////////////////////
/// Ädert die Farbe des weißen Spielers. Dazu wird ein Farbauswahldialog
/// angezeigt.
/////////////////////////////////////////////////////////////////////////////
void MainWindow::on_action_color_white_triggered( void )
{
  QColor col = QColorDialog::getColor( m_game_disp->get_col_white(), this );
  if( col.isValid () )
    {
      m_game_disp->set_col_white( col );
      m_game_disp->update();
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Ädert die Farbe des schwarzen Spielers. Dazu wird ein Farbauswahldialog
/// angezeigt.
/////////////////////////////////////////////////////////////////////////////
void MainWindow::on_action_color_black_triggered( void )
{
  QColor col = QColorDialog::getColor( m_game_disp->get_col_black(), this );
  if( col.isValid () )
    {
      m_game_disp->set_col_black( col );
      m_game_disp->update();
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Ädert die Umrandungsfarbe. Dazu wird ein Farbauswahldialog angezeigt.
/////////////////////////////////////////////////////////////////////////////
void MainWindow::on_action_color_pen_triggered( void )
{
  QColor col = QColorDialog::getColor( m_game_disp->get_col_pen(), this );
  if( col.isValid () )
    {
      m_game_disp->set_col_pen( col );
      m_game_disp->update();
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Ädert die Farbe der Bar. Dazu wird ein Farbauswahldialog angezeigt.
/////////////////////////////////////////////////////////////////////////////
void MainWindow::on_action_color_bar_triggered( void )
{
  QColor col = QColorDialog::getColor( m_game_disp->get_col_bar(), this );
  if( col.isValid () )
    {
      m_game_disp->set_col_bar( col );
      m_game_disp->update();
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Ädert die Hintergrundsfarbe. Dazu wird ein Farbauswahldialog angezeigt.
/////////////////////////////////////////////////////////////////////////////
void MainWindow::on_action_color_bg_triggered( void )
{
  QColor col = QColorDialog::getColor( m_game_disp->get_col_bg(), this );
  if( col.isValid () )
    {
      m_game_disp->set_col_bg( col );
      m_game_disp->update();
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Zeigt das Info-Fenster mit Informationen über das Programm an.
/////////////////////////////////////////////////////////////////////////////
void MainWindow::on_action_show_info_triggered( void )
{
  QDialog info_dlg( this );
  Ui::InfoDialog dlg_content;

  dlg_content.setupUi( &info_dlg );

  info_dlg.setWindowTitle( info_dlg.windowTitle().replace( "PACKAGE",
                                                           PACKAGE ) );

  dlg_content.info_label->setText(
      dlg_content.info_label->text().replace( "PACKAGE_STRING",
                                              PACKAGE_STRING ) );
  dlg_content.info_label->setText(
      dlg_content.info_label->text().replace( "PACKAGE_BUGREPORT",
                                              PACKAGE_BUGREPORT ) );
  dlg_content.info_label->setText(
      dlg_content.info_label->text().replace( "HOMEPAGE",
                                              HOMEPAGE ) );

  info_dlg.exec ();
}

/////////////////////////////////////////////////////////////////////////////
/// Zeigt das Lizenz-Fenster mit der GNU General Public License an.
/////////////////////////////////////////////////////////////////////////////
void MainWindow::on_action_show_license_triggered( void )
{
  QDialog license_dlg( this );
  Ui::LicenseDialog dlg_content;

  dlg_content.setupUi( &license_dlg );
  license_dlg.exec();
}

/*< \label{void:MainWindow::next_player(void)} >*/
/////////////////////////////////////////////////////////////////////////////
/// Setzt alle Einstellungen für den Zug des nächsten Spielers.
/////////////////////////////////////////////////////////////////////////////
void MainWindow::next_player( void )
{
  statusbar->clearMessage();

  if( !m_dice_rolled || m_game.get_game_status() == BG::FIRST_ROUND )
    return;
  m_dice_rolled = false;

  m_game_disp->setEnabled( false );
  m_game_disp->update();

  if( m_auto_dice_roll[ m_game.get_act_player() ] )
    {
      QTimer::singleShot( 0, this, SLOT( roll_dice() ) );
        // Diese Funktion darf nicht sofort aufgerufen werdne, da es sonst zu
        // einem Deadlock kommen kann. Dies könnte passieren, wenn die KI
        // zugunfähig ist, da dann AIThread::m_best_move_mutex gesperrt ist,
        // da gerade gezogen wurde und Backgammon::next_player() gesendet
        // wurde. MainWindow::next_player() ruft MainWindow::roll_dice() auf,
        // welche bei Zugunfähigkeit wiederrum MainWindow::next_player()
        // aufruft. Daher würde MainWindow::roll_dice() ein zweites mal
        // aufgerufen und aktiviert unter Umständen die KI, wobei diese über
        // AIThread::m_best_move_mutex immer noch gesperrt ist.
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Zeigt den aktuellen Würfelwurf mit \ref m_d1 und \ref m_d2 an.
/////////////////////////////////////////////////////////////////////////////
void MainWindow::roll_dice( void )
{
  dock_dice->show();

  if( m_dice_rolled || ( m_net_connection.get_joined_game()
                         && !m_game.get_dice_received() ) )
    return;

  statusbar->clearMessage();

  /* Würfel werfen... */
  m_game.lock_arrays();
  if( !m_net_connection.get_joined_game() )
    {
      do
        {
          m_game.set_dice( rand() % 6 + 1, rand() % 6 + 1 );
        } while( m_game.get_game_status() == BG::FIRST_ROUND
                && m_game.get_dice()[ 0 ] == m_game.get_dice()[ 1 ] );
    }
  m_d1->set_value( m_game.get_dice()[ 0 ] );
  m_d2->set_value( m_game.get_dice()[ 1 ] );
  m_game.unlock_arrays();
  m_d1->update();
  m_d2->update();
  m_dice_rolled = true;

  if( m_game.get_game_status() != BG::FIRST_ROUND )
    {
      if( m_game.get_act_player() == BG::WHITE )
        {
          m_d1->set_fg_color( m_game_disp->get_col_black() );
          m_d1->set_bg_color( m_game_disp->get_col_white() );
          m_d2->set_fg_color( m_game_disp->get_col_black() );
          m_d2->set_bg_color( m_game_disp->get_col_white() );
        }
      else
        {
          m_d1->set_fg_color( m_game_disp->get_col_white() );
          m_d1->set_bg_color( m_game_disp->get_col_black() );
          m_d2->set_fg_color( m_game_disp->get_col_white() );
          m_d2->set_bg_color( m_game_disp->get_col_black() );
        }
    }

  /* Anzeigen welcher Spieler am Zug ist */
  if( m_game.get_act_player() == BG::WHITE )
    m_player_disp->setText( tr(" Weiß ist am Zug. ") );
  else
    m_player_disp->setText( tr(" Schwarz ist am Zug. ") );

  /* Gegebenenfalls Zugunfähigkeit anzeigen */
  if( !m_game.is_valid_move_possible() )
    {
      m_game.lock_arrays();
      if( m_game.get_act_player() == BG::WHITE )
        {
          if( m_show_blocking_msgs[ BG::WHITE] )
            QMessageBox::information( this, tr("Zugunfähig"),
                                      tr("Weiß ist zugunfähig.") );
          else
            statusbar->showMessage( tr("Weiß ist zugunfähig.") );
        }
      else
        {
          if( m_show_blocking_msgs[ BG::BLACK ] )
            QMessageBox::information( this, tr("Zugunfähig"),
                                      tr("Schwarz ist zugunfähig.") );
          else
            statusbar->showMessage( tr("Schwarz ist zugunfähig.") );
        }
      m_game.unlock_arrays();
    }

  /* Einstellungen gemäß des Spielertyps setzen */
  switch( m_player_type[ m_game.get_act_player() ] )
    {
      case HUMAN_PLAYER:
        if( m_game.is_valid_move_possible() )
          m_game_disp->setEnabled( true );
        else
          m_game.move();
        break;
      case AI_PLAYER:
        m_ai[ m_game.get_act_player() ]->move();
        m_player_disp->setText( m_player_disp->text()
                                + tr("(KI überlegt...) ") );
        break;
      case NETWORK_PLAYER:
        break;
   }
}

/*< \label{void:MainWindow::refresh_turn_list(void)} >*/
/////////////////////////////////////////////////////////////////////////////
/// Aktualisiert die Zugliste.
/////////////////////////////////////////////////////////////////////////////
void MainWindow::refresh_turn_list( void )
{
  std::vector< BG::BackgammonTurn > turn_list = m_game.get_turn_list();
  std::vector< BG::BackgammonMove > move_list; // Liste mit den in jeweils
                                               // einer Runde ausgeführten
                                               // Zügen.
  std::vector< bool > hit_checker; // Liste, die angibt, ob beim
                                   // entsprechenden Zug in move_list ein
                                   // Spielstein geschlagen wurde.
  std::vector< BG::BackgammonMove >::iterator move_list_iter[ 2 ];
  std::vector< bool >::iterator hit_checker_iter[ 2 ];
  short int end_position; // Endposition eines Zuges.
  std::list< QString > moves; // Züge als String konvertiert.
  std::list< QString >::iterator moves_iter[ 2 ];
  QString str; // Zum Zusammensetzen der Ausgabe.
  unsigned int i, j;

  m_game_disp->update();

  m_game.lock_arrays();
  if( m_game.get_dice()[ 0 ] == m_game.get_dice()[ 1 ]
      && m_game.get_turn_number() == 1 )
    {
      m_game.unlock_arrays();
      return;
    }
  m_game.unlock_arrays();

  /* Zeilen der Zugliste setzen */
  if( turn_list[ 0 ].get_player() == BG::WHITE )
    m_turn_list_model->setRowCount(
        static_cast< int > (ceil( turn_list.size() / 2.0 )) );
  else
    m_turn_list_model->setRowCount(
        static_cast< int > (floor( turn_list.size() / 2.0 )) + 1 );

  /* Letzte Runden durchgehen */
  for( i = ( turn_list.size() >= 2) ? turn_list.size() - 2 : 0;
       i < turn_list.size(); i++ )
    {
      if( turn_list[ i ].get_dice()[ 0 ] == 0
          || turn_list[ i ].get_dice()[ 1 ] == 0
          || ( i >= turn_list.size() - 1 && !m_dice_rolled
               && m_player_type[ turn_list[ i ].get_player() ]
                  != NETWORK_PLAYER )
          || ( i == turn_list.size() - 1
               && turn_list[ i ].get_move_list().size() <= 0 ) )
        {
          m_turn_list_model->setData(
            m_turn_list_model->index( ( turn_list[ 0 ].get_player()
                                        == BG::WHITE )
                                      ? static_cast< int > (floor( i / 2.0 ))
                                      : static_cast< int > (ceil( i / 2.0 )),
                                      turn_list[ i ].get_player() ),
            QVariant( "" ) );
          continue;
        }

      moves.clear();
      move_list = turn_list[ i ].get_move_list();
      hit_checker = turn_list[ i ].get_hit_checker();

      /* Züge in der Runde durchgehen */
      for( move_list_iter[ 0 ] = move_list.begin(),
           hit_checker_iter[ 0 ] = hit_checker.begin();
           move_list_iter[ 0 ] != move_list.end();
           move_list_iter[ 0 ]++, hit_checker_iter[ 0 ]++ )
        {
          str = conv_pos( move_list_iter[ 0 ]->get_from(),
                          turn_list[ i ].get_player() );
          str += "/";
          end_position = ( ( move_list_iter[ 0 ]->get_from() != BG::BAR )
                             ? move_list_iter[ 0 ]->get_from()
                             : ( ( turn_list[ i ].get_player() == BG::WHITE )
                                   ? -1 : 24 ) )
                         + move_list_iter[ 0 ]->get_distance()
                           * (( turn_list[ i ].get_player() == BG::WHITE )
                                ? 1 : -1);
          if( end_position < 0 || end_position > 23 )
            end_position = BG::OUT_OF_GAME;
          str += conv_pos( end_position, turn_list[ i ].get_player() );
          if( *hit_checker_iter[ 0 ] )
            str += "*";

          /* Kann dieser Zug mit anderen zusammengefasst werden? */
          for( move_list_iter[ 1 ] = move_list_iter[ 0 ],
               hit_checker_iter[ 1 ] = hit_checker_iter[ 0 ];
               move_list_iter[ 1 ] != move_list.end();
               move_list_iter[ 1 ]++, hit_checker_iter[ 1 ]++)
            {
              if( move_list_iter[ 1 ] == move_list_iter[ 0 ] )
                {
                  move_list_iter[ 1 ]++;
                  hit_checker_iter[ 1 ]++;
                  if( move_list_iter[ 1 ] == move_list.end() )
                    break;
                }
              if( move_list_iter[ 1 ]->get_from() != end_position )
                continue;

              str += "/";
              end_position = ( ( move_list_iter[ 1 ]->get_from() != BG::BAR )
                             ? move_list_iter[ 1 ]->get_from()
                             : ( ( turn_list[ i ].get_player() == BG::WHITE )
                                   ? -1 : 24 ) )
                         + move_list_iter[ 1 ]->get_distance()
                           * (( turn_list[ i ].get_player() == BG::WHITE )
                                ? 1 : -1);
              if( end_position < 0 || end_position > 23 )
                end_position = BG::OUT_OF_GAME;
              str += conv_pos( end_position, turn_list[ i ].get_player() );
              if( *hit_checker_iter[ 1 ] )
                str += "*";
              move_list_iter[ 1 ] = move_list.erase( move_list_iter[ 1 ] );
              move_list_iter[ 1 ]--;
              hit_checker_iter[ 1 ] = hit_checker.erase(
                                        hit_checker_iter[ 1 ] )--;
              hit_checker_iter[ 1 ]--;
            }
          moves.push_back( str );
        }

      /* Ausgabestring zusammensetzen */
      str = QString::number( turn_list[ i ].get_dice()[ 0 ] )
            + QString::number( turn_list[ i ].get_dice()[ 1 ] )
            + ": ";
      for( moves_iter[ 0 ] = moves.begin(); moves_iter[ 0 ] != moves.end();
           moves_iter[ 0 ]++ )
        {
          j = 1;

          /* Prüfen, ob der gleiche Zug mehrmals in einer Runde ausgeführt
           * wurde */
          for( moves_iter[ 1 ] = moves_iter[ 0 ];
               moves_iter[ 1 ] != moves.end();
               moves_iter[ 1 ]++ )
            {
              if( *moves_iter[ 0 ] != *moves_iter[ 1 ]
                  || moves_iter[ 0 ] == moves_iter[ 1 ] )
                continue;

              j++;
              moves_iter[ 1 ] = moves.erase( moves_iter[ 1 ] );
              moves_iter[ 1 ]--;
            }

          str += *moves_iter[ 0 ];
          if( j > 1 )
            str += "(" + QString::number( j ) + ")";
          str += " ";
        }

      m_turn_list_model->setData(
          m_turn_list_model->index( ( turn_list[ 0 ].get_player()
                                        == BG::WHITE )
                                      ? static_cast< int > (floor( i / 2.0 ))
                                      : static_cast< int > (ceil( i / 2.0 )),
                                    turn_list[ i ].get_player() ),
          QVariant( str ) );
    }

//  if( turn_list.back().get_move_list().size() <= 0 )
//    m_turn_list_model->setData(
//      m_turn_list_model->index( static_cast< int > (
//                                  ( turn_list.back().get_player() == BG::WHITE )
//                                  ? floor( turn_list.size() / 2.0 )
//                                  : ceil( turn_list.size() / 2.0) ),
//                                turn_list[ i ].get_player() ),
//      QVariant( "" ));
//
//  if( turn_list.back().get_player() == BG::WHITE )
//    m_turn_list_model->setData(
//      m_turn_list_model->index( static_cast< int >
//                                  ( ceil( turn_list.size() / 2.0 ) ),
//                                BG::BLACK ),
//      QVariant( "" ) );

  turn_list_view->resizeColumnsToContents();
  turn_list_view->resizeRowsToContents();
  turn_list_view->verticalScrollBar()->setSliderPosition(
      turn_list_view->verticalScrollBar()->maximum() );
  turn_list_view->update();
}

/////////////////////////////////////////////////////////////////////////////
/// Setzt bei einem ungültigen Zug (wobei der Grund, warum er ungültig ist
/// in \a reason übergeben wird) eine entsprechende Nachricht in die
/// Statuszeile.
/////////////////////////////////////////////////////////////////////////////
void MainWindow::show_illegal_move_in_statusbar( int reason )
{
  switch( reason )
    {
      case BG::WRONG_DIRECTION:
        statusbar->showMessage( tr("Falsche Zugrichtung. ") );
        break;
      case BG::CHECKER_LEFT_ON_BAR:
        statusbar->showMessage( tr("Es müssen noch Spielsteine zurück ins Spiel"
                                   " gebracht werden. ") );
        break;
      case BG::NOT_ALL_IN_HOMEBOARD:
        statusbar->showMessage( tr("Es müssen erst alle Spielsteine im "
                                   "Homeboard sein, bevor mit dem Auswürfeln "
                                   "begonnen werden kann.") );
        break;
      case BG::ANOTHER_CHECKER_HAS_TO_BE_MOVED:
        statusbar->showMessage( tr("Ein anderer Spielstein hat Vorrang.") );
        break;
      case BG::BLOCKED:
        statusbar->showMessage( tr("Zielfeld ist belegt.") );
        break;
      case BG::DICE_VALUE:
        statusbar->showMessage( tr("Ein anderer Spielstein muss bewegt werden "
                                   "oder das Feld ist mit dem Würfelergebnis "
                                   "nicht erreichbar.") );
        break;
      default:
        statusbar->showMessage( tr("Dieser Zug ist nicht möglich.") );
        break;
    }
}

/*< \label{void:MainWindow::show_winner(void)} >*/
/////////////////////////////////////////////////////////////////////////////
/// Zeigt den Gewinner an, wenn \a status BG::GAME_FINISHED ist.
/////////////////////////////////////////////////////////////////////////////
void MainWindow::show_winner( void )
{
  QString msg;

  statusbar->clearMessage();
  m_player_disp->setText( tr(" Spiel beendet. ") );
  m_game_disp->setEnabled( false );

  if( m_game.get_winner() == BG::NOT_DEFINED )
    {
      if( m_net_connection.get_joined_game()
          || ( m_player_type[ BG::WHITE ] != NETWORK_PLAYER
               && m_player_type[ BG::BLACK ] != NETWORK_PLAYER ) )
        {
          if( !action_autostart_rounds->isChecked() )
            QMessageBox::information( this, tr("Spiel beendet."),
                                      tr("Spiel wurde vorzeitig beendet.") );
        }
      return;
    }

  if( m_game.get_winner() == BG::WHITE )
    {
      m_wins[ BG::WHITE ] += m_game.get_win_height();
      msg = tr("Weiß");
    }
  else
    {
      m_wins[ BG::BLACK ] += m_game.get_win_height();
      msg = tr("Schwarz");
    }

  msg += tr(" gewinnt das Spiel mit ");
  switch( m_game.get_win_height() )
    {
      case BG::NOWIN:
        msg += tr("0 Punkten.");
        break;
      case BG::GAMMON:
        msg += tr("2 Punkten (\"Gammon\").");
        break;
      case BG::BACKGAMMON:
        msg += tr("3 Punkten (\"Backgammon\").");
        break;
      default:
        msg += tr("1 Punkt.");
        break;
    }

  m_game_disp->update();
  update_stats();
  if( !action_autostart_rounds->isChecked() )
    QMessageBox::information( this, tr("Spiel beendet."), msg );

  if( m_player_type[ BG::WHITE ] != NETWORK_PLAYER
      && m_player_type[ BG::BLACK ] != NETWORK_PLAYER )
    {
      if( !action_autostart_rounds->isChecked() )
        action_start_next_round->setEnabled( true );
      else
        {
          start_next_round();
          roll_dice();
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Wird aufgerufen, wenn sich das Würfelergebnis von \ref m_game ändert
/// und aktualisiert die Würfelanzeige, wenn gerade ein Spiel über das
/// Netzwerk gespielt wird und automatisches Würfeln aktiviert ist.
/////////////////////////////////////////////////////////////////////////////
void MainWindow::refresh_dice( void )
{
  if( m_net_connection.get_joined_game()
     && ( m_auto_dice_roll[ m_game.get_act_player() ]
          || m_player_type[ m_game.get_act_player() ] == NETWORK_PLAYER
          || m_game.get_game_status() == BG::FIRST_ROUND ) )
    roll_dice();
}

/////////////////////////////////////////////////////////////////////////////
/// Verarbeitet die Nachrichten des Backgammon-Server, sofern sie für das
/// Hauptfenster interessant sind.
/////////////////////////////////////////////////////////////////////////////
void MainWindow::net_msg_received( NetBackgammonMsg msg )
{
  QStringList sub_params; // Unterparameter eines Parameters.

  if( msg.get_type() == "INFO" && msg.get_in_reply_to() == "ENDGAME" )
    {
      if( msg.get_params().size() > 1 )
        {
          sub_params = msg.get_params()[ 1 ].split( ' ' );
          if( sub_params.size() > 2 && sub_params[ 2 ].toInt() > 0 )
            {
              if( !action_autostart_rounds->isChecked() )
                action_start_next_round->setEnabled( true );
              else
                start_next_round();
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Macht den letzten Zug rückgängig.
/////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionUndo_triggered( void )
{
  if( m_net_connection.get_joined_game()
      || ( !m_dice_rolled && m_game.get_turn_number() <= 1 ) )
    return;

  m_game.undo_last_turn();

  /* Würfelanzeige aktualisieren */
  m_game.lock_arrays();
  m_d1->set_value( m_game.get_dice()[ 0 ] );
  m_d2->set_value( m_game.get_dice()[ 1 ] );
  m_game.unlock_arrays();
  m_d1->update();
  m_d2->update();
  m_dice_rolled = true;

  if( m_game.get_game_status() != BG::FIRST_ROUND )
    {
      if( m_game.get_act_player() == BG::WHITE )
        {
          m_d1->set_fg_color( m_game_disp->get_col_black() );
          m_d1->set_bg_color( m_game_disp->get_col_white() );
          m_d2->set_fg_color( m_game_disp->get_col_black() );
          m_d2->set_bg_color( m_game_disp->get_col_white() );
        }
      else
        {
          m_d1->set_fg_color( m_game_disp->get_col_white() );
          m_d1->set_bg_color( m_game_disp->get_col_black() );
          m_d2->set_fg_color( m_game_disp->get_col_white() );
          m_d2->set_bg_color( m_game_disp->get_col_black() );
        }
    }
  else
    {
      if( ( m_d1->get_value() > m_d2->get_value()
            && m_game.get_act_player() == BG::WHITE )
          || ( m_d1->get_value() < m_d2->get_value()
               && m_game.get_act_player() == BG::BLACK ) )
        {
          m_d1->set_fg_color( m_game_disp->get_col_black() );
          m_d1->set_bg_color( m_game_disp->get_col_white() );
          m_d2->set_fg_color( m_game_disp->get_col_white() );
          m_d2->set_bg_color( m_game_disp->get_col_black() );
        }
      else
        {
          m_d1->set_fg_color( m_game_disp->get_col_white() );
          m_d1->set_bg_color( m_game_disp->get_col_black() );
          m_d2->set_fg_color( m_game_disp->get_col_black() );
          m_d2->set_bg_color( m_game_disp->get_col_white() );
        }
    }

  /* Anzeigen welcher Spieler am Zug ist */
  if( m_game.get_act_player() == BG::WHITE )
    m_player_disp->setText( tr(" Weiß ist am Zug. ") );
  else
    m_player_disp->setText( tr(" Schwarz ist am Zug. ") );

  /* Gegebenenfalls Zugunfähigkeit anzeigen */
  if( !m_game.is_valid_move_possible() )
    {
      m_game.lock_arrays();
      if( m_game.get_act_player() == BG::WHITE )
        {
          if( m_show_blocking_msgs[ BG::WHITE] )
            QMessageBox::information( this, tr("Zugunfähig"),
                                      tr("Weiß ist zugunfähig.") );
          else
            statusbar->showMessage( tr("Weiß ist zugunfähig.") );
        }
      else
        {
          if( m_show_blocking_msgs[ BG::BLACK ] )
            QMessageBox::information( this, tr("Zugunfähig"),
                                      tr("Schwarz ist zugunfähig.") );
          else
            statusbar->showMessage( tr("Schwarz ist zugunfähig.") );
        }
      m_game.unlock_arrays();
    }

  /* Einstellungen gemäß des Spielertyps setzen */
  if(    m_player_type[ m_game.get_act_player() ] == HUMAN_PLAYER
      && m_game.is_valid_move_possible() )
    m_game_disp->setEnabled( true );
  else if( m_player_type[ m_game.get_act_player() ] == AI_PLAYER )
    {
      m_ai[ m_game.get_act_player() ]->move();
      m_player_disp->setText( m_player_disp->text()
                              + tr("(KI überlegt...) ") );
    }
  else
    on_actionUndo_triggered();
}

/*< \label{QString:MainWindow::conv_pos(short:int,BG::Player)} >*/
/////////////////////////////////////////////////////////////////////////////
/// Konvertiert \a pos in die tatsächliche Position auf dem Spielbrett für
/// den in \a player übergebenen Spieler.
/////////////////////////////////////////////////////////////////////////////
QString MainWindow::conv_pos( short int pos, BG::Player player )
{
  if( pos == BG::OUT_OF_GAME )
    return "off";
  if( pos == BG::BAR )
    return "bar";
  if( player == BG::WHITE )
    return QString::number( 24 - pos );
  if( player == BG::BLACK )
    return QString::number( pos + 1 );
  return "";
}
