/*!
 * \file dicewidget.cpp
 * \brief Implementation der DiceWidget-Klasse, zur Darstellung eines
 *        6-seitigen Würfels.
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

#include <QWidget>
#include <QPainter>

#include "dicewidget.h"

/////////////////////////////////////////////////////////////////////////////
/// Konstruktor
/////////////////////////////////////////////////////////////////////////////
DiceWidget::DiceWidget( QWidget *parent )
    : QWidget( parent ), m_value( 0 ), m_bg_color( QColor( 0, 0, 0) ),
      m_fg_color( QColor( 255, 255, 255 ) )
{

}

/////////////////////////////////////////////////////////////////////////////
/// Setzt \ref m_value auf \a value.
/////////////////////////////////////////////////////////////////////////////
void DiceWidget::set_value( unsigned short int value )
{
  if( value == m_value )
    return;
  m_value = value;
  update();
  emit value_changed( m_value );
}

/////////////////////////////////////////////////////////////////////////////
/// Setzt \ref m_bg_color auf \a color.
/////////////////////////////////////////////////////////////////////////////
void DiceWidget::set_bg_color( QColor color )
{
  if( color == m_bg_color )
    return;
  m_bg_color = color;
  emit bg_color_changed( m_bg_color );
}

/////////////////////////////////////////////////////////////////////////////
/// Setzt \ref m_fg_color auf \a color.
/////////////////////////////////////////////////////////////////////////////
void DiceWidget::set_fg_color( QColor color )
{
  if( color == m_fg_color )
    return;
  m_fg_color = color;
  emit fg_color_changed( m_fg_color );
}

/////////////////////////////////////////////////////////////////////////////
/// Zeichnet das Widget.
/////////////////////////////////////////////////////////////////////////////
void DiceWidget::paintEvent( QPaintEvent *event )
{
  QPainter painter( this );
  const double dot_radius = width() / 10;

  painter.setPen( m_fg_color );
  painter.setBrush( m_bg_color );
  painter.drawRoundRect( 0, 0, this->width() - 1, this->height() - 1,
                         25, 25 );

  painter.setPen( m_bg_color );
  painter.setBrush( m_fg_color );

  if( m_value == 1 || m_value == 3 || m_value == 5 )
    painter.drawEllipse( QRectF( width() / 2 - dot_radius,
                                 height() / 2 - dot_radius,
                                 2 * dot_radius, 2 * dot_radius ) );
  if( m_value >= 2)
    {
      painter.drawEllipse( QRectF( 3 * width() / 4 - 0.5 * dot_radius,
                                   height() / 4 - 1.5 * dot_radius,
                                   2 * dot_radius, 2 * dot_radius ) );
      painter.drawEllipse( QRectF( width() / 4 - 1.5 * dot_radius,
                                   3 * height() / 4 - 0.5 * dot_radius,
                                   2 * dot_radius, 2 * dot_radius ) );
    }
  if( m_value >= 4)
    {
      painter.drawEllipse( QRectF( width() / 4 - 1.5 * dot_radius,
                                   height() / 4 - 1.5 * dot_radius,
                                   2 * dot_radius, 2 * dot_radius ) );
      painter.drawEllipse( QRectF( 3 * width() / 4 - 0.5 * dot_radius,
                                   3 * height() / 4 - 0.5 * dot_radius,
                                   2 * dot_radius, 2 * dot_radius ) );
    }
  if( m_value == 6 )
    {
      painter.drawEllipse( QRectF( width() / 4 - 1.5 * dot_radius,
                                   height() / 2 - dot_radius,
                                   2 * dot_radius, 2 * dot_radius ) );
      painter.drawEllipse( QRectF( 3 * width() / 4 - 0.5 * dot_radius,
                                   height() / 2 - dot_radius,
                                   2 * dot_radius, 2 * dot_radius ) );
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Sorgt dafür, dass das Widget bei einer Größenänderung quadratisch bleibt.
/////////////////////////////////////////////////////////////////////////////
void DiceWidget::resizeEvent( QResizeEvent *event )
{
  if( width() > height() )
    resize( height(), height() );
  else if( width() < height() )
    resize( width(), width() );
}
