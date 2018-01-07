/*!
 * \file aithread.cpp
 * \brief Implementation der Klasse für die KI.
 * \date Sa Dec 23 2006
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

#include <cassert>

#include <vector>

#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QThread>

#include "backgammon.h"

#include "aithread.h"

/////////////////////////////////////////////////////////////////////////////
/// Konstruktor, wobei mit \a game ein Zeiger auf das Backgammon-Spiel
/// übergeben werden muss, für das die KI Züge sucht.
/////////////////////////////////////////////////////////////////////////////
AIThread::AIThread( BG::Backgammon *game, QObject *parent )
    : QThread( parent ), m_game( game ), m_status( TERMINATED ),
      m_request( SLEEP )
{
  unsigned int i;

  for( i = 0; i < 5; i++ )
    m_rating_factors[ i ] = 1;

  m_game_copy.set_is_auto_dice_roll_enabled( true );
}

/////////////////////////////////////////////////////////////////////////////
/// Startet die Ausführung der KI. Anschließend wird sie über spezielle
/// Steuerungsfunktionen kontrolliert.
/////////////////////////////////////////////////////////////////////////////
void AIThread::run( void )
{
  while( true )
    {
      switch( m_request )
        {
          case SLEEP:
            set_status( SLEEPING );
            msleep( 100 );
            break;
          case FIND_MOVE:
            set_status( SEARCHING_MOVE );

            m_best_move_mutex.lock();
            m_best_move.clear();
            m_best_move_mutex.unlock();
            if( m_game->is_valid_move_possible() )
              find_move();
            else
              set_moves_set( true );

            m_request_mutex.lock();
            if( m_request == FIND_MOVE )
              {
                m_request = SLEEP;
                m_request_mutex.unlock();
                emit best_move_found();
              }
            else
              m_request_mutex.unlock();
            break;
          case TERMINATE:
            set_status( TERMINATING );
            set_status( TERMINATED );
            return;
            break;
        }
    }
}

/*< \label{void:AIThread::find_move(short:int)} >*/
/////////////////////////////////////////////////////////////////////////////
/// Hauptfunktion der KI, die den besten Zug sucht. \a rec_depth muss beim
/// ersten Aufruf 0 sein und gibt die Rekursionstiefe an.
/////////////////////////////////////////////////////////////////////////////
void AIThread::find_move( short int rec_depth )
{
  const short int PLAYER_IND = ( m_game->get_act_player() == BG::WHITE )
                                 ? 1 : -1;
      // Um if-Abfragen übersichtlicher zu gestalten.
  bool mutex_locked = false; // Gibt an, ob das Mutex für die Arrays in
                             // m_game gesperrt wurde, damit dieses nicht
                             // mehrmals entsperrt wird, was zu einem
                             // undefinierten Verhalten führt.
  /*< \label{void:AIThread::find_move(short:int*,short:int)::my_dice} >*/
  bool all_dice_used; // Für alle Würfel gesetzt?
  short int end_position; // Endposition eines Zuges für Berechnungszwecke.
  double rating; // Bewertung der Spielsituation
  unsigned short int i, j;

  /* Variablen initialisieren */
  if( rec_depth == 0 )
    {
      /* m_dice initialisieren */
      m_game->lock_arrays();
      if( m_game->get_dice_result_has_to_be_used()[ 0 ]
          && m_game->get_dice_result_has_to_be_used()[ 1 ] )
        {
          for( i = 0; i < 4; i++ )
            m_dice[ i ] = m_game->get_dice()[ i ];
        }
      else if( ( m_game->get_dice_result_has_to_be_used()[ 0 ]
                 && m_game->get_dice()[ 0 ] >= m_game->get_dice()[ 1 ] )
               || ( m_game->get_dice_result_has_to_be_used()[ 1 ]
                    && m_game->get_dice()[ 0 ] <= m_game->get_dice()[ 1 ] ) )
        {
          m_dice[ 0 ] = m_game->get_dice()[ 0 ];
          m_dice[ 1 ] = m_dice[ 2 ] = m_dice[ 3 ] = 0;
        }
      else if( ( m_game->get_dice_result_has_to_be_used()[ 0 ]
                 && m_game->get_dice()[ 0 ] <= m_game->get_dice()[ 1 ] )
               || ( m_game->get_dice_result_has_to_be_used()[ 1 ]
                    && m_game->get_dice()[ 0 ] >= m_game->get_dice()[ 1 ] ) )
        {
          m_dice[ 0 ] = m_game->get_dice()[ 1 ];
          m_dice[ 1 ] = m_dice[ 2 ] = m_dice[ 3 ] = 0;
        }
      m_game->unlock_arrays();

      /* m_changes initialisieren */
      for( i = 0; i < 25; i++ )
        m_changes[ i ] = 0;
    }

  /* Sicherstellen, dass auf jeden Fall eine mögliche Zugkombination
   * zurückgegeben werden kann. */
  if( rec_depth == 0 )
    {
      m_game_copy.copy_without_turn_list( *m_game );

      i = m_game_copy.get_turn_number(); // Zur Feststellung, wann so viele
                                         // Züge in m_moves sind, dass keine
                                         // weiteren gemacht werden können.
      m_best_move_mutex.lock();
      m_best_move.clear();
      while( m_game_copy.get_turn_number() == i
             && m_game_copy.get_game_status() != BG::GAME_FINISHED )
        {
          m_best_move.push_back( BG::BackgammonMove() );
            if( m_game_copy.is_valid_move_possible( &m_best_move.back() ) )
              m_game_copy.move( m_best_move.back() );
        }
      m_best_move_rating = rate_game_situation( &m_game_copy,
                                              m_game->get_act_player() );
      m_best_move_mutex.unlock();

      set_moves_set( true );
    }

  if( get_request() != FIND_MOVE )
    return;

  /* Tiefste Rekursionsstufe? / Für alle Würfel gesetzt? Dann muss geprüft
   * werden, ob der aktuelle Zug in m_moves besser ist als der in
   * m_best_move. */
  all_dice_used = true;
  for( i = 0; all_dice_used && i < 4; i++ )
    all_dice_used &= ( m_dice[ i ] <= 0 );
  m_game->lock_arrays();
  mutex_locked = true;
  if( all_dice_used || ( m_game->get_beared_off()[ m_game->get_act_player() ]
                         + m_moves.size() >= 15 )
      || rec_depth >= 2 )
    {
      m_game->unlock_arrays();
      mutex_locked = false;

      m_game_copy.copy_without_turn_list( *m_game );

      i = m_game_copy.get_turn_number(); // Zur Feststellung, on so viele
                                         // Züge in m_moves sind, dass keine
                                         // weiteren gemacht werden können.

       if( m_game_copy.move( m_moves )
           && ( m_game_copy.get_turn_number() != i
                || m_game_copy.get_game_status() == BG::GAME_FINISHED ) )
         {
           QMutexLocker lock( &m_best_move_mutex );
           rating = rate_game_situation( &m_game_copy,
                                         m_game->get_act_player() );
          if( rating > m_best_move_rating )
            {
              m_best_move_rating = rating;
              m_best_move.clear();
              m_best_move = m_moves;
            }
          return;
        }
    }
  if( mutex_locked )
    {
      m_game->unlock_arrays();
      mutex_locked = false;
    }

  if( get_request() != FIND_MOVE )
    return;

  /* Mögliche Züge systematisch u. rekursiv durchgehen */
  m_game->lock_arrays();
  for( i = 0; i < 25 && get_request() == FIND_MOVE; i++ )
    {
      if( ( i > 23 && m_game->get_on_bar()[ m_game->get_act_player() ]
                      + m_changes[ i ] < 1 )
          || ( i <= 23
               && m_game->get_points()[ i ] * PLAYER_IND + m_changes[ i ]
                  + (( m_game->get_points()[ i ] * PLAYER_IND == -1 )
                     ? 1 : 0 ) < 1 ) )
        continue;

      m_changes[ i ]--;
      for( j = 0; j < 4 && get_request() == FIND_MOVE; j++ )
        {
          if( m_dice[ j ] < 1 || m_dice[ j ] > 6 )
            continue;

          if( i > 23 )
            end_position =
                (( m_game->get_act_player() == BG::WHITE ) ? -1 : 24)
                + m_dice[ j ] * PLAYER_IND;
          else
            end_position = i + m_dice[ j ] * PLAYER_IND;

          if( end_position >= 0 && end_position <= 23 )
            m_changes[ end_position ]++;

          m_moves.push_back(
              BG::BackgammonMove( ( i <= 23 ) ? i
                                              : static_cast< short int >
                                                (BG::BAR),
                                  m_dice[ j ] ) );
          m_dice[ j ] = 0;

          if ( end_position < 0 || end_position > 23 ||
               PLAYER_IND * m_game->get_points()[ end_position ] >= -1 )
            {
              m_game->unlock_arrays();
              find_move( rec_depth + 1 );
              m_game->lock_arrays();
            }

          m_dice[ j ] = m_moves.back().get_distance();
          m_moves.pop_back();

          if( end_position >= 0 && end_position <= 23 )
            m_changes[ end_position ]--;
        }
      m_changes[ i ]++;
    }
  m_game->unlock_arrays();
}

