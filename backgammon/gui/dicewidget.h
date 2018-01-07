/*!
 * \file dicewidget.h
 * \brief Deklaration der DiceWidget-Klasse, zur Darstellung eines 6-seitigen
 *        Würfels.
 * \date Tu Nov 28 2006
 * \author Jan Gosmann (jan@hyper-world.de)
 *
 * \b Copyright: Copyright (C) 2006 Jan Gosmann
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

#ifndef DICEWIDGET_H
#define DICEWIDGET_H 1

#include <QMouseEvent>
#include <QWidget>

/////////////////////////////////////////////////////////////////////////////
/// Diese Klasse stellt ein Widget zur Darstellung eines Würfelergebnisses
/// eines sechs-seitigen Würfels bereit.
/////////////////////////////////////////////////////////////////////////////
class DiceWidget : public QWidget
{
  Q_OBJECT

  public:
    DiceWidget( QWidget *parent = 0 );

    inline unsigned short int get_value( void ) const { return m_value; }
    inline QColor get_bg_color( void ) const { return m_bg_color; }
    inline QColor get_fg_color( void ) const { return m_fg_color; }

  public slots:
    void set_value( unsigned short int value );
        ///< \brief Setzt \ref m_value.
    void set_bg_color( QColor color ); ///< \brief Setzt \ref m_bg_color.
    void set_fg_color( QColor color ); ///< \brief Setzt \ref m_fg_color.

  signals:
    void clicked( void );

    void value_changed( unsigned short int value );
    void bg_color_changed( QColor color );
    void fg_color_changed( QColor color );

  protected:
    virtual void mousePressEvent( QMouseEvent *event ) { emit clicked(); }
    virtual void paintEvent( QPaintEvent *event );
    virtual void resizeEvent( QResizeEvent *event );

  private:
    unsigned short int m_value; ///< \brief Anzuzeigender Wert.
    QColor m_bg_color; ///< \brief Hintergrundsfarbe
    QColor m_fg_color; ///< \brief Vordergrundsfarbe
};

#endif
