/*!
 * \file serveroptions.cpp
 * \brief Implementation of the ServerOptions class.
 * \date 23.05.2009
 * \author Jan Gosmann (jan@hyper-world.de)
 *
 * \b Copyright: Copyright (C) 2009 Jan Gosmann
 */

#include "serveroptions.h"

////////////////////////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////////////////////////
ServerOptions::ServerOptions( QWidget *parent, Qt::WindowFlags f ) :
    QDialog( parent, f )
{
    setupUi( this );
    updateGui();
    users->insertRow( users->rowCount() );
    connect( this, SIGNAL( accepted( void ) ),
             this, SLOT( updateSettings( void ) ) );
}

////////////////////////////////////////////////////////////////////////////////
/// Reads the server options from a XML configuration file passed with \a file.
/// Afterwards the options in the dialog are replaced with the options read.
////////////////////////////////////////////////////////////////////////////////
void ServerOptions::readFromXml( QString file )
{
    mySettings.readFromXml( file );
    updateGui();
}

////////////////////////////////////////////////////////////////////////////////
/// Writes the current options to the configuration file \a file.
////////////////////////////////////////////////////////////////////////////////
void ServerOptions::writeToXml( QString file )
{
    updateSettings();
    mySettings.writeToXml( file );
}

////////////////////////////////////////////////////////////////////////////////
/// Updates the GUI when the \ref mySettings object was changed.
////////////////////////////////////////////////////////////////////////////////
void ServerOptions::updateGui( void )
{
    serverPort->setValue( mySettings.get_port() );
    nMaxConnections->setValue( mySettings.get_max_connections() );
    motd->setText( mySettings.get_motd() );
    needsLogin->setChecked( mySettings.unregistered_allowed() );

    users->setRowCount( 0 );
    QHashIterator<QString, QString> it( mySettings.get_user_table() );
    while( it.hasNext() )
    {
        it.next();
        users->insertRow( 0 );
        users->setItem( 0, 0, new QTableWidgetItem( it.key() ) );
        users->setItem( 0, 1, new QTableWidgetItem( it.value() ) );
    }
    if( users->rowCount() <= 0 )
        users->insertRow( 0 );
}

////////////////////////////////////////////////////////////////////////////////
/// Update the \ref mySettings object when the GUI was changed.
////////////////////////////////////////////////////////////////////////////////
void ServerOptions::updateSettings( void )
{
    mySettings.set_port( serverPort->value() );
    mySettings.set_max_connections( nMaxConnections->value() );
    mySettings.set_motd( motd->text() );
    mySettings.set_unregistered_allowed( !needsLogin->isChecked() );

    for( int i = 0; i < users->rowCount(); i++ )
    {
        if( users->item( i, 0 ) && !users->item( i, 0 )->text().isEmpty() )
        {
            if( users->item( i, 1 ) )
                mySettings.insert_user( users->item( i, 0 )->text(),
                                        users->item( i, 1 )->text() );
            else
                mySettings.insert_user( users->item( i, 0 )->text(), "" );
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
/// Shows a file selections dialog and loads the chosen configuration.
////////////////////////////////////////////////////////////////////////////////
void ServerOptions::on_load_clicked( void )
{
    QString
            file =
                    QFileDialog::getOpenFileName( this,
                                                  tr("Konfigurationsdatei wÃ¤hlen"),
                                                  "",
                                                  tr("Konfigurationsdateien (*.conf);;Alle Dateien (*)") );

    if( !file.isNull() )
    {
        try
        {
            readFromXml( file );
        }
        catch( std::runtime_error e )
        {
            QMessageBox::critical( this, tr("Fehler"),
                                   QString( tr("Es ist ein Fehler aufgetreten: ") )
                                           + e.what() );
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
/// Shows a file selection dialog and saves the current configuration.
////////////////////////////////////////////////////////////////////////////////
void ServerOptions::on_save_clicked( void )
{
    QString
            file =
                    QFileDialog::getSaveFileName( this,
                                                  tr("Konfigurationsdatei wÃ¤hlen"),
                                                  "",
                                                  tr("Konfigurationsdateien (*.conf);;Alle Dateien (*)") );

    if( !file.isNull() )
    {
        try
        {
            writeToXml( file );
        }
        catch( std::runtime_error e )
        {
            QMessageBox::critical( this, tr("Fehler"),
                                   QString( tr("Es ist ein Fehler aufgetreten: ") )
                                           + e.what() );
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
/// Adds and removes rows in the user table as needed.
////////////////////////////////////////////////////////////////////////////////
void ServerOptions::on_users_cellChanged( int row, int column )
{
    if( users->item( row, 0 ) && users->item( row, 0 )->text() != "" )
    {
        if( row == users->rowCount() - 1 )
        {
            users->insertRow( row + 1 );
            //          tbl_jobs->setItem( row + 1, JOB_NAME, new QTableWidgetItem() );
            //          tbl_jobs->setItem( row + 1, MIN_PARTICIPANTS,
            //                             new QTableWidgetItem( "0" ) );
            //          tbl_jobs->setItem( row + 1, MAX_PARTICIPANTS,
            //                             new QTableWidgetItem( "20" ) );
            //          QTableWidgetItem *item = new QTableWidgetItem();
            //          item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
            //          item->setCheckState( Qt::Unchecked );
            //          tbl_jobs->setItem( row + 1, SINGLE_BLOCK, item );
        }
    }
    else if( row < users->rowCount() - 1 && (!users->item( row, 0 )
            || users->item( row, 0 )->text().isEmpty()) )
        users->removeRow( row );
}