/*< \label{double:AIThread::rate_game_situation(const:BG::Backgammon*,BG::Player)const} >*/
/////////////////////////////////////////////////////////////////////////////
/// Bewertet die Spielsituation in \a game für Spieler \a player und gibt
/// die Bewertung zurück.
/////////////////////////////////////////////////////////////////////////////
double AIThread::rate_game_situation( const BG::Backgammon *game,
                                      BG::Player player ) const
{
  const short int PLAYER_IND = ( player == BG::WHITE ) ? 1 : -1;
      // Zur Vereinfachung von if-Abragen etc.
  double rating = 0;
  unsigned int i;

  /* Wahrscheinlichkeit, dass eigener Spielstein geschlagen wird */
  game->lock_arrays();
  for( i = 0; i < 24; i++ )
    {
      if( game->get_points()[ i ] * PLAYER_IND == 1 )
        rating += prob_position_is_reached( game, i,
                                            ( player == BG::WHITE )
                                              ? BG::BLACK : BG::WHITE )
                  * (( player == BG::WHITE ) ? i : 23 - i) / 23.0
                  * ( 2 - prob_get_checker_back_into_game( game, player ) )
                  * m_rating_factors[ PROB_CHECKER_IS_HIT ];
    }

  /* Wahrscheinlichkeiten selber zugunfähig zu sein */
  for( i = 0; i < 24; i++ )
    {
      if( game->get_points()[ i ] * PLAYER_IND > 0 )
        rating += prob_player_cannot_move_at_pos( BG::Position( i ), game,
                                                  player )
                  * game->get_points()[ i ] * PLAYER_IND
                  * (( player == BG::WHITE ) ? 23 - i : i) / 23.0
                  * m_rating_factors[ PROB_CANNOT_MOVE ];
    }

  /* Wahrscheinlichkeit, dass der Gegner zugunfähig ist */
  for( i = 0; i < 24; i++ )
    {
      if( game->get_points()[ i ] * PLAYER_IND < 0 )
        rating += prob_player_cannot_move_at_pos( BG::Position( i ), game,
                                                  ( player == BG::WHITE )
                                                    ? BG::BLACK : BG::WHITE )
                  * game->get_points()[ i ] * PLAYER_IND * -1
                  * (( player == BG::WHITE ) ? i : 23 - i) / 23.0
                  * m_rating_factors[ PROB_OP_CANNOT_MOVE ];
    }
  rating += prob_player_cannot_move_at_pos( BG::BAR, game,
                                            ( player == BG::WHITE )
                                              ? BG::BLACK : BG::WHITE )
            * game->get_on_bar()[ ( player == BG::WHITE )
                                    ? BG::BLACK : BG::WHITE ]
            * m_rating_factors[ PROB_OP_CANNOT_MOVE_BAR ];
  game->unlock_arrays();

  /* Zahl der "Zungen" mit mindestes zwei eigenen Spielsteinen */
  rating += n_points_with_checkers( game, player ) / 7.0
            * m_rating_factors[ N_POINTS_WITH_CHECKERS ];

  /* Zahl der ausgewürfelten Spielsteine */
  game->lock_arrays();
  rating += game->get_beared_off()[ player ] / 15.0
            * m_rating_factors[ BEARED_OFF ];
  game->unlock_arrays();

  /* Durchschnittliche Entfernung vom Homeboard. */
  rating += distance_from_homeboard( game, player ) / 18.0
            * m_rating_factors[ DISTANCE_FROM_HOMEBOARD ];

  /* Zahl der Spielsteine im gegnerischem Homeboard */
  rating += n_checkers_in_op_homeboard( game, player ) / 15.0
            * m_rating_factors[ CHECKERS_IN_OP_HOMEBOARD ];

  /* Durchschnittliche Entfernung zum Auswürfeln beim Gegener */
  rating += distance_from_off_game( game,
                                    ( player == BG::WHITE ) ? BG::BLACK
                                                            : BG:: WHITE )
            / 24.0
            * m_rating_factors[ OP_DISTANCE_FROM_OFF_GAME ];

  return rating;
}

