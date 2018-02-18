/*!
 * \file ai.cpp
 * \brief Implementation der KI-Klasse, für die Backgammon-KI
 * \date Mi Dec 27 2006
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

#include <QCoreApplication>
#include <QMutex>
#include <QObject>
#include <QTimer>

#include "backgammon.h"
#include "aithread.h"

#include "ai_std_config.h"

#include "ai.h"

/////////////////////////////////////////////////////////////////////////////
/// Konstruktor
/////////////////////////////////////////////////////////////////////////////
AI::AI( BG::Backgammon *game, unsigned int timeout )
    : m_timeout( timeout ), m_ai_thread( game ), m_game( game )
{
  unsigned int i;

  m_timer.setSingleShot( true );

  connect( &m_timer, SIGNAL( timeout( void ) ),
           this, SLOT( do_move( void ) ) );
  connect( &m_ai_thread, SIGNAL( best_move_found( void ) ),
           this, SLOT( do_move( void ) ), Qt::QueuedConnection );

  for( i = 0; i < AIThread::NUM_RATING_FACTORS; i++ )
    m_ai_thread.set_rating_factor( AIThread::RatingFactors( i ),
                                   AI_STD_RATING_FACTORS[ i ] );

  m_not_moved.lock();
  m_ai_thread.start();
}

/////////////////////////////////////////////////////////////////////////////
/// Destruktor
/////////////////////////////////////////////////////////////////////////////
AI::~AI( void )
{
  m_ai_thread.request( AIThread::TERMINATE );
  m_ai_thread.wait();
  m_not_moved.tryLock();
  m_not_moved.unlock();
}

/*< \label{void:AI::move(void)} >*/
/////////////////////////////////////////////////////////////////////////////
/// Startet die KI-Berechnung und führt spätestens nach \ref m_timeout
/// Millisekunden den bis dahin besten gefundenen Zug aus. Diese Funktion
/// ist nicht blockierend.
/////////////////////////////////////////////////////////////////////////////
void AI::move( void )
{
  m_timer.start( m_timeout );

  /* Sicherstellen, dass die KI schläft und durch keine queued Signals
   * versucht wird einen Zug durchzuführen, der bereits nicht mehr gesetzt
   * werden kann. */
  wait();
  QCoreApplication::processEvents(); // Sicherstellen, dass keine
                                     // Queued-Signals mehr vorliegen (in
                                     // diesem Fall könnte das höchstes
                                     // AIThread::best_move_found() sein).

  m_ai_thread.set_moves_set( false );
  m_not_moved.unlock();

  m_ai_thread.request( AIThread::FIND_MOVE );

  if( !m_timer.isActive() ) // Wenn der Timer bereits abgelaufen ist, wurde
    do_move();              // die do_move()-Funktion nicht ausgeführt, da
                            // m_not_moved noch gesperrt war.
}

/////////////////////////////////////////////////////////////////////////////
/// Wartet bis der AI-Thread wieder schlafen (AIThread::SLEEPING) ist.
/////////////////////////////////////////////////////////////////////////////
void AI::wait( void )
{
  while( m_ai_thread.get_status() != AIThread::SLEEPING )
    QCoreApplication::processEvents();
}

/*< \label{void:AI::do_move(void)} >*/
/////////////////////////////////////////////////////////////////////////////
/// Wenn die "Bedenkzeit" der KI abgelaufen ist, führt diese Funktion den
/// bis jetzt besten gefundenen Zug aus.
/////////////////////////////////////////////////////////////////////////////
void AI::do_move( void )
{
  if( !m_not_moved.tryLock() ) // Falls bereits gesetzt wurde.
    return;

  m_timer.stop();

  /* Sicherstellen, dass bereits m_ai_thread.get_best_move() einen Zug
   * zurückgibt. */
  while( !m_ai_thread.get_moves_set() )
    QCoreApplication::processEvents();

  m_ai_thread.request( AIThread::SLEEP );

  m_ai_thread.lock_best_move();
  m_game->move( m_ai_thread.get_best_move() );
  m_ai_thread.unlock_best_move();
}
