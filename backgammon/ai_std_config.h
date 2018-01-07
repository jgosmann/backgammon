/*!
 * \file ai_std_config.h
 * \brief Diese Datei enthält Deklaration für die Standard-Parameter der KI.
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

#ifndef AI_STD_CONFIG_H
#define AI_STD_CONFIG_H 1

#include "aithread.h"

/// Standard-Timeout in Millisekunden
#define AI_STD_TIMEOUT 3000

/// Standardwerte für die Bewertungsfaktoren.
const double AI_STD_RATING_FACTORS[ AIThread::NUM_RATING_FACTORS ] =
  {
    -400.0, // PROB_CHECKER_IS_HIT
    -400.0, // PROB_CANNOT_MOVE
     100.0, // PROB_OP_CANNOT_MOVE
     100.0, // PROB_OP_CANNOT_MOVE_BAR
     100.0, // N_POINTS_WITH_CHECKERS
     100.0, // BEARED_OFF
    -600.0, // DISTANCE_FROM_HOMEBOARD
    -100.0, // CHECKERS_IN_OP_HOMBEBOARD
     700.0  // OP_DISTANCE_FROM_OFF_GAME
  };

#endif