/////////////////////////////////////////////////////////////////////////////
/// Gibt zurück, für wieviele Felder \a player beim Backgammon-Spiel \a game
/// durchschnittlich ziehen muss, um einen Spielstein in das Homeboard zu
/// bringen.
/////////////////////////////////////////////////////////////////////////////
double AIThread::distance_from_homeboard( const BG::Backgammon *game,
                                         BG::Player player ) const
{
  const short int PLAYER_IND = ( player == BG::WHITE ) ? 1 : -1;
      // Zur Vereinfachung von if-Abragen etc.
  unsigned int distance_sum = 0; // Summe der Entfernungen zum Auswürfeln
  unsigned int i;

  game->lock_arrays();
  for( i = ( player == BG::WHITE ) ? 0 : 23;
       ( player == BG::WHITE && i < 18 ) || ( player == BG::BLACK && i > 5 );
       i += PLAYER_IND )
    {
      if( game->get_points()[ i ] * PLAYER_IND > 0 )
        distance_sum += game->get_points()[ i ] * PLAYER_IND
                        * ( player == BG::WHITE ) ? 18 - i : i - 5;
    }
  distance_sum += game->get_on_bar()[ player ] * 25;
  game->unlock_arrays();

  return distance_sum / 15.0;
}

/////////////////////////////////////////////////////////////////////////////
/// Gibt zurück, für wieviele Felder \a player beim Backgammon-Spiel \a game
/// durchschnittlich ziehen muss, um einen Spielstein auszuwürfeln.
/////////////////////////////////////////////////////////////////////////////
double AIThread::distance_from_off_game( const BG::Backgammon *game,
                                         BG::Player player ) const
{
  const short int PLAYER_IND = ( player == BG::WHITE ) ? 1 : -1;
      // Zur Vereinfachung von if-Abragen etc.
  unsigned int distance_sum = 0; // Summe der Entfernungen zum Auswürfeln
  unsigned int i;

  game->lock_arrays();
  for( i = 0; i < 24; i++ )
    {
      if( game->get_points()[ i ] * PLAYER_IND > 0 )
        distance_sum += game->get_points()[ i ] * PLAYER_IND
                        * ( player == BG::WHITE ) ? 24 - i : i + 1;
    }
  distance_sum += game->get_on_bar()[ player ] * 25;
  game->unlock_arrays();

  return distance_sum / 15.0;
}

