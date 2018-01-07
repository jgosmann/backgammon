/*!
 * \file serversettings.cpp
 * \brief Implementation of the \ref ServerSettings class.
 * \date So Mai 18 2008
 * \author Jan Gosmann (jan@hyper-world.de)
 *
 * \b Copyright: Copyright (C) 2006 - 2009 Jan Gosmann
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

#include "serversettings.h"

#include <QFile>
#include <QHashIterator>
#include <QXmlInputSource>
#include <QXmlSimpleReader>
#include <QXmlStreamWriter>

#include <iostream>
#include <stdexcept>
using namespace std;

////////////////////////////////////////////////////////////////////////////////
/// Initializes the object with the default values
ServerSettings::ServerSettings( void ) :
    i_allow_unregistered( false ), my_port( 30167 ), my_max_connections( 16 ),
            my_motd( "" ), i_read_motd( false )
{
}

/////////////////////////////////////////////////////////////////////////////
/// Constructor which read \a conffile to initialize the server settings.
/////////////////////////////////////////////////////////////////////////////
ServerSettings::ServerSettings( QString conffile ) :
    i_allow_unregistered( false ), my_port( 30167 ), my_max_connections( 16 ),
            my_motd( "" ), i_read_motd( false )
{
    readFromXml( conffile );
}

////////////////////////////////////////////////////////////////////////////////
/// Reads \a conffile to initialize the server settings.
////////////////////////////////////////////////////////////////////////////////
void ServerSettings::readFromXml( QString conffile )
{
    QFile input( conffile );
    if( !input.open( QIODevice::ReadOnly ) )
        throw runtime_error( "File could not be opened." );
    readFrom( &input );
}

////////////////////////////////////////////////////////////////////////////////
/// Writes the server settings to the xml file passed with \a conffile.
////////////////////////////////////////////////////////////////////////////////
void ServerSettings::writeToXml( QString conffile )
{
    QFile output( conffile );
    if( !output.open( QIODevice::WriteOnly ) )
        throw runtime_error( "File could not be opened." );
    writeTo( &output );
}

////////////////////////////////////////////////////////////////////////////////
/// Reads \a buf to initialize the server settings.
////////////////////////////////////////////////////////////////////////////////
void ServerSettings::readFrom( QIODevice *buf )
{
    i_read_motd = false;
    QXmlSimpleReader reader;
    QXmlInputSource source( buf );

    reader.setContentHandler( this );
    reader.setErrorHandler( this );

    if( !reader.parse( &source ) )
        throw runtime_error( "Config parse error." );
}

////////////////////////////////////////////////////////////////////////////////
/// Writes the server settings to \a buf.
////////////////////////////////////////////////////////////////////////////////
void ServerSettings::writeTo( QIODevice *buf )
{
    QXmlStreamWriter writer( buf );

    writer.setAutoFormatting( true );
    writer.writeStartDocument();
    writer.writeStartElement( "bgserverconf" );

    writer.writeComment(
                         " This message is shown when a player connects to the "
                             "server. " );
    writer.writeTextElement( "motd", my_motd );

    writer.writeComment( " If you set allowunregistered to true everyone can "
        "join your server. Otherwise you have to login with "
        "an username listed below. In any case the usernames "
        "below can only be used with the password. " );
    writer.writeStartElement( "users" );
    writer.writeAttribute( "allowunregistered",
                           (i_allow_unregistered) ? "true" : "false" );
    QHashIterator<QString, QString> it( my_users );
    while( it.hasNext() )
    {
        it.next();
        writer.writeStartElement( "user" );
        writer.writeAttribute( "name", it.key() );
        writer.writeAttribute( "pass", it.value() );
        writer.writeEndElement();
    }
    writer.writeEndElement();

    writer.writeComment( " This is the port on which the server will be "
        "listening. " );
    writer.writeStartElement( "option" );
    writer.writeAttribute( "name", "port" );
    writer.writeAttribute( "value", QString::number( my_port ) );
    writer.writeEndElement();

    writer.writeComment( " This value is the maximum number of simultaneous "
        "connections which the server will allow. " );
    writer.writeStartElement( "option" );
    writer.writeAttribute( "name", "max_connections" );
    writer.writeAttribute( "value", QString::number( my_max_connections ) );
    writer.writeEndElement();

    writer.writeEndElement();
    writer.writeEndDocument();
}

/////////////////////////////////////////////////////////////////////////////
/// Reads the contents of one XML element.
/////////////////////////////////////////////////////////////////////////////
bool ServerSettings::characters( const QString &ch )
{
    if( i_read_motd )
        my_motd = ch;
    return true;
}

/////////////////////////////////////////////////////////////////////////////
/// Ends the processing of one XML element.
/////////////////////////////////////////////////////////////////////////////
bool ServerSettings::endElement( const QString &namespaceURI,
                                 const QString &localName, const QString &qName )
{
    if( qName == "motd" )
        i_read_motd = false;
    return true;
}

/////////////////////////////////////////////////////////////////////////////
/// Processes one configuration setting.
/////////////////////////////////////////////////////////////////////////////
bool ServerSettings::startElement( const QString &namespaceURI,
                                   const QString &localName,
                                   const QString &qName,
                                   const QXmlAttributes &atts )
{
    if( qName == "users" )
    {
        QString val = atts.value( "allowunregistered" );
        if( val == "true" || val == "1" )
            i_allow_unregistered = true;
        else
            i_allow_unregistered = false;
    }
    else if( qName == "user" )
        my_users[atts.value( "name" )] = atts.value( "pass" );
    else if( qName == "option" )
    {
        QString name = atts.value( "name" );
        if( name == "port" )
            my_port = atts.value( "value" ).toInt();
        else if( name == "max_connections" )
            my_max_connections = atts.value( "value" ).toInt();
    }
    else if( qName == "motd" )
        i_read_motd = true;
    return true;
}
