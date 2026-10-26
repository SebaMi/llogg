#include "timerangedialog.h"

#include <QDateTimeEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QVBoxLayout>

#include "timerangefinder.h"

namespace {
const char* const kDisplayFormat = "yyyy-MM-dd HH:mm:ss";
}

TimeRangeDialog::TimeRangeDialog( QWidget* parent )
    : QDialog( parent )
    , fromEdit_( new QDateTimeEdit( this ) )
    , toEdit_( new QDateTimeEdit( this ) )
{
    setWindowTitle( tr( "Filter by time range" ) );

    for ( auto* edit : { fromEdit_, toEdit_ } ) {
        edit->setDisplayFormat( QLatin1String( kDisplayFormat ) );
        edit->setCalendarPopup( true );
    }

    auto* form = new QFormLayout();
    form->addRow( tr( "From" ), fromEdit_ );
    form->addRow( tr( "To" ), toEdit_ );

    auto* buttons = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this );
    connect( buttons, &QDialogButtonBox::accepted, this, &QDialog::accept );
    connect( buttons, &QDialogButtonBox::rejected, this, &QDialog::reject );

    auto* layout = new QVBoxLayout( this );
    layout->addLayout( form );
    layout->addWidget( buttons );

    setRange( QDateTime( QDate::currentDate(), QTime( 0, 0, 0 ) ),
              QDateTime( QDate::currentDate(), QTime( 23, 59, 59 ) ) );
}

void TimeRangeDialog::setRange( const QDateTime& from, const QDateTime& to )
{
    fromEdit_->setDateTime( from );
    toEdit_->setDateTime( to );
}

qint64 TimeRangeDialog::fromMs() const
{
    return TimeRangeFinder::toMsecs( fromEdit_->date(), fromEdit_->time() );
}

qint64 TimeRangeDialog::toMs() const
{
    // Se incluye el segundo completo del "hasta".
    return TimeRangeFinder::toMsecs( toEdit_->date(), toEdit_->time() ) + 999;
}

void TimeRangeDialog::accept()
{
    if ( fromEdit_->dateTime() > toEdit_->dateTime() ) {
        QMessageBox::warning( this, tr( "Filter by time range" ),
                              tr( "The start of the range is after its end." ) );
        return;
    }
    QDialog::accept();
}