/////////////////////////////////////////////////////////////////////////////
/// Gibt die Anzahl an Spielsteinen von \a player im gegnerischen Homeboard
/// und auf der Bar beim Backgammon-Spiel \a game zurück.
/////////////////////////////////////////////////////////////////////////////
unsigned short int AIThread::n_checkers_in_op_homeboard(
        const BG::Backgammon *game, BG::Player player ) const
{
  const short int PLAYER_IND = ( player == BG::WHITE ) ? 1 : -1;
      // Zur Vereinfachung von if-Abragen etc.
  unsigned short int ret_val = 0;
  unsigned int i;

  game->lock_arrays();
  for( i = ( player == BG::WHITE ) ? 23 : 0; i <= 5 || i >= 18;
       i -= PLAYER_IND )
    {
      if( game->get_points()[ i ] * PLAYER_IND > 0 )
        ret_val += game->get_points()[ i ] * PLAYER_IND;
    }

  ret_val += game->get_on_bar()[ player ];
  game->unlock_arrays();
  return ret_val;
}

/////////////////////////////////////////////////////////////////////////////
/// Gibt die Zahl der "Zungen" beim Backgammon-Spiel \a game mit mindestens
/// zwei Spielsteinen von \a player zurück.
/////////////////////////////////////////////////////////////////////////////
unsigned short int AIThread::n_points_with_checkers(
    const BG::Backgammon *game, BG::Player player ) const
{
  const short int PLAYER_IND = ( player == BG::WHITE ) ? 1 : -1;
      // Zur Vereinfachung von if-Abragen etc.
  unsigned int i, ret_val = 0;

  game->lock_arrays();
  for( i = 0; i < 24; i++ )
    {
      if( game->get_points()[ i ] * PLAYER_IND > 1 )
        ret_val++;
    }
  game->unlock_arrays();

  return ret_val;
}

