#include "timerangefinder.h"

#include <algorithm>

#include <QRegularExpression>

namespace {

// Maximo de lineas que se retroceden buscando el timestamp que hereda una
// linea sin hora (por ejemplo, un stack trace largo).
constexpr qint64 kMaxBackScan = 20000;

// Solo se mira el comienzo de la linea para buscar la hora.
constexpr int kMaxPrefix = 64;

} // namespace

qint64 TimeRangeFinder::toMsecs( const QDate& date, const QTime& time )
{
    return date.toJulianDay() * 86400000LL + time.msecsSinceStartOfDay();
}

std::optional<qint64> TimeRangeFinder::defaultParser( const QString& line )
{
    static const QRegularExpression re(
        QStringLiteral( R"((\d{4})-(\d{2})-(\d{2})[T ](\d{2}):(\d{2}):(\d{2})(?:[.,](\d{1,3}))?)" ) );

    const auto match = re.match( line.left( kMaxPrefix ) );
    if ( !match.hasMatch() ) {
        return std::nullopt;
    }

    int ms = 0;
    const QString frac = match.captured( 7 );
    if ( !frac.isEmpty() ) {
        // "5" -> 500 ms, "12" -> 120 ms, "123" -> 123 ms
        ms = frac.leftJustified( 3, QLatin1Char( '0' ) ).toInt();
    }

    const QDate date( match.captured( 1 ).toInt(), match.captured( 2 ).toInt(),
                      match.captured( 3 ).toInt() );
    const QTime time( match.captured( 4 ).toInt(), match.captured( 5 ).toInt(),
                      match.captured( 6 ).toInt(), ms );

    if ( !date.isValid() || !time.isValid() ) {
        return std::nullopt;
    }

    return toMsecs( date, time );
}

TimeRangeFinder::Result TimeRangeFinder::find( const AbstractLogData& data, qint64 fromMs,
                                               qint64 toMs, const Parser& parser )
{
    Result result;

    if ( fromMs > toMs ) {
        result.error = QStringLiteral( "El inicio es posterior al fin del rango." );
        return result;
    }

    const qint64 nbLines = static_cast<qint64>( data.getNbLine().get() );
    if ( nbLines == 0 ) {
        result.error = QStringLiteral( "El archivo esta vacio." );
        return result;
    }

    auto lineAt = [ & ]( qint64 i ) {
        return data.getLineString( LineNumber( static_cast<LineNumber::UnderlyingType>( i ) ) );
    };

    // Timestamp "heredado" de la linea i: el de la propia linea o, si no
    // tiene, el de la linea anterior mas cercana que si lo tenga.
    auto inheritedTimestamp = [ & ]( qint64 i ) -> std::optional<qint64> {
        const qint64 limit = std::max<qint64>( 0, i - kMaxBackScan );
        for ( qint64 k = i; k >= limit; --k ) {
            const auto ts = parser( lineAt( k ) );
            if ( ts ) {
                return ts;
            }
        }
        return std::nullopt;
    };

    // Si ni la ultima linea hereda un timestamp, el formato no se reconoce.
    if ( !inheritedTimestamp( nbLines - 1 ) ) {
        result.error = QStringLiteral( "No se reconocio ningun timestamp en el archivo." );
        return result;
    }

    // Primera linea i con timestamp heredado >= target (o > target si strict).
    // Una linea sin timestamp heredado cuenta como "anterior a todo".
    auto firstAtOrAfter = [ & ]( qint64 target, bool strict ) -> qint64 {
        qint64 lo = 0;
        qint64 hi = nbLines;
        while ( lo < hi ) {
            const qint64 mid = lo + ( hi - lo ) / 2;
            const auto ts = inheritedTimestamp( mid );
            const bool reached = ts && ( strict ? *ts > target : *ts >= target );
            if ( reached ) {
                hi = mid;
            }
            else {
                lo = mid + 1;
            }
        }
        return lo;
    };

    result.start = firstAtOrAfter( fromMs, false );
    result.end = firstAtOrAfter( toMs, true );

    if ( result.start >= result.end ) {
        result.error = QStringLiteral( "No hay lineas dentro del rango indicado." );
        return result;
    }

    result.valid = true;
    return result;
}
