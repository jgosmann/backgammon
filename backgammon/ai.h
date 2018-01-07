/*!
 * \file ai.h
 * \brief Deklaration der KI-Klasse, f�r die Backgammon-KI
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

#ifndef AI_H
#define AI_H 1

#include <QMutex>
#include <QObject>
#include <QTimer>

#include "backgammon.h"
#include "aithread.h"

#include "ai_std_config.h"

/*< \label{AI} >*/
/////////////////////////////////////////////////////////////////////////////
/// Backgammon-KI, eigentlich eine Art Wrapper f�r AIThread. So werden die
/// KI-Berechnung im Hintergrund ausgef�hrt.
/////////////////////////////////////////////////////////////////////////////
class AI : public QObject
{
  Q_OBJECT

  public:
    AI( BG::Backgammon *game, unsigned int timeout = AI_STD_TIMEOUT );
    ~AI( void );

    void move( void ); ///< \brief KI-Berechnungen starten und ziehen.
    void wait( void ); ///< \brief Wartet auf den AI-Thread, bis dieser
                       ///< wieder schlafend ist.

    inline unsigned int get_timeout( void ) const { return m_timeout; }
    inline double get_rating_factor( AIThread::RatingFactors factor ) const
        { return m_ai_thread.get_rating_factor( factor ); };
    inline void set_timeout( unsigned int val ) { m_timeout = val; }
        ///< \brief Setzt \ref m_timeout.
    inline void set_rating_factor( AIThread::RatingFactors factor,
                                   double value )
        { m_ai_thread.set_rating_factor( factor, value ); };
        ///< \brief Setzt \ref AIThread::m_rating_factors.

  protected slots:
    void do_move( void ); ///< \brief Zug ausf�hren.

  private:
    unsigned int m_timeout; ///< \brief Timeout f�r die KI in Millisekunden.
    QTimer m_timer; ///< \brief Timer zur Realisierung des Timeouts.
    /*< \label{AI::m_ai_thread} >*/
    AIThread m_ai_thread; ///< \brief AI-Thread, der die Berechnungen
                          ///< �bernimmt.
    BG::Backgammon *m_game; ///< \brief Backgammon-Spiel, f�r das die KI
                            ///< die Berechnungen vornimmt.
    QMutex m_not_moved; ///< \brief Wenn dieses Mutex nicht gesperrt ist,
                        ///< so hat die KI seit dem letzten Aufruf von move()
                        ///< noch nicht gezogen.

};

#endif