/////////////////////////////////////////////////////////////////////////////
/// Gibt die Wahrscheinlichkeit zurück, dass Spieler \a player im Spiel
/// \a game einen Spielstein wieder einwürfeln kann. Dabei wird nicht
/// berücksichtigt, ob sich überhaupt Steine auf der Bar befinden.
/////////////////////////////////////////////////////////////////////////////
double AIThread::prob_get_checker_back_into_game( const BG::Backgammon *game,
                                                  BG::Player player ) const
{
  const short int PLAYER_IND = ( player == BG::WHITE ) ? 1 : -1;
      // Zur Vereinfachung von if-Abragen etc.
  double ret_val = 0;
  unsigned int i;

  game->lock_arrays();
  for( i = ( player == BG::WHITE ) ? 0 : 23; i <= 5 || i >= 18;
       i += PLAYER_IND )
    {
      if( game->get_points()[ i ] * PLAYER_IND >= -1 )
        ret_val += 1.0 / 6.0;
    }
  game->unlock_arrays();

  return 1.0 - ( (1.0 - ret_val) * (1.0 - ret_val) );
      // Die Wahrscheinlichkeit muss hier umgerechnet werden, da mit zweien
      // anstatt einem Würfel gewürfelt wird.
}

/*< \label{double:AIThread::prob_player_cannot_move_at_pos(BG::Position,const:BG::Backgammon,BG::Player)const} >*/
/////////////////////////////////////////////////////////////////////////////
/// Gibt die Wahrscheinlichkeit zurück, dass der Spieler \a player im
/// nächsten Zug die Spielsteine auf \a pos nicht ziehen kann, wobei die
/// aktuelle Spielsituation in \a game übergeben wird.
/////////////////////////////////////////////////////////////////////////////
double AIThread::prob_player_cannot_move_at_pos( BG::Position pos,
                                                 const BG::Backgammon *game,
                                                 BG::Player player ) const
{
  const short int PLAYER_IND = ( player == BG::WHITE ) ? 1 : -1;
      // Zur Vereinfachung von if-Abragen etc.
  bool dice_result_usable[ 6 ]; // Gibt an, ob eine bestimmte Augenzahl ge-
                                // setzt werden kann.
  double ret_val = 1;
  int i, j;

  assert( player == BG::WHITE || player == BG::BLACK );

  for( i = 0; i < 6; i++ )
    dice_result_usable[ i ] = false;

  game->lock_arrays();
  if( pos == BG::BAR )
    {
      for( i = ( player == BG::WHITE ) ? 0 : 23, j = 0; j < 6;
           i += PLAYER_IND, j++ )
        {
          if( game->get_points()[ i ] * PLAYER_IND >= -1 )
            dice_result_usable[ j ] = true;
        }
    }
  else
    {
      for( i = 0; i < 6; i++ )
        {
          if( pos + (i + 1) * PLAYER_IND > 23
              || pos + (i + 1) * PLAYER_IND < 0 )
            {
              if( game->get_are_all_in_homeboard()[ player ] )
                {
                  if( pos + (i + 1) * PLAYER_IND == 24
                      || pos + (i + 1) * PLAYER_IND == -1 )
                    dice_result_usable[ i ] = true;
                  else
                    {
                      for( j = pos; j >= 18 || j <= 5; j -= PLAYER_IND )
                        {
                          if( game->get_points()[ j ] * PLAYER_IND > 0 )
                            break;
                        }
                      if( j > 5 && j < 18 )
                        dice_result_usable[ i ] = true;
                    }
                }
            }
          else
            {
              if( game->get_points()[ pos + (i + 1) * PLAYER_IND ]
                  * PLAYER_IND >= -1 )
                dice_result_usable[ i ] = true;
            }
        }
    }
  game->unlock_arrays();

  for( i = 0; i < 6; i++ )
    {
      if( dice_result_usable[ i ] )
        ret_val -= 1.0 / 6.0;
    }
  return ret_val * ret_val;
}

/*< \label{double:AIThread::prob_position_is_reached(const:BG::Backgammon*,short:int,BG::Player)const} >*/
/////////////////////////////////////////////////////////////////////////////
/// Gibt die Wahrscheinlichkeit zurück, dass der Spieler \a player die
/// in \a position übergebene Position auf dem Spielfeld im nächsten Zug
/// erreichen kann, wobei die aktuelle Spielsituation in \a game übergeben
/// wird.
///
/// \attention \a position darf nicht BG::BAR oder BG::OUT_OF_GAME sein.
/////////////////////////////////////////////////////////////////////////////
double AIThread::prob_position_is_reached( const BG::Backgammon *game,
                                           short int position,
                                           BG::Player player ) const
{
  const short int PLAYER_IND = ( player == BG::WHITE ) ? 1 : -1;
      // Zur Vereinfachung von if-Abragen etc.
  bool possible_dice_combinations[ 6 ][ 6 ]; // Würfelkombinationen, bei
                                             // denen der Gegner schlagen
                                             // könnte.
  double prob = 0.0; // Ermittelte Wahrscheinlichkeit
  short int checking_position; // Speichert die Position für die gerade die
                               // Wahrscheinlichkeit geprüft wird, dass sie
                               // vom Gegener erreicht werden kann.
  int i, j;

  assert( position >= 0 && position <= 23 );
  assert( player == BG::WHITE || player == BG::BLACK );

  /* possible_dice_combinations initialisieren */
  for( i = 0; i < 6; i++ )
    {
      for( j = 0; j < 6; j++ )
        possible_dice_combinations[ i ][ j ] = false;
    }

  /* Möglichkeiten die Position nur mit der Augenzahl eines Würfels zu
   * erreichen */
  game->lock_arrays();
  for( i = 0; i < 6; i++ )
    {
      checking_position = position - (i + 1) * PLAYER_IND;
      if( checking_position > 24 || checking_position < -1 )
        break;

      if( checking_position == 24 || checking_position == -1 )
        {
          if( game->get_on_bar()[ player ] > 0 )
            {
              for( j = 0; j < 6; j++ )
                {
                  possible_dice_combinations[ i ][ j ] = true;
                  possible_dice_combinations[ j ][ i ] = true;
                }
            }
        }
      else if( game->get_points()[ checking_position ] * PLAYER_IND > 0
               && ( game->get_on_bar()[ player ] < 2
                    || ( i == j && game->get_on_bar()[ player ] < 4 ) ) )
        {
          for( j = 0; j < 6; j++ )
            {
              possible_dice_combinations[ i ][ j ] = true;
              possible_dice_combinations[ j ][ i ] = true;
            }
        }
      else if( game->get_points()[ checking_position ] * PLAYER_IND == -1
               || game->get_points()[ checking_position ] == 0 )
        {
          /* Möglichkeiten die Position nur mit der Augenzahl beider Würfel
           * zu erreichen */
          for( j = 0; j < 6; j++ )
            {
              checking_position = position - (i + j + 2) * PLAYER_IND;
              if( checking_position > 24 || checking_position < -1 )
                break;

              if( checking_position == 24 || checking_position == -1 )
                {
                  if( game->get_on_bar()[ player ] > 0 )
                    {
                      possible_dice_combinations[ i ][ j ] = true;
                      possible_dice_combinations[ j ][ i ] = true;
                    }
                }
              else if( game->get_points()[ checking_position ]
                       * PLAYER_IND > 0
                       && ( game->get_on_bar()[ player ] < 1
                            || ( i == j
                                 && game->get_on_bar()[ player ] < 3 ) ) )
                {
                  possible_dice_combinations[ i ][ j ] = true;
                  possible_dice_combinations[ j ][ i ] = true;
                }
              else if( game->get_points()[ checking_position ]
                         * PLAYER_IND == -1
                       ||game->get_points()[ checking_position ] == 0
                       && i == j )
                {
                  /* Möglichkeiten die Position mit einem Pasch zu
                   * erreichen */
                  checking_position = position - (3 * i + 3) * PLAYER_IND;
                  if( checking_position > 24 || checking_position < -1
                      || possible_dice_combinations[ i ][ i ] )
                    continue;

                  if( checking_position == 24 || checking_position == -1 )
                    {
                      if( game->get_on_bar()[ player ] > 0 )
                        possible_dice_combinations[ i ][ i ] = true;
                    }
                  else if( game->get_points()[ checking_position ]
                           * PLAYER_IND > 0
                           && game->get_on_bar()[ player ] < 2 )
                    possible_dice_combinations[ i ][ i ] = true;
                  else if( game->get_points()[ checking_position ]
                             * PLAYER_IND == -1
                           || game->get_points()[ checking_position ] == 0 )
                    {
                      checking_position = position
                                          - (4 * i + 4) * PLAYER_IND;
                      if( checking_position > 24 || checking_position < -1 )
                        continue;

                      if( checking_position == 24
                          || checking_position == -1 )
                        {
                          if( game->get_on_bar()[ player ] > 0 )
                            possible_dice_combinations[ i ][ i ] = true;
                        }
                      else if( game->get_points()[ checking_position ]
                               * PLAYER_IND > 0
                               && game->get_on_bar()[ player ] < 1 )
                        possible_dice_combinations[ i ][ i ] = true;
                    }
                }
            }
        }
    }
  game->unlock_arrays();

  /* Wahrscheinlichkeit zusammenrechnen */
  for( i = 0; i < 6; i++ )
    {
      for( j = 0; j < 6; j++ )
        {
          if( possible_dice_combinations[ i ][ j ] )
            prob += 1.0 / 36.0;
        }
    }

  return prob;
}
